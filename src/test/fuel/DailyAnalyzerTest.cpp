#include "fuel/DailyAnalyzer.hpp"

#include <gtest/gtest.h>

#include "fuel/Domain.hpp"
#include "testUtility/FakeFuelRepository.hpp"

#include <chrono>
#include <optional>
#include <string>
#include <utility>

namespace
{

using TimePoint = std::chrono::system_clock::time_point;

constexpr std::chrono::year_month_day FIXED_DAY{
    std::chrono::year{ 2024 },
    std::chrono::month{ 6 },
    std::chrono::day{ 15 },
};

const TimePoint FIXED_NOW{ std::chrono::sys_days{ FIXED_DAY } + std::chrono::hours{ 12 } };
const TimePoint YESTERDAY_START{ FIXED_NOW - std::chrono::days{ 1 } };
const TimePoint DAY_BEFORE_START{ FIXED_NOW - std::chrono::days{ 2 } };

ful::fuel::DailyAnalyzer::StartPointProvider fixedNow()
{
    return []() {
        return FIXED_NOW;
    };
}

ful::fuel::Measurement makeMeasurement(std::string stationId, TimePoint timestamp, ful::fuel::PriceCents e5)
{
    return {
        .stationId = std::move(stationId), .timestamp = timestamp, .e5 = e5, .e10 = std::nullopt, .diesel = std::nullopt
    };
}

} // namespace

namespace ful::testing
{

/// \brief Test fixture wiring a \ref fuel::DailyAnalyzer to a \ref FakeFuelRepository with a fixed understanding of
///     what "now" means, so that the analyzed day is deterministic rather than depending on real wall-clock time.
class DailyAnalyzerTest : public ::testing::Test
{
public:
    DailyAnalyzerTest() : m_analyzer{ &m_repo, ::fixedNow() } {}
    ~DailyAnalyzerTest() override = default;

    DailyAnalyzerTest(const DailyAnalyzerTest&) = delete;
    DailyAnalyzerTest& operator=(const DailyAnalyzerTest&) = delete;
    DailyAnalyzerTest(DailyAnalyzerTest&&) = delete;
    DailyAnalyzerTest& operator=(DailyAnalyzerTest&&) = delete;

protected:
    FakeFuelRepository m_repo;
    fuel::DailyAnalyzer m_analyzer;
};

using DailyAnalyzerEmptyTest = DailyAnalyzerTest;

/// \brief Test that analyzing an empty repository produces an empty result rather than throwing an exception.
TEST_F(DailyAnalyzerEmptyTest, EmptyRepositoryProducesEmptyResult)
{
    EXPECT_NO_FATAL_FAILURE(m_analyzer.analyze());

    EXPECT_TRUE(m_analyzer.lastResult().empty());
}

using DailyAnalyzerBasicTest = DailyAnalyzerTest;

/// \brief Test that a single station's basic stats are populated from measurements within the analysis window.
TEST_F(DailyAnalyzerBasicTest, SingleStationStatsArePopulated)
{
    // NOLINTBEGIN(readability-magic-numbers): Sample measurement data initialization
    m_repo.store(::makeMeasurement("station-1", YESTERDAY_START + std::chrono::hours{ 1 }, 100));
    m_repo.store(::makeMeasurement("station-1", YESTERDAY_START + std::chrono::hours{ 2 }, 150));
    m_repo.store(::makeMeasurement("station-1", YESTERDAY_START + std::chrono::hours{ 3 }, 120));
    // NOLINTEND(readability-magic-numbers)

    m_analyzer.analyze();
    const auto result{ m_analyzer.lastResult() };

    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result.front().stationId, "station-1");

    const auto& e5Stats{ result.front().basicStats.e5 };
    EXPECT_EQ(e5Stats.sampleCount, 3);
    EXPECT_EQ(e5Stats.min, 100);
    EXPECT_EQ(e5Stats.max, 150);
    EXPECT_EQ(e5Stats.opening, 100);
    EXPECT_EQ(e5Stats.closing, 120);
}

using DailyAnalyzerPercentileTest = DailyAnalyzerTest;

/// \brief Test that a cheaper station ends up with a lower percentile rank than a more expensive station.
TEST_F(DailyAnalyzerPercentileTest, CheaperStationGetsLowerPercentileRank)
{
    // NOLINTBEGIN(readability-magic-numbers): Sample measurement data initialization
    m_repo.store(::makeMeasurement("station-cheap", YESTERDAY_START + std::chrono::hours{ 1 }, 100));
    m_repo.store(::makeMeasurement("station-expensive", YESTERDAY_START + std::chrono::hours{ 1 }, 200));
    // NOLINTEND(readability-magic-numbers)

    m_analyzer.analyze();
    const auto results{ m_analyzer.lastResult() };

    ASSERT_EQ(results.size(), 2);

    for(const auto& station : results)
    {
        ASSERT_TRUE(station.advancedStats.e5.percentileRankAmongStations.has_value());

        // NOLINTBEGIN(bugprone-unchecked-optional-access)
        if(station.stationId == "station-cheap")
            EXPECT_LT(*station.advancedStats.e5.percentileRankAmongStations, 50.0);
        else
            EXPECT_GT(*station.advancedStats.e5.percentileRankAmongStations, 50.0);
        // NOLINTEND(bugprone-unchecked-optional-access)
    }
}

using DailyAnalyzerDeltaTest = DailyAnalyzerTest;

/// \brief Test that deltaFromPreviousDay reflects the difference between yesterday's and the day before's mean price.
TEST_F(DailyAnalyzerDeltaTest, DeltaFromPreviousDayReflectsMeanDifference)
{
    // NOLINTBEGIN(readability-magic-numbers): Sample measurement data initialization
    m_repo.store(::makeMeasurement("station-1", DAY_BEFORE_START + std::chrono::hours{ 1 }, 140));
    m_repo.store(::makeMeasurement("station-1", DAY_BEFORE_START + std::chrono::hours{ 2 }, 160));

    m_repo.store(::makeMeasurement("station-1", YESTERDAY_START + std::chrono::hours{ 1 }, 190));
    m_repo.store(::makeMeasurement("station-1", YESTERDAY_START + std::chrono::hours{ 2 }, 210));
    // NOLINTEND(readability-magic-numbers)

    m_analyzer.analyze();
    const auto results{ m_analyzer.lastResult() };

    ASSERT_EQ(results.size(), 1);

    const auto& delta{ results.front().advancedStats.e5.deltaFromPreviousDay };
    ASSERT_TRUE(delta.has_value());
    // NOLINTBEGIN(bugprone-unchecked-optional-access)
    EXPECT_EQ(*delta, 50);
    // NOLINTEND(bugprone-unchecked-optional-access)
}

} // namespace ful::testing
