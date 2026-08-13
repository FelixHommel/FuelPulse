#include "utility/hash/HashCombine.hpp"

#include <gtest/gtest.h>

#include <cstddef>

namespace ful::testing
{

/// \brief Test that the same input to the hashCombine function gives the same seed for both function calls.
TEST(HashCombineTest, SameInputProduceSameSeed)
{
    static constexpr auto VALUE{ "value" };

    std::size_t seedA{ 0 };
    std::size_t seedB{ 0 };

    hash::hashCombine(seedA, VALUE);
    hash::hashCombine(seedB, VALUE);

    EXPECT_EQ(seedA, seedB);
}

/// \brief Test that different values combined with the same starting seed produce different results.
TEST(HashCombineTest, DifferentInputsProduceDifferentSeeds)
{
    constexpr auto VALUE_A{ "valueA" };
    constexpr auto VALUE_B{ "valueB" };

    std::size_t seedA{ 0 };
    std::size_t seedB{ 0 };

    hash::hashCombine(seedA, VALUE_A);
    hash::hashCombine(seedB, VALUE_B);

    EXPECT_NE(seedA, seedB);
}

/// \brief Test that \ref hash::hashCombine is sensitive to order.
TEST(HashCombineTest, OrderOfCombinationMatters)
{
    constexpr auto FIRST{ "first" };
    constexpr auto SECOND{ "second" };

    std::size_t seedA{ 0 };
    hash::hashCombine(seedA, FIRST);
    hash::hashCombine(seedA, SECOND);

    std::size_t seedB{ 0 };
    hash::hashCombine(seedB, SECOND);
    hash::hashCombine(seedB, FIRST);

    EXPECT_NE(seedA, seedB);
}

/// \brief Test that the starting seed itself affects the outcome.
TEST(HashCombineTest, StartingSeedAffectsResult)
{
    static constexpr auto VALUE{ "value" };

    std::size_t seedA{ 0 };
    std::size_t seedB{ 1 };

    hash::hashCombine(seedA, VALUE);
    hash::hashCombine(seedB, VALUE);

    EXPECT_NE(seedA, seedB);
}

} // namespace ful::testing
