#include "utility/exception/SQLiteAccessException.hpp"
#include "utility/exception/Exception.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <exception>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace ful::testing
{

namespace
{

constexpr auto MESSAGE{ "test" };
constexpr auto LOCATION{ std::source_location::current() };

} // namespace

/// \brief Test that \ref SQLiteAccessException stores the user-supplied message correctly.
TEST(SQLiteAccessExceptionTest, StoreMessage)
{
    const auto exc{ SQLiteAccessException::create(MESSAGE) };

    EXPECT_THAT(exc.message(), ::testing::HasSubstr(std::string_view{ MESSAGE }));
}

/// \brief Test that \ref SQLiteAccessException stores the correct call location.
TEST(SQLiteAccessExceptionTest, StoreLocation)
{
    const auto exc{ SQLiteAccessException::create(MESSAGE, LOCATION) };

    EXPECT_EQ(exc.where().file_name(), LOCATION.file_name());
    EXPECT_EQ(exc.where().function_name(), LOCATION.function_name());
    EXPECT_EQ(exc.where().line(), LOCATION.line());
}

/// \brief Test that \ref SQLiteAccessException contains the user-supplied message in \ref Exception::what().
TEST(SQLiteAccessExceptionTest, WhatContainsMessage)
{
    const auto exc{ SQLiteAccessException::create(MESSAGE) };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr(std::string_view{ MESSAGE }));
}

/// \brief Test that \ref SQLiteAccessException contains the \ref std::source_location message in \ref Exception::what().
TEST(SQLiteAccessExceptionTest, WhatContainsSourceLocation)
{
    const auto exc{ SQLiteAccessException::create(MESSAGE, LOCATION) };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr(std::string_view{ LOCATION.file_name() }));
    EXPECT_THAT(exc.what(), ::testing::HasSubstr(std::string_view{ LOCATION.function_name() }));
}

/// \brief Test that \ref SQLiteAccessException handles an empty user-supplied Message correctly.
TEST(SQLiteAccessExceptionTest, HandleEmptyMessage)
{
    const auto exc{ SQLiteAccessException::create("") };

    ASSERT_FALSE(exc.message().empty());
    EXPECT_NE(exc.what(), nullptr);
    EXPECT_THAT(exc.what(), ::testing::HasSubstr("SQLite access error"));
}

/// \brief Test that \ref SQLiteAccessException can be copy-constructed.
TEST(SQLiteAccessExceptionTest, CopyConstruction)
{
    const auto original{ SQLiteAccessException::create(MESSAGE) };
    const auto copy{ original };

    EXPECT_EQ(copy.message(), original.message());
    EXPECT_EQ(copy.where().file_name(), original.where().file_name());
    EXPECT_EQ(std::string(copy.what()), std::string(original.what()));
}

/// \brief Test that \ref SQLiteAccessException can be move-constructed.
TEST(SQLiteAccessExceptionTest, MoveConstruction)
{
    auto original{ SQLiteAccessException::create(MESSAGE) };

    const auto expectedMessage{ original.message() };
    const auto expectedWhat{ std::string(original.what()) };

    const auto copy{ std::move(original) };

    EXPECT_EQ(copy.message(), expectedMessage);
    EXPECT_EQ(std::string(copy.what()), expectedWhat);
}

/// \brief Test that \ref SQLiteAccessException can be caught as \ref Exception and \ref std::exception.
TEST(SQLiteAccessExceptionTest, CanBeCaughtAsStdException)
{
    ASSERT_TRUE((std::is_base_of_v<std::exception, SQLiteAccessException>));
    ASSERT_TRUE((std::is_base_of_v<Exception, SQLiteAccessException>));
    EXPECT_THAT(
        [] { throw Exception{ MESSAGE }; },
        ::testing::ThrowsMessage<std::exception>(::testing::HasSubstr(std::string_view{ MESSAGE }))
    );
    EXPECT_THAT(
        [] { throw Exception{ MESSAGE }; },
        ::testing::ThrowsMessage<Exception>(::testing::HasSubstr(std::string_view{ MESSAGE }))
    );
}

} // namespace ful::testing
