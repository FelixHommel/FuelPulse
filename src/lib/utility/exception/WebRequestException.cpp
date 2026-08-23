#include "WebRequestException.hpp"

#include "utility/HttpErrorCodes.hpp"
#include "utility/exception/Exception.hpp"

#include <format>
#include <source_location>
#include <type_traits>
#include <utility>

namespace ful
{

WebRequestException WebRequestException::create(http::HttpCode errorCode, std::source_location loc)
{
    auto message{ std::format(
        "[{}: {}] unable to complete the web request",
        static_cast<std::underlying_type_t<http::HttpCode>>(errorCode),
        http::httpCodeToString(errorCode)
    ) };

    return WebRequestException{
        Exception{ std::move(message), loc },
        errorCode
    };
}

WebRequestException::WebRequestException(Exception exception, http::HttpCode errorCode)
    : Exception{ std::move(exception) }, m_error{ errorCode }
{}

} // namespace ful
