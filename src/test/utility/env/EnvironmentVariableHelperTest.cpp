#include <gtest/gtest.h>

#include "testUtility/EnvVarGuard.hpp"
#include "utility/env/EnvironmentVariableHelper.hpp"

#include <tuple>

namespace ful::testing
{

/// \brief Test that trying to get an environment variable's value when the variable does not exist returns a \ref std::nullopt.
TEST(EnvironmentVariableHelperGetVarTest, ReturnsNulloptForUnsetVar)
{
    constexpr auto TEST_VAR_NAME{ "FUL_TEST_DEFINITELY_UNSET_VAR" };

    EXPECT_FALSE(env::getVar(TEST_VAR_NAME).has_value());
}

/// \brief Test that getting an environment variable's value when the variable exists results in a \ref std::optional
///     that contains the variables value
TEST(EnvironmentVariableHelperGetVarTest, ReturnsValueForSetVariable)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Not really unchecked here.
    constexpr auto TEST_VAR_NAME{ "FUL_TEST_VAR" };
    constexpr auto TEST_VAR_VALUE{ "test" };
    EnvVarGuard guard{ TEST_VAR_NAME, TEST_VAR_VALUE };

    const auto value{ env::getVar(TEST_VAR_NAME) };

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, TEST_VAR_VALUE);
    // NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Test that an empty-string value is distinguished from unsetting an environment variable on POSIX systems.
///
/// \note Windows systems using _putenv_s do not behave the same as POSIX systems when passing a "" as value. This test
///     is just supposed to test the POSIX behavior itself and not an empty-write.
TEST(EnvironmentVariableHelperGetVarTest, EmptyStringValueIsDistinguishedFromUnset)
{
#ifndef _WIN32
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Not really unchecked here.
    constexpr auto TEST_VAR_NAME{ "FUL_TEST_EMPTY_VAR" };
    constexpr auto TEST_VAR_VALUE{ "" };
    EnvVarGuard guard{ TEST_VAR_NAME, TEST_VAR_VALUE };

    const auto value{ env::getVar(TEST_VAR_NAME) };

    ASSERT_TRUE(value.has_value());
    EXPECT_TRUE((*value).empty());
    // NOLINTEND(bugprone-unchecked-optional-access)
#else
    GTEST_SKIP() << "writeVar('') removes the environment variable on windows";
#endif
}

/// \brief Test that a variable that did not exist before becomes visible after writing to \ref env::getVar().
TEST(EnvironmentVariableHelperWriteVarTest, WriteThenGetRoundTrip)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Not really unchecked here.
    constexpr auto TEST_VAR_NAME{ "FUL_TEST_ROUNDTRIP_VAR" };
    constexpr auto TEST_VAR_VALUE{ "test" };

    env::unsetVar(TEST_VAR_NAME);

    EXPECT_EQ(env::writeVar(TEST_VAR_NAME, TEST_VAR_VALUE), 0);

    const auto value{ env::getVar(TEST_VAR_NAME) };

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, TEST_VAR_VALUE);

    env::unsetVar(TEST_VAR_NAME);
    // NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Test that writing to a variable that already exists overwrites the value of the variable.
TEST(EnvironmentVariableHelperWriteVarTest, OverwritesExistingValue)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Not really unchecked here.
    constexpr auto TEST_VAR_NAME{ "FUL_TEST_OVERWRITE_VAR" };
    constexpr auto TEST_VAR_VALUE_ORIGINAL{ "original" };
    constexpr auto TEST_VAR_VALUE_REPLACED{ "replaced" };
    EnvVarGuard guard{ TEST_VAR_NAME, TEST_VAR_VALUE_ORIGINAL };

    EXPECT_EQ(env::writeVar(TEST_VAR_NAME, TEST_VAR_VALUE_REPLACED), 0);

    const auto value{ env::getVar(TEST_VAR_NAME) };

    ASSERT_TRUE(value.has_value());
    EXPECT_EQ(*value, TEST_VAR_VALUE_REPLACED);
    // NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Test that unsetting a previously set variable removes it entirely.
TEST(EnvironmentVariableHelperUnsetVarTest, RemovesPreviouslySetVariable)
{
    constexpr auto TEST_VAR_NAME{ "FUL_TEST_UNSET_VAR" };
    constexpr auto TEST_VAR_VALUE{ "" };
    EnvVarGuard guard{ TEST_VAR_NAME, TEST_VAR_VALUE };

    ASSERT_TRUE(env::getVar(TEST_VAR_NAME).has_value());

    EXPECT_EQ(env::unsetVar(TEST_VAR_NAME), 0);
    EXPECT_FALSE(env::getVar(TEST_VAR_NAME).has_value());
}

/// \brief Test that unsetting a variable that does not exist does not cause any errors or throws any exceptions.
TEST(EnvironmentVariableHelperUnsetVarTest, UnsettingNeverSetVariableIsSafe)
{
    constexpr auto TEST_VAR_NAME{ "FUL_TEST_DEFINITELY_UNSET_VAR" };

    EXPECT_NO_THROW(std::ignore = env::unsetVar(TEST_VAR_NAME));
}

} // namespace ful::testing
