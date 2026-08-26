#ifndef FUL_SRC_LIB_FUEL_DAILY_ANALYZER_HPP
#define FUL_SRC_LIB_FUEL_DAILY_ANALYZER_HPP

#include "analyzer/IAnalyzer.hpp"
#include "fuel/Domain.hpp"
#include "fuel/IFuelRepository.hpp"

#include <chrono>
#include <functional>
#include <utility>
#include <vector>

namespace ful::fuel
{

/// \brief Analyze one day's worth of fuel price measurements per station.
///
/// \author Felix Hommel
/// \date 8/24/2026
class DailyAnalyzer : public IAnalyzer
{
public:
    /// \brief Provider for the point in time treated as "now" when determining which day to analyze.
    using StartPointProvider = std::function<std::chrono::system_clock::time_point()>;

    /// \brief Create a new \ref DailyAnalyzer.
    ///
    /// \param repo The \ref IFuelRepository to load measurements from
    /// \param now (optional) Provider for the current time
    DailyAnalyzer(const IFuelRepository* repo, StartPointProvider start = &std::chrono::system_clock::now);
    ~DailyAnalyzer() override = default;

    DailyAnalyzer(const DailyAnalyzer&) = default;
    DailyAnalyzer& operator=(const DailyAnalyzer&) = delete;
    DailyAnalyzer(DailyAnalyzer&&) = delete;
    DailyAnalyzer& operator=(DailyAnalyzer&&) = delete;

    /// \brief Analyze the day ending at the time point returned by the configured \ref StartPointProvider.
    void analyze() override;

    [[nodiscard]] std::vector<StationAnalysis> lastResult() const { return std::move(m_lastResult); }

private:
    const IFuelRepository* m_repo;
    StartPointProvider m_start;

    std::vector<StationAnalysis> m_lastResult;
};

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_DAILY_ANALYZER_HPP
