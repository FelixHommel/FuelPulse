#include "DailyAnalyzer.hpp"
#include "fuel/Domain.hpp"
#include "fuel/IFuelRepository.hpp"
#include "fuel/StatisticsUtility.hpp"

#include <algorithm>
#include <bits/chrono.h> // FIXME: Not sure why, but the compilers don't know where to get std::chrono::hours from
#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace ful::fuel
{

namespace
{} // namespace

DailyAnalyzer::DailyAnalyzer(const IFuelRepository* repo, StartPointProvider start) : m_repo{ repo }, m_start{ start }
{}

void DailyAnalyzer::analyze()
{
    using DataPerStation = std::unordered_map<std::string, std::vector<Measurement>>;

    const auto start{ m_start() };
    const auto yesterdayStart{ start - std::chrono::days(1) };
    const auto dayBeforeStart{ start - std::chrono::days(2) };

    auto data{ m_repo->loadMeasurements(yesterdayStart, start) };
    const auto previousDayMeans{
        stats::computeMeansPerStation(m_repo->loadMeasurements(dayBeforeStart, yesterdayStart))
    };

    DataPerStation dataPerStation;
    for(auto& m : data)
        dataPerStation[m.stationId].push_back(std::move(m));

    std::ranges::for_each(dataPerStation, [](DataPerStation::value_type& station) {
        std::ranges::sort(station.second, {}, &Measurement::timestamp);
    });

    m_lastResult.clear();
    for(const auto& [stationId, measurements] : dataPerStation)
    {
        StationAnalysis analysis{
            .stationId = stationId,
        };

        for(auto type : stats::ALL_FUEL_TYPES)
        {
            const auto stats{ stats::accumulateRunningStats(measurements, type) };
            const auto priceChangesCount{ stats::countPriceChanges(measurements, type) };

            analysis.basicStats[type] = stats::analyzeBasicStats(measurements, type, stats, priceChangesCount);
            analysis.advancedStats[type] = stats::analyzeAdvancedStats(measurements, type, stats, priceChangesCount);
            analysis.cheapestWindow[type] = stats::analyzeCheapestWindow(measurements, type);

            if(const auto it{ previousDayMeans.find(stationId) }; it != previousDayMeans.cend())
            {
                analysis.advancedStats[type].deltaFromPreviousDay = std::make_optional(
                    static_cast<std::int64_t>(analysis.basicStats[type].mean)
                    - static_cast<std::int64_t>(it->second[type])
                );
            }
        }

        m_lastResult.emplace_back(std::move(analysis));
    }

    stats::assignPercentileRanks(m_lastResult);
}

} // namespace ful::fuel
