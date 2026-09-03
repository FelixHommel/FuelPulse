#include "StatisticsUtility.hpp"

#include "fuel/Domain.hpp"
#include "fuel/FuelType.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <execution>
#include <iterator>
#include <limits>
#include <numeric>
#include <optional>
#include <ranges>
#include <ratio>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ful::fuel::stats
{

[[nodiscard]] constexpr std::optional<PriceCents> priceFor(const Measurement& measurement, FuelType type)
{
    switch(type)
    {
        using enum FuelType;
    case E5:
        return measurement.e5;
    case E10:
        return measurement.e10;
    case Diesel:
        return measurement.diesel;
    }

    std::unreachable();
}

[[nodiscard]] RunningStats accumulateRunningStats(std::span<const Measurement> measurements, FuelType type)
{
    return std::transform_reduce(
        std::execution::par,
        measurements.begin(),
        measurements.end(),
        RunningStats{},
        [](RunningStats lhs, const RunningStats& rhs) {
            lhs.count += rhs.count;
            lhs.sum += rhs.sum;
            lhs.sumSq += rhs.sumSq;
            lhs.min = std::min(lhs.min, rhs.min);
            lhs.max = std::max(lhs.max, rhs.max);

            return lhs;
        },
        [&type](const Measurement& m) -> RunningStats {
            if(const auto price{ priceFor(m, type) }; price.has_value())
            {
                return {
                    .count = 1,
                    .sum = *price,
                    .sumSq = static_cast<double>(*price) * static_cast<double>(*price),
                    .min = *price,
                    .max = *price,
                };
            }

            return {};
        }
    );
}

[[nodiscard]] PriceCents calculateMedian(std::span<const Measurement> measurements, FuelType type)
{
    std::vector<PriceCents> prices;
    prices.reserve(measurements.size());

    for(const auto& m : measurements)
    {
        if(const auto price{ priceFor(m, type) }; price.has_value())
            prices.push_back(*price);
    }

    if(prices.empty())
        return 0;

    const auto mid{ prices.size() / 2 };
    std::ranges::nth_element(prices, prices.begin() + static_cast<std::ptrdiff_t>(mid));

    if(prices.size() % 2 != 0)
        return prices[mid];

    // NOTE: std::range::nth_elment guarantees that the n-th element is in the location where it would be if the entire
    //  container is sorted and that everything before the n-th element is less than or equal to the n-th element.
    //  Therefore, the lower bound is the maximum number of the numbers before the n-th element.
    const auto lower{ std::ranges::max_element(prices.begin(), prices.begin() + static_cast<std::ptrdiff_t>(mid)) };
    return static_cast<PriceCents>((*lower + prices[mid]) / 2);
}

[[nodiscard]] std::size_t countPriceChanges(std::span<const Measurement> measurements, FuelType type)
{
    if(measurements.size() < 2)
        return 0;

    std::size_t count{ 0 };
    for(std::size_t idx{ 1 }; idx < measurements.size(); ++idx)
    {
        if(priceFor(measurements[idx], type) != priceFor(measurements[idx - 1], type))
            ++count;
    }

    return count;
}

[[nodiscard]] double calculateStdDeviation(const RunningStats& stats)
{
    if(stats.count == 0)
        return 0.0;

    const auto meanD{ static_cast<double>(stats.sum) / static_cast<double>(stats.count) };
    const auto variance{ (stats.sumSq / static_cast<double>(stats.count)) - (meanD * meanD) };

    return std::sqrt(std::max(variance, 0.0));
}

[[nodiscard]] PriceCents calculateTimeWeightAverage(std::span<const Measurement> measurements, FuelType type)
{
    if(measurements.size() < 2)
        return 0;

    double weightedSum{ 0.0 };
    double totalWeight{ 0.0 };
    for(std::size_t idx{ 1 }; idx < measurements.size(); ++idx)
    {
        const auto price{ priceFor(measurements[idx], type) };

        if(!price.has_value())
            continue;

        const auto delta{ std::chrono::duration_cast<std::chrono::duration<double>>(
            measurements[idx].timestamp - measurements[idx - 1].timestamp
        ) };

        weightedSum += static_cast<double>(*price) * delta.count();
        totalWeight += delta.count();
    }

    if(totalWeight <= 0.0)
        return 0;

    return static_cast<PriceCents>(std::llround(weightedSum / totalWeight));
}

[[nodiscard]] double calculatePriceChangesPerHour(
    std::span<const Measurement> measurements, std::size_t priceChangeCount
)
{
    if(measurements.size() < 2)
        return 0.0;

    // clang-format off
    const auto hoursSpan{ std::chrono::duration_cast<std::chrono::duration<double, std::ratio<3600>>>(
          measurements.back().timestamp - measurements.front().timestamp
    ) .count() };
    // clang-format on

    if(hoursSpan <= 0.0)
        return 0.0;

    return static_cast<double>(priceChangeCount) / hoursSpan;
}

[[nodiscard]] double calculateTrendSlopePerHour(std::span<const Measurement> measurements, FuelType type)
{
    if(measurements.size() < 2)
        return 0.0;

    const auto start{ measurements.front().timestamp };

    double sumX{ 0.0 };
    double sumY{ 0.0 };
    double sumXY{ 0.0 };
    double sumXX{ 0.0 };
    std::size_t n{ 0 };

    for(const auto& m : measurements)
    {
        const auto price{ priceFor(m, type) };

        if(!price.has_value())
            continue;

        const auto hours{
            std::chrono::duration_cast<std::chrono::duration<double, std::ratio<3600>>>(m.timestamp - start).count()
        };
        const auto y{ static_cast<double>(*price) };

        sumX += hours;
        sumY += y;
        sumXY += hours * y;
        sumXX += hours * hours;

        ++n;
    }

    if(n < 2)
        return 0.0;

    const auto denominator{ (static_cast<double>(n) * sumXX) - (sumX * sumX) };
    if(denominator == 0.0)
        return 0.0;

    return ((static_cast<double>(n) * sumXY) - (sumX * sumY)) / denominator;
}

[[nodiscard]] int calculateCheapestHourOfDay(std::span<const Measurement> measurements, FuelType type)
{
    std::array<RunningStats, 24> perHour{}; // NOLINT: 24 hours per day

    for(const auto& m : measurements)
    {
        const auto price{ priceFor(m, type) };

        if(!price.has_value())
            continue;

        // NOLINTNEXTLINE(misc-include-cleaner): <chrono> is included
        const auto dayPoint{ std::chrono::floor<std::chrono::days>(m.timestamp) };
        const std::chrono::hh_mm_ss time{ m.timestamp - dayPoint };
        auto& bucket{ perHour.at(static_cast<std::size_t>(time.hours().count())) };

        ++bucket.count;
        bucket.sum += *price;
    }

    auto* const cheapest{ std::ranges::min_element(perHour, {}, [](const RunningStats& stats) {
        return stats.count > 0 ? static_cast<double>(stats.sum) / static_cast<double>(stats.count)
                               : std::numeric_limits<double>::max();
    }) };

    return static_cast<int>(std::distance(perHour.begin(), cheapest));
}

[[nodiscard]] CheapestWindow analyzeCheapestWindow(std::span<const Measurement> measurements, FuelType type)
{
    CheapestWindow best{ .price = std::numeric_limits<PriceCents>::max() };

    std::optional<PriceCents> runPrice;
    std::chrono::system_clock::time_point runStart{};

    const auto closeRun = [&](std::chrono::system_clock::time_point runEnd) {
        if(runPrice.has_value() && *runPrice < best.price)
            best = { .start = runStart, .end = runEnd, .price = *runPrice };
    };

    for(const auto& m : measurements)
    {
        const auto price{ stats::priceFor(m, type) };

        if(!price.has_value())
            continue;

        if(!runPrice.has_value() || *price != *runPrice)
        {
            closeRun(m.timestamp);

            runPrice = price;
            runStart = m.timestamp;
        }
    }

    if(!measurements.empty())
        closeRun(measurements.back().timestamp);

    return best;
}

[[nodiscard]] BasicStats analyzeBasicStats(
    std::span<const Measurement> measurements,
    FuelType type,
    const stats::RunningStats& runningStats,
    std::size_t priceChangeCount
)
{
    BasicStats stats{
        .min = runningStats.count > 0 ? runningStats.min : 0,
        .max = runningStats.count > 0 ? runningStats.max : 0,
        .mean = runningStats.count > 0 ? static_cast<PriceCents>(runningStats.sum / runningStats.count) : 0,
        .median = stats::calculateMedian(measurements, type),
        .sampleCount = measurements.size(),
        .priceChagnedCount = priceChangeCount,
    };

    for(const auto& m : measurements)
    {
        if(const auto price{ stats::priceFor(m, type) }; price.has_value())
        {
            stats.opening = *price;
            break;
        }
    }

    for(const auto& m : std::views::reverse(measurements))
    {
        if(const auto price{ stats::priceFor(m, type) }; price.has_value())
        {
            stats.closing = *price;
            break;
        }
    }

    return stats;
}

[[nodiscard]] AdvancedStats analyzeAdvancedStats(
    std::span<const Measurement> measurements,
    FuelType type,
    const stats::RunningStats& runningStats,
    std::size_t priceChangeCount
)
{
    return AdvancedStats{
        .stdDeviation = calculateStdDeviation(runningStats),
        .timeWeightAverage = stats::calculateTimeWeightAverage(measurements, type),
        .changesPerHour = stats::calculatePriceChangesPerHour(measurements, priceChangeCount),
        .trendSlopePerHour = stats::calculateTrendSlopePerHour(measurements, type),
        .cheapestHourOfDay = stats::calculateCheapestHourOfDay(measurements, type),
        .percentileRankAmongStations = std::nullopt,
        .deltaFromPreviousDay = std::nullopt,
    };
}

[[nodiscard]] std::unordered_map<std::string, PerFuel<PriceCents>> computeMeansPerStation(
    std::span<const Measurement> data
)
{
    std::unordered_map<std::string, std::vector<Measurement>> grouped;
    for(const auto& m : data)
        grouped[m.stationId].push_back(m);

    std::unordered_map<std::string, PerFuel<PriceCents>> result;
    for(const auto& [stationId, measurements] : grouped)
    {
        PerFuel<PriceCents> means{};
        for(auto fuel : ALL_FUEL_TYPES)
        {
            const auto stats{ stats::accumulateRunningStats(measurements, fuel) };

            means[fuel] = stats.count > 0
                            ? static_cast<PriceCents>(
                                  std::llround(static_cast<double>(stats.sum) / static_cast<double>(stats.count))
                              )
                            : 0;
        }

        result.emplace(stationId, means);
    }

    return result;
}

void assignPercentileRanks(std::vector<StationAnalysis>& results)
{
    constexpr auto TO_PERCENT{ 100.0 };

    if(results.size() < 2)
        return;

    for(auto fuel : ALL_FUEL_TYPES)
    {
        std::vector<PriceCents> means;
        means.reserve(results.size());
        for(const auto& r : results)
            means.push_back(r.basicStats[fuel].mean);

        for(auto& r : results)
        {
            const auto mean{ r.basicStats[fuel].mean };
            const auto rank{ std::ranges::count_if(means, [mean](PriceCents price) { return price <= mean; }) };

            r.advancedStats[fuel].percentileRankAmongStations = std::make_optional(
                static_cast<double>(rank - 1) / static_cast<double>(means.size() - 1) * TO_PERCENT
            );
        }
    }
}

} // namespace ful::fuel::stats
