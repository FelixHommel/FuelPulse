#ifndef FUL_SRC_LIB_FUEL_CPR_TANKER_KOENIG_GATEWAY_HPP
#define FUL_SRC_LIB_FUEL_CPR_TANKER_KOENIG_GATEWAY_HPP

#include <cpr/api.h>
#include <cpr/cprtypes.h>
#include <cpr/parameters.h>

#include <stdexcept>
#include <string>
#include <utility>

namespace ful::fuel
{

namespace
{

constexpr auto HTTP_OK{ 200l };
constexpr auto HTTP_BAD_REQUEST{ 400l };
constexpr auto HTTP_UNAUTHORIZED{ 401l };
constexpr auto HTTP_NOT_FOUND{ 404l };
constexpr auto HTTP_INTERNAL_SERVER_ERROR{ 500l };
constexpr auto HTTP_SERVICE_UNAVAILABLE{ 503l };

} // namespace

/// \brief Gateway to the TankerKoenig API using curl via the cpr library.
///
/// \author Felix Hommel
/// \date 8/14/2026
class CprTankerKoenigGateway
{
public:
    // NOLINTBEGIN(readability-convert-member-functions-to-static): Not supposed to be static by design

    /// \brief Make a web-request to the TankerKoenig API using curl.
    ///
    /// \param apiKey The API key which to provide the API
    /// \param postalCode The postal code around which is scanned
    ///
    /// \returns A \ref std::string containing the raw response of the API
    [[nodiscard]] std::string fetchPrices(const std::string& apiKey, unsigned int postalCode) const
    {
        const auto curlResponse{
            cpr::Get(
                cpr::Url{ REQUEST_URL },
                cpr::Parameters{ { "apikey", apiKey }, { "postalcode", std::to_string(postalCode) } }
            )
        };

        if(curlResponse.status_code != HTTP_OK)
            throw std::runtime_error(""); // TODO: Implement custom exception

        return std::move(curlResponse.text);
    }

    // NOLINTEND(readability-convert-member-functions-to-static)

private:
    static constexpr auto REQUEST_URL{ "https://creativecommons.tankerkoenig.de/api/v4/stations/postalcode" };
};

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_CPR_TANKER_KOENIG_GATEWAY_HPP
