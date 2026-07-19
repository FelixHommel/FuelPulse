#include "core/CommandInterpreter.hpp"
#include "core/CommandRegistry.hpp"
#include "core/EventBus.hpp"
#include "core/Scheduler.hpp"
#include "testUtility/ConcurrencyTest.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <sstream>
#include <thread>
#include <utility>
#include <vector>

namespace
{

/// \brief Test structure used to simulate the \ref Scheduler driving the system via \ref EventBus.
struct MinuteTickTest
{};

/// \brief Example a user command event.
struct StartRequestedTest
{};

} // namespace

namespace ful::testing
{

/// \brief Integration tests covering the \ref EventBus <-> \ref CommandInterpreter pairing.
///
/// Commands dispatched from user input should be able to communicate with the rest of the system exclusively through
/// published events.
///
/// \author Felix Hommel
/// \date 7/19/2026
class EventBusCommandInterpreterIntegrationTest : public ::testing::Test
{
public:
    EventBusCommandInterpreterIntegrationTest() = default;
    ~EventBusCommandInterpreterIntegrationTest() override = default;

    EventBusCommandInterpreterIntegrationTest(const EventBusCommandInterpreterIntegrationTest&) = delete;
    EventBusCommandInterpreterIntegrationTest& operator=(const EventBusCommandInterpreterIntegrationTest&) = delete;
    EventBusCommandInterpreterIntegrationTest(EventBusCommandInterpreterIntegrationTest&&) = delete;
    EventBusCommandInterpreterIntegrationTest& operator=(EventBusCommandInterpreterIntegrationTest&&) = delete;

protected:
    static constexpr auto START_CMD{ "start" };

    EventBus bus;
    CommandRegistry registry;
};

/// \brief Test that a command dispatched by the command interpreter can publish an event that a bus subscriber receives.
TEST_F(EventBusCommandInterpreterIntegrationTest, DispatchedCommandPublishesEventOnBus)
{
    std::atomic<int> received{ 0 };

    bus.subscribe<::StartRequestedTest>([&count = received](const ::StartRequestedTest&) {
        count.fetch_add(1, std::memory_order_relaxed);
    });

    registry.registerCommand(START_CMD, [](const std::vector<std::string>&, EventBus& b) {
        b.publish<::StartRequestedTest>();
    });

    std::stringstream input;
    input << START_CMD << "\n";
    CommandInterpreter interpreter(std::move(registry), bus, input, true);

    EXPECT_TRUE(waitOn([&received]() { return received.load(std::memory_order_relaxed) == 1; }));

    interpreter.requestStop(true);
}

/// \brief Test that every subscriber on the bus receives an event published as a result of command dispatch, not just
///     the first one.
TEST_F(EventBusCommandInterpreterIntegrationTest, MultipleSubscribersAllReceivedEVentFromDispatchedCommand)
{
    std::atomic<int> countA{ 0 };
    std::atomic<int> countB{ 0 };

    bus.subscribe<::StartRequestedTest>([&count = countA](const ::StartRequestedTest&) {
        count.fetch_add(1, std::memory_order_relaxed);
    });
    bus.subscribe<::StartRequestedTest>([&count = countB](const ::StartRequestedTest&) {
        count.fetch_add(1, std::memory_order_relaxed);
    });

    registry.registerCommand(START_CMD, [](const std::vector<std::string>&, EventBus& b) {
        b.publish<::StartRequestedTest>();
    });

    std::stringstream input;
    input << START_CMD << "\n";
    CommandInterpreter interpreter(std::move(registry), bus, input, true);

    EXPECT_TRUE(waitOn([&countA, &countB]() {
        return countA.load(std::memory_order_relaxed) == 1 && countB.load(std::memory_order_relaxed);
    }));

    interpreter.requestStop(true);
}

/// \brief Test that an unrecognized command is silently ignored and never reaches the bus, while subsequent valid
///     commands still dispatch normally.
TEST_F(EventBusCommandInterpreterIntegrationTest, UnknownCommandDoesNotPublishAnyEvent)
{
    std::atomic<int> received{ 0 };

    bus.subscribe<::StartRequestedTest>([&count = received](const ::StartRequestedTest&) {
        count.fetch_add(1, std::memory_order_relaxed);
    });

    std::atomic<bool> startDispatched{ false };
    registry.registerCommand(START_CMD, [&flag = startDispatched](const std::vector<std::string>&, EventBus& b) {
        flag.store(true, std::memory_order_relaxed);
        b.publish<::StartRequestedTest>();
    });

    std::stringstream input;
    input << "unknwon\n" << START_CMD << "\n";
    CommandInterpreter interpreter(std::move(registry), bus, input, true);

    EXPECT_TRUE(waitOn([&startDispatched]() { return startDispatched.load(std::memory_order_acquire); }));
    EXPECT_EQ(received.load(std::memory_order_relaxed), 1);

    interpreter.requestStop(true);
}

/// \brief Integration tests covering the \ref EventBus <-> \ref Scheduler pairing.
///
/// Scheduled tasks should be able to drive the rest of the system exclusively through published events.
///
/// \author Felix Hommel
/// \date 7/19/2026
class EventBusSchedulerSchedulerIntegrationTest : public ::testing::Test
{
public:
    EventBusSchedulerSchedulerIntegrationTest() = default;
    ~EventBusSchedulerSchedulerIntegrationTest() override = default;

    EventBusSchedulerSchedulerIntegrationTest(const EventBusSchedulerSchedulerIntegrationTest&) = delete;
    EventBusSchedulerSchedulerIntegrationTest& operator=(const EventBusSchedulerSchedulerIntegrationTest&) = delete;
    EventBusSchedulerSchedulerIntegrationTest(EventBusSchedulerSchedulerIntegrationTest&&) = delete;
    EventBusSchedulerSchedulerIntegrationTest& operator=(EventBusSchedulerSchedulerIntegrationTest&&) = delete;

protected:
    static constexpr auto INTERVAL{ std::chrono::milliseconds(10) };
    EventBus bus;
    Scheduler<> scheduler;
};

/// \brief Test that a single scheduled task can publish an event that a bus subscriber receives.
TEST_F(EventBusSchedulerSchedulerIntegrationTest, ScheduledTaskPublishesEventReceivedBySubscriber)
{
    int received{ 0 };
    bus.subscribe<::MinuteTickTest>([&count = received](const ::MinuteTickTest) { ++count; });

    scheduler.scheduleAt(std::chrono::steady_clock::now(), [&b = bus]() { b.publish<::MinuteTickTest>(); });
    scheduler.runDue();

    EXPECT_EQ(received, 1);
}

/// \brief Test that a recurring scheduled task publishes one event per tick over several ticks.
TEST_F(EventBusSchedulerSchedulerIntegrationTest, RecurringScheduledTaskPublishesOneEventPerTick)
{
    constexpr auto ITERATIONS{ 4 };

    int received{ 0 };
    bus.subscribe<::MinuteTickTest>([&count = received](const ::MinuteTickTest) { ++count; });

    scheduler.scheduleEvery(INTERVAL, [&b = bus]() { b.publish<::MinuteTickTest>(); });

    auto now{ std::chrono::steady_clock::now() };
    for(auto i{ 0 }; i < ITERATIONS; ++i)
    {
        scheduler.runDue(now);
        now += INTERVAL;
    }

    EXPECT_EQ(received, ITERATIONS);
}

/// \brief Test that canceling a recurring task through the \ref Scheduler stops it from publishing further events.
TEST_F(EventBusSchedulerSchedulerIntegrationTest, CancelingScheduledTaskStopsEventPublication)
{
    int received{ 0 };
    bus.subscribe<::MinuteTickTest>([&count = received](const ::MinuteTickTest) { ++count; });

    const auto id{ scheduler.scheduleEvery(INTERVAL, [&b = bus]() { b.publish<::MinuteTickTest>(); }) };

    auto now{ std::chrono::steady_clock::now() };
    scheduler.runDue(now);
    EXPECT_EQ(received, 1);

    now += INTERVAL;
    scheduler.cancel(id);
    scheduler.runDue(now);

    EXPECT_EQ(received, 1);
}

/// \brief Test that multiple subscribers on the bus receive a tick event published by the scheduler.
TEST_F(EventBusSchedulerSchedulerIntegrationTest, MultipleSubscribersAllReceiveScheduledEventTick)
{
    int countA{ 0 };
    int countB{ 0 };

    bus.subscribe<::StartRequestedTest>([&count = countA](const ::StartRequestedTest&) { count++; });
    bus.subscribe<::StartRequestedTest>([&count = countB](const ::StartRequestedTest&) { count++; });

    scheduler.scheduleAt(std::chrono::steady_clock::now(), [&b = bus]() { b.publish<::StartRequestedTest>(); });
    scheduler.runDue();

    EXPECT_EQ(countA, 1);
    EXPECT_EQ(countB, 1);
}

/// \brief Test that the background thread of \ref Scheduler reliably publishes multiple tick events over real time.
TEST_F(EventBusSchedulerSchedulerIntegrationTest, BackgroundSchedulerPublishesTicksOverTime)
{
    std::atomic<int> received{ 0 };

    bus.subscribe<::MinuteTickTest>([&count = received](const ::MinuteTickTest&) {
        count.fetch_add(1, std::memory_order_relaxed);
    });

    scheduler.scheduleEvery(INTERVAL, [&b = bus]() { b.publish<::MinuteTickTest>(); });
    scheduler.start();

    EXPECT_TRUE(waitOn([&received]() { return received.load(std::memory_order_relaxed) >= 3; }));

    scheduler.requestStop(true);
}

/// \brief Integration test covering the entire application core triangle of
///     Scheduler <-> EventBus <-> CommandInterpreter.
///
/// These tests should mirror the application level usage. User commands as well as Scheduled ticks both communicate and
/// flow exclusively via the \ref EventBus.
///
/// \author Felix Hommel
/// \date 7/19/2026
class FullCoreIntegrationTest : public ::testing::Test
{
public:
    FullCoreIntegrationTest() = default;
    ~FullCoreIntegrationTest() override = default;

    FullCoreIntegrationTest(const FullCoreIntegrationTest&) = delete;
    FullCoreIntegrationTest& operator=(const FullCoreIntegrationTest&) = delete;
    FullCoreIntegrationTest(FullCoreIntegrationTest&&) = delete;
    FullCoreIntegrationTest& operator=(FullCoreIntegrationTest&&) = delete;

protected:
    static constexpr auto START_CMD{ "start" };
    static constexpr auto STOP_CMD{ "stop" };
    static constexpr auto TICK_INTERVAL{ std::chrono::milliseconds(5) };

    EventBus bus;
    CommandRegistry registry;
    Scheduler<> scheduler;
};

/// \brief Test that a "start" command dispatched via the \ref CommandInterpreter starts the \ref Scheduler, which then
///     begins publishing tick events via the \ref EventBus.
TEST_F(FullCoreIntegrationTest, StartCommandStartsSchedulerProducingTickEvents)
{
    std::atomic<int> tickCount{ 0 };

    bus.subscribe<::MinuteTickTest>([&c = tickCount](const ::MinuteTickTest&) {
        c.fetch_add(1, std::memory_order_relaxed);
    });

    registry.registerCommand(START_CMD, [&sched = scheduler](const std::vector<std::string>&, EventBus& b) {
        sched.scheduleEvery(TICK_INTERVAL, [&b]() { b.publish<::MinuteTickTest>(); });
        sched.start();
    });

    std::stringstream input;
    input << START_CMD << "\n";

    CommandInterpreter interpreter(std::move(registry), bus, input, true);

    EXPECT_TRUE(waitOn([&tickCount]() { return tickCount.load(std::memory_order_relaxed) >= 3; }));

    interpreter.requestStop(true);
    scheduler.requestStop(true);
}

/// \brief Test that a "stop" command dispatched via the \ref CommandInterpreter halts the running \ref Scheduler,
/// ending tick production.
TEST_F(FullCoreIntegrationTest, StopCommandHaltsSchedulerTickProduction)
{
    std::atomic<int> tickCount{ 0 };

    bus.subscribe<::MinuteTickTest>([&c = tickCount](const ::MinuteTickTest&) {
        c.fetch_add(1, std::memory_order_relaxed);
    });

    scheduler.scheduleEvery(TICK_INTERVAL, [&b = bus]() { b.publish<::MinuteTickTest>(); });
    scheduler.start();

    EXPECT_TRUE(waitOn([&tickCount]() { return tickCount.load(std::memory_order_relaxed) >= 1; }));

    std::atomic<bool> stopDispatched{ false };
    registry.registerCommand(
        STOP_CMD, [&sched = scheduler, &flag = stopDispatched](const std::vector<std::string>, EventBus&) {
            sched.requestStop(true);
            flag.store(true, std::memory_order_release);
        }
    );

    std::stringstream input;
    input << STOP_CMD << "\n";

    CommandInterpreter interpreter(std::move(registry), bus, input, true);

    EXPECT_TRUE(waitOn([&stopDispatched]() { return stopDispatched.load(std::memory_order_acquire); }));

    const auto countAfterStop{ tickCount.load(std::memory_order_relaxed) };
    std::this_thread::sleep_for(TICK_INTERVAL * 2 * 3);

    EXPECT_EQ(tickCount.load(std::memory_order_relaxed), countAfterStop);

    interpreter.requestStop(true);
}

} // namespace ful::testing
