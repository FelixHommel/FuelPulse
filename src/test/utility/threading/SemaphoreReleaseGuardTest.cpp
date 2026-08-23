#include "utility/threading/SemaphoreReleaseGuard.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <semaphore>

namespace ful::testing
{

namespace
{

constexpr auto ACQUIRE_TIMEOUT{ std::chrono::milliseconds(50) };

} // namespace

/// \brief Test that destroying a \ref SemaphoreReleaseGuard releases the wrapped semaphore exactly once.
TEST(SemaphoreReleaseGuardTest, DestructorReleasesSemaphore)
{
    std::binary_semaphore sem{ 0 };

    {
        SemaphoreReleaseGuard guard{ sem };
    }

    EXPECT_TRUE(sem.try_acquire_for(ACQUIRE_TIMEOUT));
}

/// \brief Test that a disarmed \ref SemaphoreReleaseGuard does not release the semaphore on destruction.
TEST(SemaphoreReleaseGuardTest, DisarmPreventsReleaseOnDestruction)
{
    std::binary_semaphore sem{ 0 };

    {
        SemaphoreReleaseGuard guard{ sem };
        guard.disarm();
    }

    EXPECT_FALSE(sem.try_acquire_for(ACQUIRE_TIMEOUT));
}

/// \brief Test that calling \ref SemaphoreReleaseGuard::disarm() more than once is safe and still results in exactly
///     one skipped release.
TEST(SemaphoreReleaseGuardTest, DisarmingMultipleTimesIsSafe)
{
    std::binary_semaphore sem{ 0 };

    {
        SemaphoreReleaseGuard guard{ sem };

        guard.disarm();
        guard.disarm();
    }

    EXPECT_FALSE(sem.try_acquire_for(ACQUIRE_TIMEOUT));
}

/// \brief Test that multiple independent guards over the same counting semaphores each contribute one release.
TEST(SemaphoreReleaseGuardTest, MultipleGuardsEachReleaseOnce)
{
    constexpr auto MAX_COUNT{ 5 };

    std::counting_semaphore<MAX_COUNT> sem{ 0 };

    {
        SemaphoreReleaseGuard guard1{ sem };
        SemaphoreReleaseGuard guard2{ sem };
        SemaphoreReleaseGuard guard3{ sem };
    }

    EXPECT_TRUE(sem.try_acquire_for(ACQUIRE_TIMEOUT));
    EXPECT_TRUE(sem.try_acquire_for(ACQUIRE_TIMEOUT));
    EXPECT_TRUE(sem.try_acquire_for(ACQUIRE_TIMEOUT));
    EXPECT_FALSE(sem.try_acquire_for(ACQUIRE_TIMEOUT));
}

/// \brief Test that disarming one guard does not suppress the release of sibling guards on the same semaphore.
TEST(SemaphoreReleaseGuardTest, DisarmingOneGuardDoesNotAffectOthers)
{
    constexpr auto MAX_COUNT{ 5 };

    std::counting_semaphore<MAX_COUNT> sem{ 0 };

    {
        SemaphoreReleaseGuard guard1{ sem };
        SemaphoreReleaseGuard guard2{ sem };

        guard2.disarm();
    }

    EXPECT_TRUE(sem.try_acquire_for(ACQUIRE_TIMEOUT));
    EXPECT_FALSE(sem.try_acquire_for(ACQUIRE_TIMEOUT));
}

} // namespace ful::testing
