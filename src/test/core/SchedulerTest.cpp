#include "core/Scheduler.hpp"
#include "testUtility/ConcurrencyTest.hpp"
#include "testUtility/RandomNumberGenerator.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <stdexcept>
#include <thread>
#include <vector>

namespace
{

/// \brief Deterministic, manually-advanced clock used to test the \ref Scheduler without depending on wall-clock time.
///
/// \author Felix Hommel
/// \date 7/18/2026
class TestClock
{
public:
    using duration = std::chrono::milliseconds;
    using rep = duration::rep;
    using period = duration::period;
    using time_point = std::chrono::time_point<TestClock, duration>;

    static time_point now() { return s_now; }

    static void advance(duration d) noexcept { s_now += d; };
    static void reset() noexcept { s_now = time_point{}; };

private:
    static inline time_point s_now{};
};

} // namespace

namespace ful::testing
{

/// \brief Test fixture providing a \ref Scheduler driven by a \ref TestClock to test timing behavior deterministically.
///
/// \author Felix Hommel
/// \date 7/18/2026
class SchedulerTest : public ::testing::Test
{
public:
    SchedulerTest() = default;
    ~SchedulerTest() override = default;

    SchedulerTest(const SchedulerTest&) = delete;
    SchedulerTest& operator=(const SchedulerTest&) = delete;
    SchedulerTest(SchedulerTest&&) = delete;
    SchedulerTest& operator=(SchedulerTest&&) = delete;

    void SetUp() override { ::TestClock::reset(); }

protected:
    Scheduler<::TestClock> scheduler;
};

using SchedulerScheduleAtTest = SchedulerTest;

/// \brief Test that a task that was scheduled does not run before it's scheduled time is reached.
TEST_F(SchedulerScheduleAtTest, TaskDoesNotRunBeforeDueTime)
{
    bool called{ false };

    scheduler.scheduleAt(::TestClock::now() + std::chrono::milliseconds(3), [&flag = called]() { flag = true; });
    scheduler.runDue();

    EXPECT_FALSE(called);
}

/// \brief Test that a scheduled task runs exactly when the scheduled time is reached.
TEST_F(SchedulerScheduleAtTest, TaskRunsExactlyAtDueTime)
{
    const auto due{ ::TestClock::now() + ::std::chrono::milliseconds(100) };

    bool called{ false };

    scheduler.scheduleAt(::TestClock::now() + std::chrono::milliseconds(3), [&flag = called]() { flag = true; });
    scheduler.runDue(due);

    EXPECT_TRUE(called);
}

/// \brief Test that a task whose scheduled time is already passed is executed immediately.
TEST_F(SchedulerScheduleAtTest, TaskScheduledInThePastRunsImmediately)
{
    bool called{ false };

    scheduler.scheduleAt(::TestClock::now() - std::chrono::milliseconds(1), [&flag = called]() { flag = true; });
    scheduler.runDue();

    EXPECT_TRUE(called);
}

using SchedulerScheduleInTest = SchedulerTest;

/// \brief Test that a task that is scheduled with a relative delay only runs once the delay has fully elapsed.
TEST_F(SchedulerScheduleInTest, TaskRunsOnlyAfterDelayElapsed)
{
    constexpr auto SCHEDULED_EXEC{ std::chrono::milliseconds(50) };
    bool called{ false };

    scheduler.scheduleIn(SCHEDULED_EXEC, [&flag = called]() { flag = true; });

    ::TestClock::advance(SCHEDULED_EXEC - std::chrono::milliseconds(1));
    scheduler.runDue();

    EXPECT_FALSE(called);

    ::TestClock::advance(std::chrono::milliseconds(1));
    scheduler.runDue();

    EXPECT_TRUE(called);
}

class SchedulerScheduleEveryTest : public SchedulerTest
{
protected:
    static constexpr auto INTERVAL{ std::chrono::milliseconds(10) };
};

/// \brief Test that a recurring task without an explicit first-run time is due immediately
TEST_F(SchedulerScheduleEveryTest, DefaultFirstRunIsImmediate)
{
    int count{ 0 };

    scheduler.scheduleEvery(INTERVAL, [&c = count]() { ++c; });
    scheduler.runDue();

    EXPECT_EQ(count, 1);
}

/// \brief Test that a recurring task gets rescheduled correctly.
TEST_F(SchedulerScheduleEveryTest, TaskRescheduledsAfterEachRun)
{
    constexpr auto ITERATIONS{ 5 };

    int count{ 0 };

    scheduler.scheduleEvery(INTERVAL, [&c = count]() { ++c; });

    for(auto i{ 0 }; i < ITERATIONS; ++i)
    {
        scheduler.runDue();
        ::TestClock::advance(INTERVAL);
    }

    EXPECT_EQ(count, ITERATIONS);
}

/// \brief Test that an explicit first-run time delays the initial execution of a recurring task.
TEST_F(SchedulerScheduleEveryTest, ExplicitFirstRunDelaysInitialExecution)
{
    constexpr auto OFFSET{ std::chrono::milliseconds(30) };
    const auto firstRun{ ::TestClock::now() + OFFSET };

    int count{ 0 };

    scheduler.scheduleEvery(INTERVAL, [&c = count]() { ++c; }, firstRun);
    scheduler.runDue();

    EXPECT_EQ(count, 0);

    ::TestClock::advance(OFFSET);
    scheduler.runDue();

    EXPECT_EQ(count, 1);
}

using SchedulerRunDueTest = SchedulerTest;

/// \brief Test that \ref Scheduler::runDue() reports no pending work when nothing was scheduled.
TEST_F(SchedulerRunDueTest, ReturnsNulloptWhenNoTasksAreScheduled)
{
    EXPECT_FALSE(scheduler.runDue().has_value());
}

/// \brief Test that \ref Scheduler::runDue() reports the next pending task's due time when a task remains scheduled.
TEST_F(SchedulerRunDueTest, ReturnsNextPendingTimeWhenTaskRemains)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Not unsafe in this context

    const auto due{ ::TestClock::now() + std::chrono::milliseconds(50) };

    scheduler.scheduleAt(due, []() {});
    const auto next{ scheduler.runDue() };

    ASSERT_TRUE(next.has_value());
    EXPECT_EQ(*next, due);

    // NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Test that multiple tasks due at the same point in time are all executed by a single call.
TEST_F(SchedulerRunDueTest, MultipleTasksDueAtSameTimeAllRun)
{
    int count{ 0 };

    scheduler.scheduleAt(::TestClock::now(), [&c = count]() { ++c; });
    scheduler.scheduleAt(::TestClock::now(), [&c = count]() { ++c; });
    scheduler.scheduleAt(::TestClock::now(), [&c = count]() { ++c; });

    scheduler.runDue();

    EXPECT_EQ(count, 3);
}

/// \brief Test that only tasks that were due before or at the queried point in time are executed, later tasks are left
///     pending.
TEST_F(SchedulerRunDueTest, OnlyTasksDueAtOrBeforeNowRun)
{
    bool early{ false };
    bool onTime{ false };
    bool late{ false };

    scheduler.scheduleAt(::TestClock::now() - std::chrono::milliseconds(1), [&flag = early]() { flag = true; });
    scheduler.scheduleAt(::TestClock::now(), [&flag = onTime]() { flag = true; });
    scheduler.scheduleAt(::TestClock::now() + std::chrono::milliseconds(1), [&flag = late]() { flag = true; });

    scheduler.runDue();

    EXPECT_TRUE(early);
    EXPECT_TRUE(onTime);
    EXPECT_FALSE(late);
}

/// \brief Test that tasks are executed in ascending order of their due time, regardless of scheduling order.
TEST_F(SchedulerRunDueTest, TasksRunInOrderOfDueTime)
{
    std::vector<int> order;

    scheduler.scheduleAt(::TestClock::now() + std::chrono::milliseconds(2), [&order]() { order.push_back(2); });
    scheduler.scheduleAt(::TestClock::now() + std::chrono::milliseconds(1), [&order]() { order.push_back(1); });
    scheduler.scheduleAt(::TestClock::now(), [&order]() { order.push_back(0); });

    ::TestClock::advance(std::chrono::milliseconds(2));
    scheduler.runDue();

    ASSERT_EQ(order.size(), 3);
    EXPECT_EQ(order[0], 0);
    EXPECT_EQ(order[1], 1);
    EXPECT_EQ(order[2], 2);
}

using SchedulerCancelTest = SchedulerTest;

/// \brief Test that canceling a pending task prevents it from ever running.
TEST_F(SchedulerCancelTest, CancelingPendingTaskPreventsExecution)
{
    bool called{ false };
    const auto id{ scheduler.scheduleAt(::TestClock::now() + std::chrono::milliseconds(1), [&flag = called]() {
        flag = true;
    }) };

    EXPECT_TRUE(scheduler.cancel(id));

    ::TestClock::advance(std::chrono::milliseconds(1));
    scheduler.runDue();

    EXPECT_FALSE(called);
}

/// \brief Test that canceling an unknown \ref Scheduler::TaskId is reported as a no-op.
TEST_F(SchedulerCancelTest, CancelReturnsFalseForUnknownId)
{
    EXPECT_FALSE(scheduler.cancel(generateRandomValue<Scheduler<::TestClock>::TaskId>(1)));
}

/// \brief Test that canceling the same task twice reports failure on the second attempt.
TEST_F(SchedulerCancelTest, CancelTwiceReturnsFalseOnSecondAttempt)
{
    const auto id{ scheduler.scheduleAt(::TestClock::now(), []() {}) };

    EXPECT_TRUE(scheduler.cancel(id));
    EXPECT_FALSE(scheduler.cancel(id));
}

/// \brief Test that canceling a recurring task before it first run means it never executes at all
TEST_F(SchedulerCancelTest, CancelingBeforeFirstRunOfRecurringTaskPreventsExecution)
{
    constexpr auto INTERVAL{ std::chrono::milliseconds(10) };

    int count{ 0 };

    const auto id{ scheduler.scheduleEvery(INTERVAL, [&c = count]() { ++c; }) };

    scheduler.cancel(id);
    scheduler.runDue();

    EXPECT_EQ(count, 0);
}

/// \brief Test that canceling a recurring task after it has already ran stops any future runs.
TEST_F(SchedulerCancelTest, CancelingRecurringTaskStopsFutureRuns)
{
    constexpr auto INTERVAL{ std::chrono::milliseconds(10) };

    int count{ 0 };

    const auto id{ scheduler.scheduleEvery(INTERVAL, [&c = count]() { ++c; }) };

    scheduler.runDue();

    EXPECT_EQ(count, 1);

    ::TestClock::advance(INTERVAL);
    EXPECT_TRUE(scheduler.cancel(id));
    scheduler.runDue();

    EXPECT_EQ(count, 1);
}

using SchedulerExceptionSafetyTest = SchedulerTest;

/// \brief Test that a task throwing a \ref std::exception does not interrupt the internal state of the application.
TEST_F(SchedulerExceptionSafetyTest, TaskThrowingStdExceptionDoesNotStopOtherTasks)
{
    bool secondCalled{ false };

    scheduler.scheduleAt(::TestClock::now(), []() { throw std::runtime_error("failed"); });
    scheduler.scheduleAt(::TestClock::now(), [&flag = secondCalled]() { flag = true; });

    EXPECT_NO_THROW(scheduler.runDue());
    EXPECT_TRUE(secondCalled);
}

/// \brief Test that a task throwing a non \ref std::exception does not interrupt the internal state of the application.
TEST_F(SchedulerExceptionSafetyTest, TaskThrowingNonStdExceptionDoesNotStopOtherTasks)
{
    bool secondCalled{ false };

    scheduler.scheduleAt(::TestClock::now(), []() { throw 1; });
    scheduler.scheduleAt(::TestClock::now(), [&flag = secondCalled]() { flag = true; });

    EXPECT_NO_THROW(scheduler.runDue());
    EXPECT_TRUE(secondCalled);
}

/// \brief Test fixture for testing the \ref Scheduler background thread behavior using a real
///     \ref std::chrono::steady_clock.
class SchedulerBackgroundTest : public ::testing::Test
{
public:
    SchedulerBackgroundTest() = default;
    ~SchedulerBackgroundTest() override = default;

    SchedulerBackgroundTest(const SchedulerBackgroundTest&) = delete;
    SchedulerBackgroundTest& operator=(const SchedulerBackgroundTest&) = delete;
    SchedulerBackgroundTest(SchedulerBackgroundTest&&) = delete;
    SchedulerBackgroundTest& operator=(SchedulerBackgroundTest&&) = delete;

protected:
    static constexpr auto OFFSET{ std::chrono::milliseconds(10) };
    static constexpr auto INTERVAL{ std::chrono::milliseconds(5) };

    Scheduler<> scheduler;
};

/// \brief Test that a task scheduled with a short relative delay runs on the background thread once started.
TEST_F(SchedulerBackgroundTest, ScheduledTaskRunsOnBackgroundThread)
{
    std::atomic<bool> called{ false };

    scheduler.scheduleIn(OFFSET, [&flag = called]() { flag.store(true, std::memory_order_release); });
    scheduler.start();

    EXPECT_TRUE(waitOn([&called]() { return called.load(std::memory_order_acquire); }));

    scheduler.requestStop(true);
}

/// \brief Test that a recurring task fires multiple times while the background thread is running.
TEST_F(SchedulerBackgroundTest, RecurringTaskRunsMultipleTimesInBackground)
{
    std::atomic<int> count{ 0 };

    scheduler.scheduleEvery(INTERVAL, [&c = count]() { c.fetch_add(1, std::memory_order_relaxed); });
    scheduler.start();

    EXPECT_TRUE(waitOn([&count]() { return count.load(std::memory_order_acquire) >= 3; }));

    scheduler.requestStop(true);
}

/// \brief Test that requesting a stop actually halts further execution of a recurring task.
TEST_F(SchedulerBackgroundTest, RequestStopPreventsFurtherExecutions)
{
    std::atomic<int> count{ 0 };

    scheduler.scheduleEvery(INTERVAL, [&c = count]() { c.fetch_add(1, std::memory_order_relaxed); });
    scheduler.start();

    EXPECT_TRUE(waitOn([&count]() { return count.load(std::memory_order_acquire) >= 1; }));

    scheduler.requestStop(true);
    const auto countAfterStop{ count.load(std::memory_order_acquire) };

    std::this_thread::sleep_for(3 * OFFSET);

    EXPECT_EQ(count.load(std::memory_order_acquire), countAfterStop);
}

/// \brief Test that calling \ref Scheduler::start() multiple times does not cause issues in the internal state.
TEST_F(SchedulerBackgroundTest, DoubleStartIsSafeAndDoesNotDeadlock)
{
    std::atomic<bool> called{ false };

    scheduler.scheduleIn(OFFSET, [&flag = called]() { flag.store(true, std::memory_order_release); });
    scheduler.start();

    EXPECT_NO_FATAL_FAILURE(scheduler.start());

    EXPECT_TRUE(waitOn([&called]() { return called.load(std::memory_order_acquire); }));

    scheduler.requestStop(true);
}

/// \brief Test that waiting for a \ref Scheduler that was never started to stop does not hang or crash.
TEST_F(SchedulerBackgroundTest, WaitUntilStoppedBeforeStartDoesNotHang)
{
    EXPECT_NO_FATAL_FAILURE(scheduler.waitUnitlStopped());
}

/// \brief Test that requesting a stop before the \re Scheduler was ever started does not crash.
TEST_F(SchedulerBackgroundTest, RequestStopBeforeStartDoesNotCrash)
{
    EXPECT_NO_FATAL_FAILURE(scheduler.requestStop());
}

/// \brief Test that destroying the running \ref Scheduler stops and joins its background thread without hanging.
TEST_F(SchedulerBackgroundTest, DestructorStopsBackgroundThreadWithoutHanging)
{
    std::atomic<bool> called{ false };

    {
        Scheduler<> localScheduler;
        localScheduler.scheduleEvery(INTERVAL, [&flag = called]() { flag.store(true, std::memory_order_release); });
        localScheduler.start();

        EXPECT_TRUE(waitOn([&called]() { return called.load(std::memory_order_acquire); }));
    }

    SUCCEED();
}

} // namespace ful::testing
