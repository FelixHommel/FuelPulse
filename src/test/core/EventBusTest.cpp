#include "core/EventBus.hpp"

#include "testUtility/RandomNumberGenerator.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace
{

struct TestEvent1
{
    int value{ 0 };
};

struct TestEvent2
{
    std::string message;
};

} // namespace

namespace ful::testing
{

/// \brief Simple test fixture providing a \ref EventBus instance to perform tests on.
///
/// \author Felix Hommel
/// \date 7/16/2026
class EventBusTest : public ::testing::Test
{
public:
    EventBusTest() = default;
    ~EventBusTest() override = default;

    EventBusTest(const EventBusTest&) = delete;
    EventBusTest& operator=(const EventBusTest&) = delete;
    EventBusTest(EventBusTest&&) = delete;
    EventBusTest& operator=(EventBusTest&&) = delete;

protected:
    EventBus bus;
};

using EventBusPubSubTest = EventBusTest;

/// Test that subscribers to an event are delivered the event on publish.
TEST_F(EventBusPubSubTest, SubscribeAndPublishDeliversEvent)
{
    bool called{ false };

    bus.subscribe<::TestEvent1>([&flag = called](const ::TestEvent1&) { flag = true; });
    bus.publish<TestEvent1>();

    EXPECT_TRUE(called);
}

/// \brief Test that the correct event data is published.
TEST_F(EventBusPubSubTest, EventDataIsPassedCorrectly)
{
    const auto value{ generateRandomValue<int>() };

    ::TestEvent1 received{};

    bus.subscribe<TestEvent1>([&r = received](const TestEvent1& event) { r = event; });
    bus.publish<TestEvent1>({ .value = value });

    EXPECT_EQ(received.value, value);
}

/// \brief Test that all subscribers that are subscribed to a certain event type are notified when the event is published.
TEST_F(EventBusPubSubTest, MultipleSubscribersAllReceiveEvent)
{
    int count{ 0 };

    bus.subscribe<TestEvent1>([&c = count](const TestEvent1&) { ++c; });
    bus.subscribe<TestEvent1>([&c = count](const TestEvent1&) { ++c; });
    bus.subscribe<TestEvent1>([&c = count](const TestEvent1&) { ++c; });

    bus.publish<TestEvent1>();

    EXPECT_EQ(count, 3);
}

/// \brief Test that the event handler is invoked every time a new event is published.
TEST_F(EventBusPubSubTest, MultiplePublishesInvokeHandlerMultipleTimes)
{
    constexpr auto REPS{ 10 };

    int count{ 0 };

    bus.subscribe<TestEvent1>([&c = count](const TestEvent1&) { ++c; });

    for(int i{ 0 }; i < REPS; ++i)
        bus.publish<TestEvent1>();

    EXPECT_EQ(count, REPS);
}

using EventBusTypeIsolationTest = EventBusTest;

/// \brief Test that subscribers only react to the event type they have subscribed to.
TEST_F(EventBusTypeIsolationTest, DifferentEventTypesAreIsolated)
{
    int count1{ 0 };
    int count2{ 0 };

    bus.subscribe<TestEvent1>([&c = count1](const TestEvent1&) { ++c; });
    bus.subscribe<TestEvent2>([&c = count2](const TestEvent2&) { ++c; });

    bus.publish<TestEvent1>();

    EXPECT_EQ(count1, 1);
    EXPECT_EQ(count2, 0);

    bus.publish<TestEvent2>();

    EXPECT_EQ(count1, 1);
    EXPECT_EQ(count2, 1);
}

/// \brief Test that subscribers are assigned different IDs across different types.
TEST_F(EventBusTypeIsolationTest, SubscriptionIdsAreUniqueAcrossEventTypes)
{
    const auto id1{ bus.subscribe<TestEvent1>([](const TestEvent1&) {}) };
    const auto id2{ bus.subscribe<TestEvent2>([](const TestEvent2&) {}) };

    EXPECT_NE(id1, id2);
}

using EventBusUnsubscribeTest = EventBusTest;

/// \brief Test that once a subscriber unsubscribes from an event, it does no longer receive the events.
TEST_F(EventBusUnsubscribeTest, UnsubscribeStopDelivery)
{
    int count{ 0 };

    const auto id{ bus.subscribe<TestEvent1>([&c = count](const TestEvent1&) { ++c; }) };

    bus.publish<TestEvent1>();

    EXPECT_EQ(count, 1);

    bus.unsubscribe<TestEvent1>(id);
    bus.publish<TestEvent1>();

    EXPECT_EQ(count, 1);
}

/// \brief Test that one subscriber unsubscribing from the bus does not affect other subscribers.
TEST_F(EventBusUnsubscribeTest, UnsubscribingDoesNotAffectOthers)
{
    int count1{ 0 };
    int count2{ 0 };

    const auto id{ bus.subscribe<TestEvent1>([&c = count1](const TestEvent1&) { ++c; }) };
    bus.subscribe<TestEvent1>([&c = count2](const TestEvent1&) { ++c; });

    bus.unsubscribe<TestEvent1>(id);
    bus.publish<TestEvent1>();

    EXPECT_EQ(count1, 0);
    EXPECT_EQ(count2, 1);
}

/// \brief Test that the \ref EventBus handles unsubscribe calls from unknown types gracefully.
TEST_F(EventBusUnsubscribeTest, UnsubscribeUnknownTypeDoesNothing)
{
    EXPECT_NO_THROW(bus.unsubscribe<TestEvent1>(generateRandomValue<EventBus::SubscriptionId>()));
}

/// \brief Test that the \ref EventBus handles unsubscribe calls from nonsubscribers gracefully.
TEST_F(EventBusUnsubscribeTest, UnsubscribeUnknownIdDoesNothing)
{
    int count{ 0 };

    bus.subscribe<TestEvent1>([&c = count](const TestEvent1&) { ++c; });
    bus.unsubscribe<TestEvent1>(generateRandomValue<EventBus::SubscriptionId>(1));

    bus.publish<TestEvent1>();

    EXPECT_EQ(count, 1);
}

/// \brief Test that the \ref EventBus handles attempts to unsubscribe multiple times gracefully.
TEST_F(EventBusUnsubscribeTest, UnsubscribeTwiceIsSafe)
{
    const auto id{ bus.subscribe<TestEvent1>([](const TestEvent1&) {}) };
    bus.unsubscribe<TestEvent1>(id);

    EXPECT_NO_THROW(bus.unsubscribe<TestEvent1>(id));
}

/// \brief Test that unsubscribing from within a handler callback does not disrupt event distribution.
TEST_F(EventBusUnsubscribeTest, UnsubscribingWithHandlerDoesNotAffectCurrentPublish)
{
    int secondCallCount{ 0 };
    EventBus::SubscriptionId secondId{};

    bus.subscribe<TestEvent1>([&bus = bus, &id2 = secondId](const TestEvent1&) { bus.unsubscribe<TestEvent1>(id2); });
    secondId = bus.subscribe<TestEvent1>([&count = secondCallCount](const TestEvent1&) { ++count; });

    bus.publish<TestEvent1>();
    EXPECT_EQ(secondCallCount, 1);

    bus.publish<TestEvent1>();
    EXPECT_EQ(secondCallCount, 1);
}

using EventBusExceptionSafetyTest = EventBusTest;

/// \brief Test that the \ref EventBus does not let handler \ref std::exception propagate through the handler call.
TEST_F(EventBusExceptionSafetyTest, HandlerThrowingStdExecptionDoesNotPropagate)
{
    bus.subscribe<TestEvent1>([](const TestEvent1&) { throw std::runtime_error("failure"); });

    EXPECT_NO_THROW(bus.publish<TestEvent1>());
}

/// \brief Test that the \ref EventBus does not let handler exceptions propagate through the handler call.
TEST_F(EventBusExceptionSafetyTest, HandlerThrowingNonStdExecptionDoesNotPropagate)
{
    bus.subscribe<TestEvent1>([](const TestEvent1&) { throw 1; });

    EXPECT_NO_THROW(bus.publish<TestEvent1>());
}

/// \brief Test that one handler throwing an exception does not disturb the execution of other handlers.
TEST_F(EventBusExceptionSafetyTest, HandlerThrowingDoesNotStopOtherHandlers)
{
    bool secondCall{ false };

    bus.subscribe<TestEvent1>([](const TestEvent1&) { throw std::runtime_error("failure"); });
    bus.subscribe<TestEvent1>([&flag = secondCall](const TestEvent1&) { flag = true; });

    bus.publish<TestEvent1>();

    EXPECT_TRUE(secondCall);
}

using EventBusRecursiveTest = EventBusTest;

/// \brief Test that event handlers can themselves publish new events to the \ref EventBus.
TEST_F(EventBusRecursiveTest, RecursivePublishFromHandlerWorks)
{
    int outerCount{ 0 };
    int innerCount{ 0 };

    bus.subscribe<TestEvent1>([&count = outerCount, &bus = bus](const TestEvent1& e) {
        ++count;
        if(e.value == 0)
            bus.publish<TestEvent1>({ .value = 1 });
    });
    bus.subscribe<TestEvent1>([&count = innerCount](const TestEvent1&) { ++count; });

    bus.publish<TestEvent1>();

    EXPECT_EQ(outerCount, 2);
    EXPECT_EQ(innerCount, 2);
}

using EventBusThreadSafetyTest = EventBusTest;

/// \brief Test that the \ref EventBus handles the concurrent publishing of events well.
TEST_F(EventBusThreadSafetyTest, ConcurrentPublishFromMultipleThreadsIsThreadSafe)
{
    constexpr auto PUBLISHES_PER_THREAD{ 500u };
    const auto NUM_THREADS{ std::thread::hardware_concurrency() >= 2u ? std::thread::hardware_concurrency() / 4u : 1u };

    std::atomic<int> counter{ 0 };

    bus.subscribe<TestEvent1>([&c = counter](const TestEvent1&) { c.fetch_add(1, std::memory_order_relaxed); });

    std::vector<std::thread> threads;
    threads.reserve(NUM_THREADS);

    for(unsigned int i{ 0 }; i < NUM_THREADS; ++i)
    {
        threads.emplace_back([&bus = bus]() {
            for(unsigned int j{ 0 }; j < PUBLISHES_PER_THREAD; ++j)
                bus.publish<TestEvent1>();
        });
    }

    for(auto& thread : threads)
        thread.join();

    EXPECT_EQ(counter.load(), NUM_THREADS * PUBLISHES_PER_THREAD);
}

/// \brief Test that the \ref EventBus can handle many concurrent subscribe and unsubscribe calls.
TEST_F(EventBusThreadSafetyTest, ConcurrentSubscribeUnsubscribeWhilePublishingDoesNotCrash)
{
    constexpr auto PUBLISHES_PER_THREAD{ 200u };
    const auto NUM_THREADS{ std::thread::hardware_concurrency() >= 2u ? std::thread::hardware_concurrency() / 8u : 1u };

    std::atomic<bool> stop{ false };

    std::thread publisher([&bus = bus, &stopFlag = stop]() {
        while(!stopFlag.load(std::memory_order_relaxed))
            bus.publish<TestEvent1>();
    });

    std::vector<std::thread> subscribers;
    subscribers.reserve(NUM_THREADS);

    for(unsigned int i{ 0 }; i < NUM_THREADS; ++i)
    {
        subscribers.emplace_back([&bus = bus]() {
            for(unsigned int j{ 0 }; j < PUBLISHES_PER_THREAD; ++j)
            {
                const auto id{ bus.subscribe<TestEvent1>([](const TestEvent1&) {}) };
                bus.unsubscribe<TestEvent1>(id);
            }
        });
    }

    for(auto& thead : subscribers)
        thead.join();

    stop.store(true, std::memory_order_relaxed);
    publisher.join();

    SUCCEED();
}

} // namespace ful::testing
