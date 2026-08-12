#include "utility/exception/Exception.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <exception>
#include <source_location>
#include <string>
#include <type_traits>
#include <utility>

namespace ful::testing
{

namespace
{

constexpr auto MESSAGE{ "test" };
constexpr auto LOCATION{ std::source_location::current() };

} // namespace

/// \brief Test that \ref Exception stores the user-supplied message correctly.
TEST(ExceptionTest, StoreMessage)
{
    const Exception exc{ MESSAGE };

    EXPECT_EQ(exc.message(), MESSAGE);
}

/// \brief Test that \ref Exception stores the correct call location.
TEST(ExceptionTest, StoreLocation)
{
    const Exception exc{ MESSAGE, LOCATION };

    EXPECT_EQ(exc.where().file_name(), LOCATION.file_name());
    EXPECT_EQ(exc.where().function_name(), LOCATION.function_name());
    EXPECT_EQ(exc.where().line(), LOCATION.line());
}

/// \brief Test that \ref Exception contains the user-supplied message in \ref Exception::what().
TEST(ExceptionTest, WhatContainsMessage)
{
    const Exception exc{ MESSAGE };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr(MESSAGE));
}

/// \brief Test that \ref Exception contains the \ref std::source_location message in \ref Exception::what().
TEST(ExceptionTest, WhatContainsSourceLocation)
{
    const Exception exc{ MESSAGE, LOCATION };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr(LOCATION.file_name()));
    EXPECT_THAT(exc.what(), ::testing::HasSubstr(LOCATION.function_name()));
}

/// \brief Test that \ref Exception handles an empty user-supplied Message correctly.
TEST(ExceptionTest, HandleEmptyMessage)
{
    const Exception exc{ "" };

    ASSERT_TRUE(exc.message().empty());
    EXPECT_NE(exc.what(), nullptr);
}

/// \brief Test that \ref Exception can be copy-constructed.
TEST(ExceptionTest, CopyConstruction)
{
    const Exception original{ MESSAGE };
    const Exception copy{ original };

    EXPECT_EQ(copy.message(), original.message());
    EXPECT_EQ(copy.where().file_name(), original.where().file_name());
    EXPECT_EQ(std::string(copy.what()), std::string(original.what()));
}

/// \brief Test that \ref Exception can be move-constructed.
TEST(ExceptionTest, MoveConstruction)
{
    Exception original{ MESSAGE };

    const auto expectedMessage{ original.message() };
    const auto expectedWhat{ std::string(original.what()) };

    const Exception copy{ std::move(original) };

    EXPECT_EQ(copy.message(), expectedMessage);
    EXPECT_EQ(std::string(copy.what()), expectedWhat);
}

/// \brief Test that \ref Exception can be caught as \ref std::exception.
TEST(ExceptionTest, CanBeCaughtAsStdException)
{
    ASSERT_TRUE((std::is_base_of_v<std::exception, Exception>));
    EXPECT_THAT(
        [] { throw Exception{ MESSAGE }; }, ::testing::ThrowsMessage<std::exception>(::testing::HasSubstr(MESSAGE))
    );
}

#if FUL_USE_STACKTRACE

/// \brief Test that \ref Exception gives access to the stacktrace of where the exception was caused.
TEST(ExceptionTest, ContainsStacktraceInWhatMessage)
{
    const Exception exc{ MESSAGE };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr("Stacktrace"));
}

#endif

} // namespace ful::testing
