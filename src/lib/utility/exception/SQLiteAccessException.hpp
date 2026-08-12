#ifndef FUL_SRC_LIB_UTILITY_EXCEPTION_SQLITE_ACCESS_EXCEPTION_HPP
#define FUL_SRC_LIB_UTILITY_EXCEPTION_SQLITE_ACCESS_EXCEPTION_HPP

#include "utility/exception/Exception.hpp"

#include <source_location>
#include <string_view>

namespace ful
{

/// \brief Exception for errors that occur during operations involving a \ref SQLiteConnection and wrong access modes.
///
/// \author Felix Hommel
/// \date 8/12/2026
class SQLiteAccessException final : public Exception
{
public:
    ~SQLiteAccessException() override = default;

    SQLiteAccessException(const SQLiteAccessException&) = default;
    SQLiteAccessException& operator=(const SQLiteAccessException&) = default;
    SQLiteAccessException(SQLiteAccessException&&) = default;
    SQLiteAccessException& operator=(SQLiteAccessException&&) = default;

    /// \brief Create a new \ref SQLiteAccessException.
    ///
    /// \param reason Provided reason for the exception
    /// \param loc (optional) The \ref std::source_location where the exception was caused
    ///
    /// \returns The new \ref SQLiteAccessException
    [[nodiscard]] static SQLiteAccessException create(
        std::string_view reason, std::source_location loc = std::source_location::current()
    );

private:
    explicit SQLiteAccessException(Exception exception);
};

} // namespace ful

#endif // !FUL_SRC_LIB_UTILITY_EXCEPTION_SQLITE_ACCESS_EXCEPTION_HPP
