#ifndef FUL_SRC_LIB_FUEL_DOMAIN_HPP
#define FUL_SRC_LIB_FUEL_DOMAIN_HPP

#include "utility/hash/HashCombine.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
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

    constexpr auto operator<=>(const Station&) const = default;
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

    constexpr auto operator<=>(const Measurement&) const = default;
};

} // namespace ful::fuel

namespace std
{

template<>
struct hash<std::chrono::system_clock::time_point>
{
    std::size_t operator()(const std::chrono::system_clock::time_point& tp) const noexcept
    {
        return std::hash<std::chrono::system_clock::rep>{}(tp.time_since_epoch().count());
    }
};

template<>
struct hash<ful::fuel::Station>
{
    std::size_t operator()(const ful::fuel::Station& station) const noexcept
    {
        std::size_t seed{ 0 };

        ful::hash::hashCombine(seed, station.id);
        ful::hash::hashCombine(seed, station.name);
        ful::hash::hashCombine(seed, station.brand);
        ful::hash::hashCombine(seed, station.longitude);
        ful::hash::hashCombine(seed, station.latitude);

        return seed;
    }
};

template<>
struct hash<ful::fuel::Measurement>
{
    constexpr std::size_t operator()(const ful::fuel::Measurement& measurement) const noexcept
    {
        std::size_t seed{ 0 };

        ful::hash::hashCombine(seed, measurement.stationId);
        ful::hash::hashCombine(seed, measurement.timestamp);
        ful::hash::hashCombine(seed, measurement.e5);
        ful::hash::hashCombine(seed, measurement.e10);
        ful::hash::hashCombine(seed, measurement.diesel);

        return seed;
    }
};

} // namespace std

#endif // !FUL_SRC_LIB_FUEL_DOMAIN_HPP
