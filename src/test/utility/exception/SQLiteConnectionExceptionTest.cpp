#include "utility/exception/SQLiteConnectionException.hpp"
#include "utility/exception/Exception.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <exception>
#include <filesystem>
#include <optional>
#include <source_location>
#include <type_traits>
#include <utility>

namespace ful::testing
{

namespace
{

constexpr auto MESSAGE{ "message" };
const auto DB_PATH{ std::filesystem::path(TEST_RESOURCE_DIR) };
constexpr auto LOCATION{ std::source_location::current() };

} // namespace

/// \brief Test that \ref SQLiteConnectionException can be created without specifying a database path.
TEST(SQLiteConnectionExceptionTest, CreateMessageWithoutDatabsePath)
{
    const auto exc{ SQLiteConnectionException::create(MESSAGE) };

    EXPECT_FALSE(exc.dbPath().has_value());
    EXPECT_EQ(exc.dbPath(), std::nullopt);
}

/// \brief Test that \ref SQLiteConnectionException can be created with specifying a database path.
TEST(SQLiteConnectionExceptionTest, CreateMessageWithDatabsePath)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    const auto exc{ SQLiteConnectionException::create(MESSAGE, DB_PATH) };

    ASSERT_TRUE(exc.dbPath().has_value());
    EXPECT_EQ(*exc.dbPath(), DB_PATH);
    // NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Test that \ref SQLiteConnectionException stores a user-supplied message.
TEST(SQLiteConnectionExceptionTest, StoreMessage)
{
    const auto exc{ SQLiteConnectionException::create(MESSAGE) };

    EXPECT_THAT(exc.message(), ::testing::HasSubstr("SQLite Connection"));
    EXPECT_THAT(exc.message(), ::testing::HasSubstr(MESSAGE));
}

/// \brief Test that \ref SQLiteConnectionException stores the \ref std::source_location.
TEST(SQLiteConnectionExceptionTest, StoreLocation)
{
    const auto exc{ SQLiteConnectionException::create(MESSAGE, DB_PATH, LOCATION) };

    EXPECT_THAT(exc.where().file_name(), ::testing::HasSubstr(LOCATION.file_name()));
    EXPECT_THAT(exc.where().function_name(), ::testing::HasSubstr(LOCATION.function_name()));
    EXPECT_THAT(exc.where().line(), LOCATION.line());
}

/// \brief Test that \ref SQLiteConnectionException::what() contains the user-supplied message.
TEST(SQLiteConnectionExceptionTest, WhatContainsMessage)
{
    const auto exc{ SQLiteConnectionException::create(MESSAGE) };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr(MESSAGE));
}

/// \brief Test that \ref SQLiteConnectionException::what() contains the \ref std::source_location.
TEST(SQLiteConnectionExceptionTest, WhatContainsLocation)
{
    const auto exc{ SQLiteConnectionException::create(MESSAGE) };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr(LOCATION.file_name()));
    EXPECT_THAT(exc.what(), ::testing::HasSubstr(LOCATION.function_name()));
}

/// \brief Test that \ref SQLiteConnectionException::what() contains the database path if one was specified.
TEST(SQLiteConnectionExceptionTest, WhatContainsDatabaseLocationIfSupplied)
{
    const auto exc{ SQLiteConnectionException::create(MESSAGE, DB_PATH) };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr(DB_PATH));
}

/// \brief Test that \ref SQLiteConnectionException handles an empty user-supplied message correctly when no database
///     path was provided.
TEST(SQLiteConnectionExceptionTest, HandleEmptyMessageWithoutDatabasePath)
{
    const auto exc{ SQLiteConnectionException::create("") };

    ASSERT_FALSE(exc.message().empty());
    EXPECT_NE(exc.what(), nullptr);
    EXPECT_THAT(exc.what(), ::testing::HasSubstr("SQLite Connection error"));
}

/// \brief Test that \ref SQLiteConnectionException handles an empty user-supplied message correctly when a database
///     path was provided.
TEST(SQLiteConnectionExceptionTest, HandleEmptyMessageWithDatabasePath)
{
    const auto exc{ SQLiteConnectionException::create("", DB_PATH) };

    ASSERT_FALSE(exc.message().empty());
    EXPECT_NE(exc.what(), nullptr);
    EXPECT_THAT(exc.what(), ::testing::HasSubstr("SQLite Connection error"));
    EXPECT_THAT(exc.what(), ::testing::HasSubstr(DB_PATH));
}
/// \brief Test that \ref SQLiteConnectionException is copy-constructible when no database path is specified.
TEST(SQLiteConnectionExceptionTest, CopyConstructionWithoutDatabasePath)
{
    const auto original{ SQLiteConnectionException::create(MESSAGE) };
    const auto copy{ original };

    EXPECT_EQ(copy.message(), original.message());
    EXPECT_EQ(copy.dbPath(), original.dbPath());
    EXPECT_EQ(copy.where().file_name(), original.where().file_name());
    EXPECT_EQ(copy.where().function_name(), original.where().function_name());
    EXPECT_EQ(copy.where().line(), original.where().line());
    EXPECT_EQ(std::string(copy.what()), std::string(original.what()));
}

/// \brief Test that \ref SQLiteConnectionException is copy-constructible when a database path is specified.
TEST(SQLiteConnectionExceptionTest, CopyConstructionWithDatabasePath)
{
    const auto original{ SQLiteConnectionException::create(MESSAGE) };
    const auto copy{ original };

    EXPECT_EQ(copy.message(), original.message());
    EXPECT_EQ(copy.dbPath(), original.dbPath());
    EXPECT_EQ(copy.where().file_name(), original.where().file_name());
    EXPECT_EQ(copy.where().function_name(), original.where().function_name());
    EXPECT_EQ(copy.where().line(), original.where().line());
    EXPECT_EQ(std::string(copy.what()), std::string(original.what()));
}

/// \brief Test that \ref SQLiteConnectionException is move-constructible when no database path is specified.
TEST(SQLiteConnectionExceptionTest, MoveConstructionWithoutDatabasePath)
{
    auto original{ SQLiteConnectionException::create(MESSAGE) };

    const std::string expectedMessage{ original.message() };
    const bool expectedPath{ original.dbPath().has_value() };
    const std::source_location expectedLocation{ original.where() };
    const std::string expectedWhat{ original.what() };

    const auto copy{ std::move(original) };

    EXPECT_EQ(copy.message(), expectedMessage);
    EXPECT_EQ(copy.dbPath().has_value(), expectedPath);
    EXPECT_EQ(copy.dbPath(), std::nullopt);
    EXPECT_EQ(copy.where().file_name(), expectedLocation.file_name());
    EXPECT_EQ(copy.where().function_name(), expectedLocation.function_name());
    EXPECT_EQ(copy.where().line(), expectedLocation.line());
    EXPECT_EQ(std::string(copy.what()), expectedWhat);
}

/// \brief Test that \ref SQLiteConnectionException is move-constructible when a database path is specified.
TEST(SQLiteConnectionExceptionTest, MoveConstructionWitDatabasePath)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    auto original{ SQLiteConnectionException::create(MESSAGE, DB_PATH) };

    const std::string expectedMessage{ original.message() };
    const bool expectedPathHasValue{ original.dbPath().has_value() };
    const std::filesystem::path expectedPath{ *original.dbPath() };
    const std::source_location expectedLocation{ original.where() };
    const std::string expectedWhat{ original.what() };

    const auto copy{ std::move(original) };

    EXPECT_EQ(copy.message(), expectedMessage);
    EXPECT_EQ(copy.dbPath().has_value(), expectedPathHasValue);
    EXPECT_EQ(*copy.dbPath(), expectedPath);
    EXPECT_EQ(copy.where().file_name(), expectedLocation.file_name());
    EXPECT_EQ(copy.where().function_name(), expectedLocation.function_name());
    EXPECT_EQ(copy.where().line(), expectedLocation.line());
    EXPECT_EQ(std::string(copy.what()), expectedWhat);
    // NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Test that \ref SQLiteConnectionException can be caught as \ref Exception and \ref std::exception.
TEST(SQLiteConnectionExceptionTest, CanBeCaughtAsStdException)
{
    ASSERT_TRUE((std::is_base_of_v<std::exception, SQLiteConnectionException>));
    ASSERT_TRUE((std::is_base_of_v<Exception, SQLiteConnectionException>));
    EXPECT_THAT(
        [] { throw Exception{ MESSAGE }; }, ::testing::ThrowsMessage<std::exception>(::testing::HasSubstr(MESSAGE))
    );
    EXPECT_THAT([] { throw Exception{ MESSAGE }; }, ::testing::ThrowsMessage<Exception>(::testing::HasSubstr(MESSAGE)));
}

} // namespace ful::testing
