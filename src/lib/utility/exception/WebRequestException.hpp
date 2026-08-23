#ifndef FUL_SRC_LIB_UTILITY_EXCEPTION_WEB_REQUEST_EXCEPTION_HPP
#define FUL_SRC_LIB_UTILITY_EXCEPTION_WEB_REQUEST_EXCEPTION_HPP

#include "utility/HttpErrorCodes.hpp"
#include "utility/exception/Exception.hpp"

#include <source_location>

namespace ful
{

/// \brief Exception for when a web request failed.
///
/// \author Felix Hommel
/// \date 8/23/2026
class WebRequestException : public Exception
{
public:
    ~WebRequestException() override = default;

    WebRequestException(const WebRequestException&) = default;
    WebRequestException& operator=(const WebRequestException&) = default;
    WebRequestException(WebRequestException&&) = default;
    WebRequestException& operator=(WebRequestException&&) = default;

    /// \brief Create a new \ref WebRequestException.
    ///
    /// \param errorCode The HTTP error code that caused the exception
    /// \param loc (optional) The \ref std::source_location where the exception was caused
    ///
    /// \returns The new \ref WebRequestException
    [[nodiscard]] static WebRequestException create(
        http::HttpCode errorCode, std::source_location loc = std::source_location::current()
    );

    [[nodiscard]] http::HttpCode errorCode() const noexcept { return m_error; }

private:
    WebRequestException(Exception exception, http::HttpCode errorCode);

    http::HttpCode m_error;
};

} // namespace ful

#endif // !FUL_SRC_LIB_UTILITY_EXCEPTION_WEB_REQUEST_EXCEPTION_HPP
