#include "SQLiteAccessException.hpp"

#include "utility/exception/Exception.hpp"

#include <format>
#include <source_location>
#include <string_view>
#include <utility>

namespace ful
{

SQLiteAccessException::SQLiteAccessException(Exception exception) : Exception{ std::move(exception) } {}

SQLiteAccessException SQLiteAccessException::create(std::string_view reason, std::source_location loc)
{
    return SQLiteAccessException{
        Exception{ std::format("SQLite access error: {}", reason), loc }
    };
}

} // namespace ful
