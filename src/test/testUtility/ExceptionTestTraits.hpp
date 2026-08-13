#ifndef FUL_SRC_TEST_TEST_UTILITY_EXCEPTION_TEST_TRAITS_HPP
#define FUL_SRC_TEST_TEST_UTILITY_EXCEPTION_TEST_TRAITS_HPP

#include "utility/exception/ConfigValidationException.hpp"
#include "utility/exception/Exception.hpp"
#include "utility/exception/FileIOException.hpp"
#include "utility/exception/SQLiteAccessException.hpp"
#include "utility/exception/SQLiteConnectionException.hpp"
#include "utility/validation/Validator.hpp"

#include <filesystem>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace ful::testing
{

/// \brief Uniform factory for constructing each \ref Exception subtype from just a message and a
///     \ref std::source_location, so behavior common to the whole hierarchy can be tested generically through
///     \ref ExceptionContractTest.
///
/// Specialize this template for any new \ref Exception subtype to pull it into the shared contract tests.
///
/// \tparam ExcT The concrete exception type in the \ref Exception hierarchy that is being constructed
///
/// \author Felix Hommel
/// \date 8/12/2026
template<typename ExcT>
    requires std::is_base_of_v<Exception, ExcT>
struct ExceptionTraits;

template<>
struct ExceptionTraits<Exception>
{
    static Exception make(std::string_view message, std::source_location loc)
    {
        return Exception{ std::string(message), loc };
    }
};

template<>
struct ExceptionTraits<FileIOException>
{
    static FileIOException make(std::string_view message, std::source_location loc)
    {
        return FileIOException::create(std::filesystem::path(TEST_RESOURCE_DIR), message, loc);
    }
};

template<>
struct ExceptionTraits<SQLiteAccessException>
{
    static SQLiteAccessException make(std::string_view message, std::source_location loc)
    {
        return SQLiteAccessException::create(message, loc);
    }
};

template<>
struct ExceptionTraits<SQLiteConnectionException>
{
    static SQLiteConnectionException make(std::string_view message, std::source_location loc)
    {
        return SQLiteConnectionException::create(message, std::filesystem::path(), loc);
    }
};

template<>
struct ExceptionTraits<ConfigValidationException>
{
    static ConfigValidationException make(std::string_view message, std::source_location loc)
    {
        auto errors{ Validator::ValidationErrors{ {
            .path = nlohmann::json::json_pointer{},
            .message = std::string(message),
        } } };

        return ConfigValidationException::create(std::move(errors), loc);
    }
};

} // namespace ful::testing

#endif // !FUL_SRC_TEST_TEST_UTILITY_EXCEPTION_TEST_TRAITS_HPP
