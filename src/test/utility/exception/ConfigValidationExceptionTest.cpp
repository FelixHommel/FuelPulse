#include "utility/exception/ConfigValidationException.hpp"
#include "utility/exception/Exception.hpp"
#include "utility/validation/Validator.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json_fwd.hpp>

#include <exception>
#include <source_location>
#include <string>
#include <type_traits>
#include <utility>

namespace ful::testing
{

namespace
{

constexpr auto LOCATION{ std::source_location::current() };

} // namespace

/// \brief Test that \ref ConfigValidationException creates a message even when no config errors are supplied.
TEST(ConfigValidationExceptionTest, StoreEmptyErrorList)
{
    const auto exc{ ConfigValidationException::create({}) };

    ASSERT_FALSE(exc.message().empty());
    EXPECT_THAT(exc.message(), ::testing::HasSubstr("Failed to validate config"));
    EXPECT_TRUE(exc.errors().empty());
}

/// \brief Test that \ref ConfigValidationException stores the errors it was provided correctly.
TEST(ConfigValidationException, StoreErrors)
{
    constexpr auto PTR_PATH{ "/test" };
    constexpr auto MESSAGE{ "test message" };
    const Validator::ValidationErrors ERRORS{
        { .path = nlohmann::json::json_pointer{ PTR_PATH }, .message = MESSAGE }
    };

    const auto exc{ ConfigValidationException::create(ERRORS) };

    ASSERT_EQ(exc.errors().size(), 1);
    EXPECT_EQ(exc.errors().at(0).path.to_string(), ERRORS.at(0).path.to_string());
    EXPECT_EQ(exc.errors().at(0).message, ERRORS.at(0).message);
}

/// \brief Test that \ref ConfigValidationException stores the \ref std::source_location correctly.
TEST(ConfigValidationExceptionTest, StoreLocation)
{
    const auto exc{ ConfigValidationException::create({}, LOCATION) };

    EXPECT_THAT(exc.where().file_name(), ::testing::HasSubstr(LOCATION.file_name()));
    EXPECT_THAT(exc.where().function_name(), ::testing::HasSubstr(LOCATION.function_name()));
    EXPECT_THAT(exc.where().line(), LOCATION.line());
}

/// \brief Test that \ref ConfigValidationException stores and uses a single \ref ValidationError correctly.
TEST(ConfigValidationExceptionTest, SingleErrorMessageContainsPathAndMessage)
{
    constexpr auto PTR_PATH{ "/test" };
    constexpr auto MESSAGE{ "test message" };
    const Validator::ValidationErrors ERRORS{
        { .path = nlohmann::json::json_pointer{ PTR_PATH }, .message = MESSAGE }
    };

    const auto exc{ ConfigValidationException::create(ERRORS) };

    EXPECT_THAT(exc.message(), ::testing::HasSubstr(PTR_PATH));
    EXPECT_THAT(exc.message(), ::testing::HasSubstr(MESSAGE));
}

/// \brief Test that \ref ConfigValidationException stores and uses multiple \ref ValidationError correctly.
TEST(ConfigValidationExceptionTest, MultipleErrorsAllAppearInMessage)
{
    constexpr auto PTR_PATH_BASE{ "/test_" };
    constexpr auto MESSAGE_BASE{ "test_message_" };
    const Validator::ValidationErrors ERRORS{
        { .path = nlohmann::json::json_pointer{ PTR_PATH_BASE + std::to_string(1) },
         .message = MESSAGE_BASE + std::to_string(1) },
        { .path = nlohmann::json::json_pointer{ PTR_PATH_BASE + std::to_string(2) },
         .message = MESSAGE_BASE + std::to_string(2) },
    };

    const auto exc{ ConfigValidationException::create(ERRORS) };

    for(const auto& e : ERRORS)
    {
        EXPECT_THAT(exc.message(), ::testing::HasSubstr(e.path.to_string()));
        EXPECT_THAT(exc.message(), ::testing::HasSubstr(e.message));
    }
}

/// \brief Test that \ref ConfigValidationException includes the validation errors in the
///     \ref ConfigValidationException::what() message.
TEST(ConfigValidationExceptionTest, WhatMessageContainsErrors)
{
    constexpr auto PTR_PATH{ "/test" };
    constexpr auto MESSAGE{ "test message" };
    const Validator::ValidationErrors ERRORS{
        { .path = nlohmann::json::json_pointer{ PTR_PATH }, .message = MESSAGE }
    };

    const auto exc{ ConfigValidationException::create(ERRORS) };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr(PTR_PATH));
    EXPECT_THAT(exc.what(), ::testing::HasSubstr(MESSAGE));
}

/// \brief Test that \ref ConfigValidationException can be copy-constructed.
TEST(ConfigValidationExceptionTest, CopyConstruction)
{
    const auto original{ ConfigValidationException::create({}) };
    const auto copy{ original };

    EXPECT_EQ(copy.message(), original.message());
    EXPECT_EQ(copy.where().file_name(), original.where().file_name());
    EXPECT_EQ(std::string(copy.what()), std::string(original.what()));
}

/// \brief Test that \ref ConfigValidationException can be move-constructed.
TEST(ConfigValidationExceptionTest, MoveConstruction)
{
    auto original{ ConfigValidationException::create({}) };

    const auto expectedMessage{ original.message() };
    const auto expectedWhat{ std::string(original.what()) };

    const auto copy{ std::move(original) };

    EXPECT_EQ(copy.message(), expectedMessage);
    EXPECT_EQ(std::string(copy.what()), expectedWhat);
}

/// \brief Test that \ref ConfigValidationException can be caught as \ref Exception and \ref std::exception.
TEST(ConfigValidationExceptionTest, CanBeCaughtAsException)
{
    ASSERT_TRUE((std::is_base_of_v<Exception, ConfigValidationException>));
    ASSERT_TRUE((std::is_base_of_v<std::exception, ConfigValidationException>));

    EXPECT_THAT(
        [] { throw ConfigValidationException::create({}); },
        ::testing::ThrowsMessage<Exception>(::testing::HasSubstr("Failed to validate config"))
    );
    EXPECT_THAT(
        [] { throw ConfigValidationException::create({}); },
        ::testing::ThrowsMessage<std::exception>(::testing::HasSubstr("Failed to validate config"))
    );
}

} // namespace ful::testing
