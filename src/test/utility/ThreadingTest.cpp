#include "utility/ThreadingDetail.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <thread>
#include <utility>

namespace
{

constexpr auto MS_20{ std::chrono::milliseconds(20) };

} // namespace

namespace ful::testing
{

using namespace threading::detail;

/// \brief Test that a freshly created \ref stop_source_t's token is not stopped.
TEST(CustomStopSourceTest, GetTokenInitiallyNotStopped)
{
    stop_source_t source;

    EXPECT_FALSE(source.get_token().stop_requested());
}

/// \brief Test that requesting a stop is reflected in a token obtained beforehand.
TEST(CustomStopSourceTest, RequestStopReflectedInPreviouslyObtainedToken)
{
    stop_source_t source;
    const auto token{ source.get_token() };

    EXPECT_FALSE(token.stop_requested());

    source.request_stop();

    EXPECT_TRUE(token.stop_requested());
}

/// \brief Test that all tokens from the same source share the same underlying state.
TEST(CustomStopSourceTest, MultipleTokensShareState)
{
    stop_source_t source;
    const auto token1{ source.get_token() };
    const auto token2{ source.get_token() };

    source.request_stop();

    EXPECT_TRUE(token1.stop_requested());
    EXPECT_TRUE(token2.stop_requested());
}

/// \brief Test that a default constructed \ref stop_token_t reports no stop requested.
TEST(CustomStopTokenTest, DefaultConstructedTokenReportsNotStopped)
{
    const stop_token_t token;

    EXPECT_FALSE(token.stop_requested());
}

/// \brief Test that a default constructed \ref thread_t is not joinable.
TEST(CustomThreadTest, DefaultConstructedIsNotJoinable)
{
    const thread_t t;

    EXPECT_FALSE(t.joinable());
}

/// \brief Test that constructing with a no-argument callable starts and runs it.
TEST(CustomThreadTest, ConstructWithNoArgCallableRuns)
{
    std::atomic<bool> called{ false };

    thread_t t([&flag = called]() { flag.store(true, std::memory_order_release); });

    EXPECT_TRUE(t.joinable());

    t.join();

    EXPECT_TRUE(called.load(std::memory_order_acquire));
}

/// \brief Test that a callable accepting a \ref stop_token_t receives a token that initially reports no stop requested.
TEST(CustomThreadTest, CallableReceivesTokenNotInitiallyStopped)
{
    std::atomic<bool> observedStopRequested{ false };
    std::atomic<bool> ran{ false };

    thread_t t([&stopFlag = observedStopRequested, &ranFlag = ran](stop_token_t token) {
        stopFlag.store(token.stop_requested(), std::memory_order_release);
        ranFlag.store(true, std::memory_order_release);
    });

    t.join();

    EXPECT_TRUE(ran.load(std::memory_order_acquire));
    EXPECT_FALSE(observedStopRequested.load(std::memory_order_acquire));
}

/// \brief Test that \ref thread_t::request_stop causes a cooperative loop observing the token to exit.
TEST(CustomThreadTest, RequestStopStopsCooperativeLoop)
{
    std::atomic<int> iterations{ 0 };

    thread_t t([&count = iterations](stop_token_t token) {
        while(!token.stop_requested())
        {
            count.fetch_add(1, std::memory_order_relaxed);
            count.notify_all();
        }
    });

    iterations.wait(0);

    t.request_stop();
    t.join();

    EXPECT_GT(iterations.load(std::memory_order_relaxed), 0);
    EXPECT_FALSE(t.joinable());
}

/// \brief Test that a callable which does not accept a \ref stop_token_t still runs to completion and is unaffected by
///     \ref thread_t::request_stop().
TEST(CustomThreadTest, NonCooperativeCallableIgnoresRequestStop)
{
    std::atomic<bool> completed{ false };

    thread_t t([&flag = completed]() {
        std::this_thread::sleep_for(::MS_20);
        flag.store(true, std::memory_order_release);
    });

    t.request_stop();
    t.join();

    EXPECT_TRUE(completed.load(std::memory_order_acquire));
}

/// \brief Test that move constructing a \ref thread_t transfers ownership, leaves source not joinable.
TEST(CustomThreadTest, MoveConstructTransfersOwnership)
{
    // NOLINTBEGIN(bugprone-use-after-move): deliberate test subject

    std::atomic<bool> called{ false };

    thread_t source([&flag = called]() { flag.store(true, std::memory_order_release); });
    thread_t dest(std::move(source));

    EXPECT_FALSE(source.joinable());
    EXPECT_TRUE(dest.joinable());

    dest.join();

    EXPECT_TRUE(called.load(std::memory_order_acquire));

    // NOLINTEND(bugprone-use-after-move)
}

/// \brief Test that move assigning onto an already running \ref thread_t joins side the old thread first, ensuring its
///     side effects have completed before it is replaced.
TEST(CustomThreadTest, MoveAssignOntoRunningThreadJoinsOldThreadFirst)
{
    std::atomic<bool> proceed{ false };
    std::atomic<bool> oldCompleted{ false };
    std::atomic<bool> newCompleted{ false };

    thread_t target([&gate = proceed, &flag = oldCompleted]() {
        gate.wait(false);
        flag.store(true, std::memory_order_release);
    });
    thread_t replacement([&flag = newCompleted]() { flag.store(true, std::memory_order_release); });

    proceed.store(true, std::memory_order_release);
    proceed.notify_all();

    target = std::move(replacement);

    EXPECT_TRUE(oldCompleted.load(std::memory_order_acquire));

    target.join();

    EXPECT_TRUE(newCompleted.load(std::memory_order_acquire));
}

/// \brief Test that separate \ref thread_t instances have independent stop flags.
TEST(CustomThread, IndependentInstancesHaveIndependentStopFlags)
{
    std::atomic<bool> go{ false };
    std::atomic<bool> firstStopped{ false };
    std::atomic<bool> secondStopped{ false };

    thread_t first([&gate = go, &flag = firstStopped](stop_token_t token) {
        gate.wait(false);
        flag.store(token.stop_requested(), std::memory_order_release);
    });
    thread_t second([&gate = go, &flag = secondStopped](stop_token_t token) {
        gate.wait(false);
        flag.store(token.stop_requested(), std::memory_order_release);
    });

    first.request_stop();

    go.store(true, std::memory_order_release);
    go.notify_all();

    first.join();
    second.join();

    EXPECT_TRUE(firstStopped.load(std::memory_order_acquire));
    EXPECT_FALSE(secondStopped.load(std::memory_order_acquire));
}

} // namespace ful::testing
