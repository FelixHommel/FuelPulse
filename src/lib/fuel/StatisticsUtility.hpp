#ifndef FUL_SRC_LIB_FUEL_STATISTICS_UTILITY_HPP
#define FUL_SRC_LIB_FUEL_STATISTICS_UTILITY_HPP

#include "fuel/Domain.hpp"
#include "fuel/FuelType.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>

namespace ful::fuel::stats
{

inline constexpr std::array<FuelType, 3> ALL_FUEL_TYPES{ FuelType::E5, FuelType::E10, FuelType::Diesel };

/// \brief Keep track of often used properties of a measurement sample without needing to reiterate over the entire set
///     all the time.
///
/// \author Felix Hommel
/// \date 8/26/2026
struct RunningStats
{
    std::uint64_t count{ 0 };
    std::uint64_t sum{ 0 };
    double sumSq{ 0 };
    PriceCents min{ std::numeric_limits<PriceCents>::max() };
    PriceCents max{ 0 };
};

/// \brief Utility function to get the price of a certain fuel within the measurement.
///
/// \param measurement The sample \ref Measurement from a set of all measurements
/// \param type The \ref FuelType for which the price is requested
///
/// \returns the \ref std::optional that contains the price of \p type
[[nodiscard]] constexpr std::optional<PriceCents> priceFor(const Measurement& measurement, FuelType type);

/// \brief Iterate over an entire set of \ref Measurement to accumulate all common stats that can be obtained while
///     iterating over the set.
///
/// \param measurement The set of \ref Measurement
/// \param type The \ref FuelType for which the stats are accumulated
///
/// \returns \ref RunningStats for \p type
[[nodiscard]] RunningStats accumulateRunningStats(std::span<const Measurement> measurements, FuelType type);
/// \brief Calculate the median price for all prices of a fuel type within a set of measurements.
///
/// \param measurement The set of \ref Measurement
/// \param type The \ref FuelType for which the median is calculated
///
/// \returns The median Price of \p type in \p measurements
[[nodiscard]] PriceCents calculateMedian(std::span<const Measurement> measurements, FuelType type);
/// \brief Count how often the price of a certain type of fuel changed within a set of measurements.
///
/// \param measurement The set of \ref Measurement
/// \param type The \ref FuelType for which the price changes are counted
///
/// \return The amount of times the price of \p type changed within \p measurements
[[nodiscard]] std::size_t countPriceChanges(std::span<const Measurement> measurements, FuelType type);
/// \brief Calculate the standard deviation of the price of a fuel type.
///
/// \param stats The \ref RunningStats of a fuel type that was precalculated
///
/// \returns the standard deviation of the fuel price which is tracked in \p stats
[[nodiscard]] double calculateStdDeviation(const RunningStats& stats);
/// \brief Calculate the Time-Weight Average price for a certain fuel type.
///
/// \param measurement The set of \ref Measurement
/// \param type The \ref FuelType for which the Time-Weight Average is calculated
///
/// \returns The Time-Weight Average price for \p type within \p measurements
[[nodiscard]] PriceCents calculateTimeWeightAverage(std::span<const Measurement> measurements, FuelType type);
/// \brief Calculate the price changes per hour for a certain type of fuel.
///
/// \param measurement The set of \ref Measurement
/// \param priceChangeCount The amount of times the price changed in total
///
/// \returns The amount of price changes averaged per hour
[[nodiscard]] double calculatePriceChangesPerHour(
    std::span<const Measurement> measurements, std::size_t priceChangeCount
);
/// \brief Calculate the incline of the of the fuel price slope.
///
/// \param measurement The set of \ref Measurement
/// \param type The \ref FuelType for which the incline is calculated
///
/// returns The incline of the slope that represents the price of \p type within \p measurements
[[nodiscard]] double calculateTrendSlopePerHour(std::span<const Measurement> measurements, FuelType type);
/// \brief Calculate the cheapest hour of the day for a certain type of fuel.
///
/// \param measurement The set of \ref Measurement
/// \param type The \ref FuelType for which the cheapest hour is determined
///
/// \returns The hour in which \p is the cheapest within \p measurements
[[nodiscard]] int calculateCheapestHourOfDay(std::span<const Measurement> measurements, FuelType type);
/// \brief Find the cheapest window for a certain type of fuel.
///
/// \param measurement The set of \ref Measurement
/// \param type The \ref FuelType for which the cheapest hour is determined
///
/// \returns \ref CheapestWindow for \p type within \p measurements
[[nodiscard]] CheapestWindow analyzeCheapestWindow(std::span<const Measurement> measurements, FuelType type);

/// \brief Analyze a set of \ref Measurement for basic statistical measures.
///
/// \param measurement The set of \ref Measurement
/// \param type The \ref FuelType that is analyzed
/// \param runningStats The \ref RunningStats that can be reused for \p type
/// \param priceChangeCount The amount of times \p type has changed prices within \p measurements
///
/// \returns \ref BasicStats about \p measurements for \p type
[[nodiscard]] BasicStats analyzeBasicStats(
    std::span<const Measurement> measurements,
    FuelType type,
    const stats::RunningStats& runningStats,
    std::size_t priceChangeCount
);
/// \brief Analyze a set of \ref Measurement for advanced statistical measures.
///
/// \param measurement The set of \ref Measurement
/// \param type The \ref FuelType that is analyzed
/// \param runningStats The \ref RunningStats that can be reused for \p type
/// \param priceChangeCount The amount of times \p type has changed prices within \p measurements
///
/// \returns \ref AdvancedStats about \p measurements for \p type
[[nodiscard]] AdvancedStats analyzeAdvancedStats(
    std::span<const Measurement> measurements,
    FuelType type,
    const stats::RunningStats& runningStats,
    std::size_t priceChangeCount
);

/// \brief Mean price per fuel for every distinct station present in \p data.
///
/// \param data The measurements to compute means from, may contain multiple stations in any order
///
/// \returns A map from station id to its \ref PerFuel of mean prices, 0 for any fuel a station never reported
[[nodiscard]] std::unordered_map<std::string, PerFuel<PriceCents>> computeMeansPerStation(
    std::span<const Measurement> data
);

/// \brief Rank each station's mean price against every other station's, per fuel, storing the result in
///     \ref StationAnalysis::advancedStats.
///
/// \param results The per-station results to rank, modified in place. Left untouched if it has fewer than two entries
void assignPercentileRanks(std::vector<StationAnalysis>& results);

} // namespace ful::fuel::stats

#endif // !FUL_SRC_LIB_FUEL_STATISTICS_UTILITY_HPP
