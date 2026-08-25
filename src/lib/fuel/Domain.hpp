#ifndef FUL_SRC_LIB_FUEL_DOMAIN_HPP
#define FUL_SRC_LIB_FUEL_DOMAIN_HPP

#include "fuel/FuelType.hpp"
#include "utility/hash/HashCombine.hpp"

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <utility>

namespace
{

template<typename T>
struct PerFuel
{
    T e5;
    T e10;
    T diesel;

    T& operator[](ful::fuel::FuelType t)
    {
        switch(t)
        {
            using enum ful::fuel::FuelType;
        case E5:
            return e5;
        case E10:
            return e10;
        case Diesel:
            return diesel;
        };

        std::unreachable();
    }
    const T& operator[](ful::fuel::FuelType t) const { return operator[](t); }
};

} // namespace

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

/// \brief Simple statistics about a single \ref Station.
///
/// \author Felix Hommel
/// \date 8/24/2026
struct BasicStats
{
    PriceCents min{};
    PriceCents max{};
    PriceCents mean{};
    PriceCents median{};
    PriceCents opening{};
    PriceCents closing{};
    std::size_t sampleCount{};
    std::size_t priceChagnedCount{};
};

/// \brief Advanced statistics about a single \ref Station.
///
/// \author Felix Hommel
/// \date 8/24/2026
struct AdvancedStats
{
    double stdDeviation{};
    PriceCents timeWeightAverage{};
    double changesPerHour{};
    double trendSlopePerHour{};
    int cheapestHourOfDay{};
    std::optional<double> percentileRankAmongStations;
    std::optional<std::int64_t> deltaFromPreviousDay;
};

/// \brief The cheapest window of the day to purchase fuel from a \ref Station.
///
/// \author Felix Hommel
/// \date 8/24/2026
struct CheapestWindow
{
    std::chrono::system_clock::time_point start;
    std::chrono::system_clock::time_point end;
    PriceCents price{};
};

/// \brief Complete analysis of a single \ref Station.
///
/// \author Felix Hommel
/// \date 8/24/2026
struct StationAnalysis
{
    std::string stationId;
    ::PerFuel<BasicStats> basicStats;
    ::PerFuel<AdvancedStats> advancedStats;
    ::PerFuel<CheapestWindow> cheapestWindow;
};

} // namespace ful::fuel

// NOLINTBEGIN(cert-dcl58-cpp): modifying std namespace for hash overload is accepted
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
// NOLINTEND(cert-dcl58-cpp)

#endif // !FUL_SRC_LIB_FUEL_DOMAIN_HPP
