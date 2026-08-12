#include "utility/exception/ConfigValidationException.hpp"
#include "utility/exception/Exception.hpp"
#include "utility/validation/Validator.hpp"

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

/// \brief Test that \ref ConfigValidationException stores the \ref std::source_location correctly.
TEST(ConfigValidationExceptionTest, StoreLocation)
{
    const auto exc{ ConfigValidationException::create({}, LOCATION) };

    EXPECT_THAT(exc.where().file_name(), ::testing::HasSubstr(LOCATION.file_name()));
    EXPECT_THAT(exc.where().function_name(), ::testing::HasSubstr(LOCATION.function_name()));
    EXPECT_THAT(exc.where().line(), LOCATION.line());
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
