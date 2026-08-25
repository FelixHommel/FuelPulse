#ifndef FUL_SRC_LIB_FUEL_DAILY_ANALYZER_HPP
#define FUL_SRC_LIB_FUEL_DAILY_ANALYZER_HPP

#include "analyzer/IAnalyzer.hpp"
#include "fuel/Domain.hpp"
#include "fuel/IFuelRepository.hpp"
#include <utility>
#include <vector>

namespace ful::fuel
{

class DailyAnalyzer : public IAnalyzer
{
public:
    DailyAnalyzer(IFuelRepository& repo);
    ~DailyAnalyzer() override = default;

    DailyAnalyzer(const DailyAnalyzer&) = default;
    DailyAnalyzer& operator=(const DailyAnalyzer&) = delete;
    DailyAnalyzer(DailyAnalyzer&&) = delete;
    DailyAnalyzer& operator=(DailyAnalyzer&&) = delete;

    void analyze() override;

    [[nodiscard]] std::vector<StationAnalysis> lastResult() const { return std::move(m_lastResult); }

private:
    IFuelRepository& m_repo;

    std::vector<StationAnalysis> m_lastResult;
};

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_DAILY_ANALYZER_HPP
