#ifndef FUL_SRC_LIB_UTILITY_EXCEPTION_EXCEPTION_HPP
#define FUL_SRC_LIB_UTILITY_EXCEPTION_EXCEPTION_HPP

#include "utility/exception/Stacktrace.hpp"

#include <exception>
#include <source_location>
#include <string>

namespace ful
{

/// \brief A General-purpose exception type that represents the most generic type of exception.
///
/// \author Felix Hommel
/// \date 8/8/2026
class Exception : public std::exception
{
public:
    /// \brief Construct a new \ref Exception.
    ///
    /// \param message The exception message
    /// \param loc (optional) The \ref std::source_location where the exception was caused
    explicit Exception(std::string message, std::source_location loc = std::source_location::current());

    ~Exception() override = default;

    Exception(const Exception&) = default;
    Exception& operator=(const Exception&) = default;
    Exception(Exception&&) = default;
    Exception& operator=(Exception&&) = default;

    /// \brief Get the whole exception message.
    [[nodiscard]] const char* what() const noexcept override { return m_formatted.c_str(); };
    /// \brief Get the user-provieded exception message.
    [[nodiscard]] const std::string& message() const noexcept { return m_message; }
    /// \brief Get the location where the exception was caused.
    [[nodiscard]] const std::source_location& where() const noexcept { return m_location; }

protected:
    std::string m_message;
    std::source_location m_location;
    detail::Stacktrace m_trace;
    std::string m_formatted;
};

} // namespace ful

#endif // !FUL_SRC_LIB_UTILITY_EXCEPTION_EXCEPTION_HPP
