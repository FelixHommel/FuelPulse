#ifndef FUL_SRC_UTILITY_HTTP_ERROR_CODES_HPP
#define FUL_SRC_UTILITY_HTTP_ERROR_CODES_HPP

#include <cstdint>
#include <string>
#include <utility>

namespace ful::http
{

/// \brief Possible return values for an HTTP request
///
/// \author Felix Hommel
/// \date 8/23/2026
enum class HttpCode : std::uint16_t
{
    BadRequest = 400l,
    Unauthorized = 401l,
    NotFound = 404l,
    InternalServerError = 500l,
    ServiceUnavailable = 503l
};

/// \brief Convert a \ref HttpCode to its string representation.
[[nodiscard]] constexpr std::string httpCodeToString(HttpCode code)
{
    switch(code)
    {
        using enum HttpCode;
    case BadRequest:
        return "Bad Request";
    case Unauthorized:
        return "Unauthorized";
    case NotFound:
        return "Not Found";
    case InternalServerError:
        return "Internal Server Error";
    case ServiceUnavailable:
        return "Service Unavailable";
    }

    std::unreachable();
}

} // namespace ful::http

#endif // !FUL_SRC_UTILITY_HTTP_ERROR_CODES_HPP
