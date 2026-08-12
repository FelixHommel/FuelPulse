#include "utility/exception/FileIOException.hpp"
#include "utility/exception/Exception.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <exception>
#include <filesystem>
#include <source_location>
#include <type_traits>
#include <utility>

namespace ful::testing
{

namespace
{

constexpr auto MESSAGE{ "test" };
const auto PATH{ std::filesystem::path(TEST_RESOURCE_DIR) };
constexpr auto LOCATION{ std::source_location::current() };

} // namespace

/// \brief Test that \ref FileIOException stores the user-supplied message.
TEST(FileIOExceptionTest, StoreMessage)
{
    const auto exc{ FileIOException::create(PATH, MESSAGE) };

    EXPECT_THAT(exc.message(), ::testing::HasSubstr(MESSAGE));
}

/// \brief Test that \ref FileIOException stores the \ref std::source_location.
TEST(FileIOExceptionTest, StoreSourceLocation)
{
    const auto exc{ FileIOException::create(PATH, MESSAGE, LOCATION) };

    EXPECT_THAT(exc.where().file_name(), ::testing::HasSubstr(LOCATION.file_name()));
    EXPECT_THAT(exc.where().function_name(), ::testing::HasSubstr(LOCATION.function_name()));
    EXPECT_THAT(exc.where().line(), LOCATION.line());
}

/// \brief Test that \ref FileIOException stores the \ref std::filesystem::path.
TEST(FileIOExceptionTest, StorePath)
{
    const auto exc{ FileIOException::create(PATH, MESSAGE) };

    EXPECT_EQ(exc.path(), PATH);
}

/// \brief Test that \ref FileIOException provides the user-supplied message in \ref FileIOException::what().
TEST(FileIOExceptionTest, WhatContainsMessage)
{
    const auto exc{ FileIOException::create(PATH, MESSAGE) };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr("File I/O"));
    EXPECT_THAT(exc.what(), ::testing::HasSubstr(MESSAGE));
}

/// \brief Test that \ref FileIOException provides the \ref std::source_location in \ref FileIOException::what().
TEST(FileIOExceptionTest, WhatContainsSourceLocation)
{
    const auto exc{ FileIOException::create(PATH, MESSAGE, LOCATION) };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr(LOCATION.file_name()));
    EXPECT_THAT(exc.what(), ::testing::HasSubstr(LOCATION.function_name()));
}

/// \brief Test that \ref FileIOException provides a message even with no specific user-supplied message.
TEST(FileIOExceptionTest, HandleEmptyMessage)
{
    const auto exc{ FileIOException::create(PATH, "") };

    ASSERT_FALSE(exc.message().empty());
    EXPECT_NE(exc.what(), nullptr);
}

/// \brief Test that \ref FileIOException is copy-constructible.
TEST(FileIOExceptionTest, CopyConstruction)
{
    const auto original{ FileIOException::create(PATH, MESSAGE) };
    const auto copy{ original };

    EXPECT_EQ(copy.message(), original.message());
    EXPECT_EQ(copy.path(), original.path());
    EXPECT_EQ(copy.where().file_name(), original.where().file_name());
    EXPECT_EQ(copy.where().function_name(), original.where().function_name());
    EXPECT_EQ(copy.where().line(), original.where().line());
    EXPECT_EQ(std::string(copy.what()), std::string(original.what()));
}

/// \brief Test that \ref FileIOException is move-constructible.
TEST(FileIOExceptionTest, MoveConstruction)
{
    auto original{ FileIOException::create(PATH, MESSAGE) };

    const std::string expectedMessage{ original.message() };
    const std::filesystem::path expectedPath{ original.path() };
    const std::source_location expectedLocation{ original.where() };
    const std::string expectedWhat{ original.what() };

    const auto copy{ std::move(original) };

    EXPECT_EQ(copy.message(), expectedMessage);
    EXPECT_EQ(copy.path(), expectedPath);
    EXPECT_EQ(copy.where().file_name(), expectedLocation.file_name());
    EXPECT_EQ(copy.where().function_name(), expectedLocation.function_name());
    EXPECT_EQ(copy.where().line(), expectedLocation.line());
    EXPECT_EQ(std::string(copy.what()), expectedWhat);
}

/// \brief Test that \ref FileIOException can be caught as \ref Exception and \ref std::exception.
TEST(FileIOExceptionTest, CanBeCaughtAsStdException)
{
    ASSERT_TRUE((std::is_base_of_v<std::exception, FileIOException>));
    ASSERT_TRUE((std::is_base_of_v<Exception, FileIOException>));
    EXPECT_THAT(
        [] { throw Exception{ MESSAGE }; }, ::testing::ThrowsMessage<std::exception>(::testing::HasSubstr(MESSAGE))
    );
    EXPECT_THAT([] { throw Exception{ MESSAGE }; }, ::testing::ThrowsMessage<Exception>(::testing::HasSubstr(MESSAGE)));
}

} // namespace ful::testing
