#ifndef FUL_SRC_LIB_FUEL_DOMAIN_HPP
#define FUL_SRC_LIB_FUEL_DOMAIN_HPP

#include <chrono>
#include <cstdint>
#include <optional>
#include <string>

namespace ful::fuel
{

/// \brief Price in whole cents (avoid floating point math)
using PriceCents = std::uint32_t;

/// \brief Gas station details as reported by the API.
///
/// \author Felix Hommel
/// \date 7/21/2026
struct Station
{
    std::string id;
    std::string name;
    std::string brand;
    double longitude{};
    double latitude{};
};

/// \brief A single point-in-time price reading for one station.
///
/// \author Felix Hommel
/// \date 7/21/2026
struct Measurement
{
    std::string stationId;
    std::chrono::system_clock::time_point timestamp;
    std::optional<PriceCents> e5;
    std::optional<PriceCents> e10;
    std::optional<PriceCents> diesel;
};

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_DOMAIN_HPP
