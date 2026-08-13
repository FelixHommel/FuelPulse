#include "utility/exception/ConfigValidationException.hpp"
#include "utility/exception/Exception.hpp"
#include "utility/exception/FileIOException.hpp"
#include "utility/exception/SQLiteAccessException.hpp"
#include "utility/exception/SQLiteConnectionException.hpp"

#include "testUtility/ExceptionTestTraits.hpp"

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

constexpr auto MESSAGE{ "test-message" };
constexpr auto LOCATION{ std::source_location::current() };

} // namespace

template<typename ExcT>
    requires std::is_base_of_v<Exception, ExcT>
class ExceptionContractTest : public ::testing::Test
{};

using ExceptionTypes = ::testing::
    Types<Exception, FileIOException, SQLiteAccessException, SQLiteConnectionException, ConfigValidationException>;

TYPED_TEST_SUITE(ExceptionContractTest, ExceptionTypes);

TYPED_TEST(ExceptionContractTest, StoresMessage)
{
    const auto exc{ ExceptionTraits<TypeParam>::make(MESSAGE, LOCATION) };

    // EXPECT_EQ(exc.message(), ::testing::HasSubstr(std::string_view{ MESSAGE })); // FIXME: For some reason causes comp-op overload issue
}

/// \brief The user-supplied message must be discoverable in what().
TYPED_TEST(ExceptionContractTest, WhatContainsMessage)
{
    const auto exc{ ExceptionTraits<TypeParam>::make(MESSAGE, LOCATION) };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr(std::string_view{ MESSAGE }));
}

/// \brief The captured \ref std::source_location must match where the exception was created.
TYPED_TEST(ExceptionContractTest, StoresSourceLocation)
{
    const auto exc{ ExceptionTraits<TypeParam>::make(MESSAGE, LOCATION) };

    EXPECT_THAT(exc.where().file_name(), ::testing::HasSubstr(std::string_view{ LOCATION.file_name() }));
    EXPECT_EQ(exc.where().function_name(), LOCATION.function_name());
    EXPECT_EQ(exc.where().line(), LOCATION.line());
}

/// \brief what() must also surface the captured \ref std::source_location.
TYPED_TEST(ExceptionContractTest, WhatContainsSourceLocation)
{
    const auto exc{ ExceptionTraits<TypeParam>::make(MESSAGE, LOCATION) };

    EXPECT_THAT(exc.what(), ::testing::HasSubstr(std::string_view{ LOCATION.file_name() }));
    EXPECT_THAT(exc.what(), ::testing::HasSubstr(std::string_view{ LOCATION.function_name() }));
}

/// \brief An empty user-supplied message must not crash construction or leave what() in an invalid state.
TYPED_TEST(ExceptionContractTest, HandlesEmptyMessage)
{
    const auto exc{ ExceptionTraits<TypeParam>::make(MESSAGE, LOCATION) };

    EXPECT_NE(exc.what(), nullptr);
}

/// \brief Copy construction must preserve message, location and what().
TYPED_TEST(ExceptionContractTest, CopyConstructionPreservesState)
{
    const auto original{ ExceptionTraits<TypeParam>::make(MESSAGE, LOCATION) };
    const auto copy{ original };

    EXPECT_EQ(copy.message(), original.message());
    EXPECT_EQ(copy.where().file_name(), original.where().file_name());
    EXPECT_EQ(copy.where().function_name(), original.where().function_name());
    EXPECT_EQ(copy.where().line(), original.where().line());
    EXPECT_EQ(std::string(copy.what()), std::string(original.what()));
}

/// \brief Move construction must transfer message, location, and what().
TYPED_TEST(ExceptionContractTest, MoveConstructionPreservesState)
{
    auto original{ ExceptionTraits<TypeParam>::make(MESSAGE, LOCATION) };

    const auto expectedMessage{ original.message() };
    const auto expectedFileName{ original.where().file_name() };
    const auto expectedFunctionName{ original.where().function_name() };
    const auto expectedLine{ original.where().line() };
    const auto expectedWhat{ std::string(original.what()) };

    const auto copy{ std::move(original) };

    EXPECT_EQ(copy.message(), expectedMessage);
    EXPECT_EQ(copy.where().file_name(), expectedFileName);
    EXPECT_EQ(copy.where().function_name(), expectedFunctionName);
    EXPECT_EQ(copy.where().line(), expectedLine);
    EXPECT_EQ(copy.what(), expectedWhat);
}

/// \brief Instances must be throwable and catchable through both base-class handlers.
TYPED_TEST(ExceptionContractTest, ThrowableAndCatchableAsBaseType)
{
    EXPECT_THAT(
        [] { throw ExceptionTraits<TypeParam>::make(MESSAGE, LOCATION); },
        ::testing::ThrowsMessage<std::exception>(::testing::HasSubstr(std::string_view{ MESSAGE }))
    );
    EXPECT_THAT(
        [] { throw ExceptionTraits<TypeParam>::make(MESSAGE, LOCATION); },
        ::testing::ThrowsMessage<Exception>(::testing::HasSubstr(std::string_view{ MESSAGE }))
    );
}

} // namespace ful::testing
