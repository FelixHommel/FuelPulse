#ifndef FUL_SRC_LIB_UTILITY_EXCEPTION_SQLITE_CONNECTION_EXCEPTION_HPP
#define FUL_SRC_LIB_UTILITY_EXCEPTION_SQLITE_CONNECTION_EXCEPTION_HPP

#include "utility/exception/Exception.hpp"

#include <filesystem>
#include <optional>
#include <source_location>
#include <string_view>

namespace ful
{

/// \brief Exception for errors that occur during operations involving a \ref SQLiteConnection.
///
/// \author Felix Hommel
/// \date 8/8/2026
class SQLiteConnectionException final : public Exception
{
public:
    ~SQLiteConnectionException() override = default;

    SQLiteConnectionException(const SQLiteConnectionException&) = default;
    SQLiteConnectionException& operator=(const SQLiteConnectionException&) = default;
    SQLiteConnectionException(SQLiteConnectionException&&) = default;
    SQLiteConnectionException& operator=(SQLiteConnectionException&&) = default;

    /// \brief Create a new \ref SQLiteConnectionException.
    ///
    /// \param reason Provided reason for the exception
    /// \param dbPath (optional) A \ref std::filesystem::path to the database on the disk
    /// \param loc (optional) The \ref std::source_location where the exception was caused
    ///
    /// \returns The new \ref SQLiteConnectionException
    [[nodiscard]] static SQLiteConnectionException create(
        std::string_view reason,
        std::filesystem::path dbPath = std::filesystem::path(),
        std::source_location loc = std::source_location::current()
    );

    [[nodiscard]] const std::optional<std::filesystem::path>& dbPath() const noexcept { return m_dbPath; }

private:
    std::optional<std::filesystem::path> m_dbPath;

    SQLiteConnectionException(Exception exception);
    SQLiteConnectionException(Exception exception, std::optional<std::filesystem::path> dbPath);
};

} // namespace ful

#endif // !FUL_SRC_LIB_UTILITY_EXCEPTION_SQLITE_CONNECTION_EXCEPTION_HPP
