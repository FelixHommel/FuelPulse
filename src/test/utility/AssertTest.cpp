#include "utility/Assert.hpp"

#include <gtest/gtest.h>

namespace ful::testing
{

#if FUL_ENABLE_ASSERTIONS

/// \brief Test that a passing \ref FUL_ASSERT condition has no effect on control flow.
TEST(AssertDeathTest, PassingConditionDoesNotAbort)
{
    EXPECT_NO_FATAL_FAILURE(FUL_ASSERT(1 == 1));
}

/// \brief Test that \ref FUL_ASSERT aborts the program when its condition is false.
TEST(AssertDeathTest, FailingConditionAborts)
{
    EXPECT_DEATH(FUL_ASSERT(1 == 2), "ful assertion failed");
}

/// \brief Test that the optional user message is included in the abort output.
TEST(AssertDeathTest, FailingConditionIncludesUserMessage)
{
    constexpr auto MESSAGE{ "custom message" };

    EXPECT_DEATH(FUL_ASSERT(false, MESSAGE), MESSAGE);
}

/// \brief Test that the abort output captures the location of the FUL_ASSERT call site.
TEST(AssertDeathTest, FailureOutputIncludesCallerLocation)
{
    EXPECT_DEATH(FUL_ASSERT(false), "AssertTest.cpp");
}

#endif // !FUL_ENABLE_ASSERTIONS

/// \brief Test that \ref ful::assertion::msgOrNull() with no argument yields nullptr.
TEST(AssertMsgOrNullTest, NoArgumentReturnsNullptr)
{
    EXPECT_EQ(ful::assertion::msgOrNull(), nullptr);
}

/// \brief Test that \ref ful::assertion::msgOrNull() with an argument returns that same argument unchanged.
TEST(AssertMsgOrNullTest, WithArgumentReturnsSamePointer)
{
    constexpr auto MESSAGE{ "message" };

    EXPECT_EQ(ful::assertion::msgOrNull(MESSAGE), MESSAGE);
}

} // namespace ful::testing
