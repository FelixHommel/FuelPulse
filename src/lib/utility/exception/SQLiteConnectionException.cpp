#include "SQLiteConnectionException.hpp"

#include "utility/exception/Exception.hpp"

#include <filesystem>
#include <format>
#include <optional>
#include <source_location>
#include <string_view>
#include <utility>

namespace ful
{

SQLiteConnectionException::SQLiteConnectionException(Exception exception)
    : Exception{ std::move(exception) }, m_dbPath{ std::nullopt }
{}
SQLiteConnectionException::SQLiteConnectionException(Exception exception, std::optional<std::filesystem::path> dbPath)
    : Exception{ std::move(exception) }, m_dbPath{ std::move(dbPath) }
{}

SQLiteConnectionException SQLiteConnectionException::create(
    std::string_view reason, std::filesystem::path dbPath, std::source_location loc
)
{
    if(dbPath.empty())
    {
        return SQLiteConnectionException{
            Exception{ std::format("SQLite Connection error: {}", reason), loc }
        };
    }

    auto message{ std::format("[{}] SQLite Connection error: {}", dbPath.string(), reason) };
    return SQLiteConnectionException{
        Exception{ std::move(message), loc },
        std::make_optional(std::move(dbPath))
    };
}

} // namespace ful
