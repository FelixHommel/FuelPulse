#include "utility/exception/WebRequestException.hpp"
#include "utility/HttpErrorCodes.hpp"
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

constexpr auto ERROR{ http::HttpCode::BadRequest };
constexpr auto LOCATION{ std::source_location::current() };

} // namespace

/// \brief Test that \ref FileIOException stores the user-supplied message.
TEST(WebRequestExceptionTest, StoreErrorCode)
{
    const auto exc{ WebRequestException::create(ERROR) };

    EXPECT_THAT(exc.message(), ::testing::HasSubstr(http::httpCodeToString(ERROR)));
    EXPECT_THAT(
        exc.message(), ::testing::HasSubstr(std::to_string(static_cast<std::underlying_type_t<http::HttpCode>>(ERROR)))
    );
    EXPECT_EQ(exc.errorCode(), ERROR);
}

/// \brief Test that \ref WebRequestException stores the \ref std::source_location.
TEST(WebRequestExceptionTest, StoreSourceLocation)
{
    const auto exc{ WebRequestException::create(ERROR, LOCATION) };

    EXPECT_THAT(exc.where().file_name(), ::testing::HasSubstr(std::string_view{ LOCATION.file_name() }));
    EXPECT_THAT(exc.where().function_name(), ::testing::HasSubstr(std::string_view{ LOCATION.function_name() }));
    EXPECT_THAT(exc.where().line(), LOCATION.line());
}

/// \brief Test that \ref WebRequestException provides the user-supplied message in \ref FileIOException::what().
TEST(WebRequestExceptionTest, WhatContainsErroCode)
{
    const auto exc{ WebRequestException::create(ERROR) };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr(http::httpCodeToString(ERROR)));
    EXPECT_THAT(
        exc.what(), ::testing::HasSubstr(std::to_string(static_cast<std::underlying_type_t<http::HttpCode>>(ERROR)))
    );
}

/// \brief Test that \ref WebRequestException provides the \ref std::source_location in \ref FileIOException::what().
TEST(WebRequestExceptionTest, WhatContainsSourceLocation)
{
    const auto exc{ WebRequestException::create(ERROR, LOCATION) };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr(std::string_view{ LOCATION.file_name() }));
    EXPECT_THAT(exc.what(), ::testing::HasSubstr(std::string_view{ LOCATION.function_name() }));
}

/// \brief Test that \ref WebRequestException is copy-constructible.
TEST(WebRequestExceptionTest, CopyConstruction)
{
    const auto original{ WebRequestException::create(ERROR) };
    const auto copy{ original };

    EXPECT_EQ(copy.message(), original.message());
    EXPECT_EQ(copy.where().file_name(), original.where().file_name());
    EXPECT_EQ(copy.where().function_name(), original.where().function_name());
    EXPECT_EQ(copy.where().line(), original.where().line());
    EXPECT_EQ(std::string(copy.what()), std::string(original.what()));
}

/// \brief Test that \ref WebRequestException is move-constructible.
TEST(WebRequestExceptionTest, MoveConstruction)
{
    auto original{ WebRequestException::create(ERROR) };

    const std::string expectedMessage{ original.message() };
    const std::source_location expectedLocation{ original.where() };
    const std::string expectedWhat{ original.what() };

    const auto copy{ std::move(original) };

    EXPECT_EQ(copy.message(), expectedMessage);
    EXPECT_EQ(copy.where().file_name(), expectedLocation.file_name());
    EXPECT_EQ(copy.where().function_name(), expectedLocation.function_name());
    EXPECT_EQ(copy.where().line(), expectedLocation.line());
    EXPECT_EQ(std::string(copy.what()), expectedWhat);
}

/// \brief Test that \ref WebRequestException can be caught as \ref Exception and \ref std::exception.
TEST(WebRequestExceptionTest, CanBeCaughtAsStdException)
{
    ASSERT_TRUE((std::is_base_of_v<std::exception, WebRequestException>));
    ASSERT_TRUE((std::is_base_of_v<Exception, WebRequestException>));
    EXPECT_THAT(
        [] { throw WebRequestException::create(ERROR); },
        ::testing::ThrowsMessage<std::exception>(::testing::HasSubstr(http::httpCodeToString(ERROR)))
    );
    EXPECT_THAT(
        [] { throw WebRequestException::create(ERROR); },
        ::testing::ThrowsMessage<Exception>(::testing::HasSubstr(http::httpCodeToString(ERROR)))
    );
}

} // namespace ful::testing
