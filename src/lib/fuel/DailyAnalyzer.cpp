#include "DailyAnalyzer.hpp"
#include "fuel/Domain.hpp"
#include "fuel/FuelType.hpp"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ful::fuel
{

namespace
{

constexpr std::array<FuelType, 3> ALL_FUEL_TYPES{ FuelType::E5, FuelType::E10, FuelType::Diesel };

[[nodiscard]] constexpr std::optional<PriceCents> priceFor(const Measurement& m, FuelType fuel)
{
    switch(fuel)
    {
        using enum FuelType;
    case E5:
        return m.e5;
    case E10:
        return m.e10;
    case Diesel:
        return m.diesel;
    }

    std::unreachable();
}

[[nodiscard]] PriceCents findMin(std::span<const Measurement> m, FuelType t)
{
    const auto result{ priceFor(
        *std::ranges::min_element(
            m, [&t](const Measurement& lhs, const Measurement& rhs) { return priceFor(lhs, t) < priceFor(rhs, t); }
        ),
        t
    ) };

    return result.value_or(0);
}

[[nodiscard]] PriceCents findMax(std::span<const Measurement> m, FuelType t)
{
    const auto result{ priceFor(
        *std::ranges::max_element(
            m, [&t](const Measurement& lhs, const Measurement& rhs) { return priceFor(lhs, t) < priceFor(rhs, t); }
        ),
        t
    ) };

    return result.value_or(0);
}

[[nodiscard]] PriceCents calculateMean(std::span<const Measurement> m, FuelType t)
{
    if(m.empty())
        return 0;

    const auto result{ std::accumulate<std::span<const Measurement>::const_iterator, std::uint32_t>(
        m.begin(), m.end(), 0, [&t](std::uint32_t acc, const Measurement& elem) {
            return acc + priceFor(elem, t).value_or(0);
        }
    ) };

    return result > 0 ? result / m.size() : 0;
}

[[nodiscard]] PriceCents calculateMedian(std::span<const Measurement> m, FuelType t)
{
    const auto valFloor{
        priceFor(m[static_cast<std::size_t>(std::floor(static_cast<float>(m.size()) / 2.f))], t).value_or(0)
    };

    if(m.size() % 2 == 0)
        return valFloor;

    return valFloor
         + priceFor(m[static_cast<std::size_t>(std::ceil(static_cast<float>(m.size()) / 2.f))], t).value_or(0);
}

[[nodiscard]] std::size_t countPriceChanges(std::span<const Measurement> m, FuelType t)
{
    if(m.empty())
        return 0;

    if(m.size() == 1)
        return 1;

    std::size_t count{ 0 };

    for(std::size_t idx{ 1 }; idx < m.size(); ++idx)
    {
        if(const auto currentPrice{ priceFor(m[idx], t) }, previousPrice{ priceFor(m[idx - 1], t) };
           currentPrice != previousPrice)
        {
            ++count;
        }
    }

    return count;
}

[[nodiscard]] double calculateStdDeviation(std::span<const Measurement> measurements, FuelType type, PriceCents mean)
{
    const auto variance{ std::accumulate(
        measurements.begin(), measurements.end(), 0, [&type, &mean](int total, const Measurement& value) {
            const auto devAtValue{ priceFor(value, type).value_or(mean) - mean };

            return total + (devAtValue * devAtValue);
        }
    ) };

    return std::sqrt(variance);
}

[[nodiscard]] PriceCents calculateTimeWeightAverage(std::span<const Measurement> measurements, FuelType type)
{
    if(measurements.empty() || measurements.size() == 1)
        return 0;

    // NOTE: Currently skips the first element because it's impossible to calculate a time delta for the first measurement
    std::uint32_t sumOfPriceDelta{ 0 };
    std::uint32_t sumOfPriceTimesDelta{ 0 };
    for(std::size_t idx{ 1 }; idx < measurements.size(); ++idx)
    {
        const auto timeDelta{ (measurements[idx].timestamp - measurements[idx - 1].timestamp).count() };

        sumOfPriceDelta += timeDelta;
        sumOfPriceTimesDelta += priceFor(measurements[idx], type).value_or(0) * timeDelta;
    }

    return sumOfPriceTimesDelta / sumOfPriceDelta;
}

[[nodiscard]] BasicStats analyzeBasicStats(std::span<const Measurement> measurements, FuelType type)
{
    BasicStats stats{
        .min = findMin(measurements, type),
        .max = findMax(measurements, type),
        .mean = calculateMean(measurements, type),
        .median = calculateMedian(measurements, type),
        .sampleCount = measurements.size(),
        .priceChagnedCount = countPriceChanges(measurements, type),
    };

    // TODO: Determine price at opening and closing time.

    return stats;
}

[[nodiscard]] AdvancedStats analyzeAdvancedStats(
    std::span<const Measurement> measurements, FuelType type, PriceCents mean
)
{
    AdvancedStats stats{
        .stdDeviation = calculateStdDeviation(measurements, type, mean),
        .timeWeightAverage = calculateTimeWeightAverage(measurements, type),
    };

    return stats;
}

[[nodiscard]] CheapestWindow analyzeCheapestWindow(std::span<const Measurement> measurements, FuelType type)
{
    CheapestWindow window{};

    return window;
}

} // namespace

void DailyAnalyzer::analyze()
{
    using DataPerStation = std::unordered_map<std::string, std::vector<Measurement>>;

    // NOTE: Step 1: Gather raw data
    const auto startTime{ std::chrono::system_clock::now() };
    auto data{ m_repo.loadMeasurements(startTime, startTime - std::chrono::days{ 1 }) };

    // NOTE: Step 2: Map data to stations
    DataPerStation dataPerStation;
    for(auto idx{ 0 }; idx < data.size(); ++idx)
    {
        const auto& id{ data[idx].stationId };

        if(!dataPerStation.contains(id))
            dataPerStation.emplace(id, std::vector<Measurement>{});

        dataPerStation[id].emplace_back(std::move(data[idx]));
    }

    std::ranges::for_each(dataPerStation, [](DataPerStation::value_type& station) {
        std::ranges::sort(station.second, {}, &Measurement::timestamp);
    });

    // NOTE: Step 3: Analyze the station data
    std::ranges::for_each(dataPerStation, [&result = m_lastResult](const DataPerStation::value_type& station) {
        StationAnalysis analysis{
            .stationId = station.first,
        };

        std::ranges::for_each(ALL_FUEL_TYPES, [&station, &analysis](FuelType type) {
            analysis.basicStats[type] = analyzeBasicStats(station.second, type);
            analysis.advancedStats[type] = analyzeAdvancedStats(station.second, type, analysis.basicStats[type].mean);
            analysis.cheapestWindow[type] = analyzeCheapestWindow(station.second, type);
        });

        result.emplace_back(std::move(analysis));
    });
}

} // namespace ful::fuel
