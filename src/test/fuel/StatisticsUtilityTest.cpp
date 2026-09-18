#include "fuel/StatisticsUtility.hpp"
#include "fuel/Domain.hpp"
#include "fuel/FuelType.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <cmath>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace
{

using namespace ful::fuel;
using TimePoint = std::chrono::system_clock::time_point;

Measurement makeMeasurement(
    TimePoint timestamp,
    std::optional<PriceCents> e5 = std::nullopt,
    std::optional<PriceCents> e10 = std::nullopt,
    std::optional<PriceCents> diesel = std::nullopt,
    std::string stationId = "station-1"
)
{
    return { .stationId = stationId, .timestamp = timestamp, .e5 = e5, .e10 = e10, .diesel = diesel };
}

TimePoint atHour(int hour)
{
    constexpr std::chrono::year_month_day DAY{
        std::chrono::year{ 2024 },
        std::chrono::month{ 1 },
        std::chrono::day{ 1 },
    };

    return std::chrono::sys_days{ DAY } + std::chrono::hours{ hour };
}

StationAnalysis makeAnalysis(std::string id, PriceCents e5Mean)
{
    StationAnalysis a{ .stationId = std::move(id), .basicStats = {}, .advancedStats = {}, .cheapestWindow = {} };
    a.basicStats.e5.mean = e5Mean;

    return a;
}

} // namespace

namespace ful::testing
{

/// \brief Test that gathering stats from an empty span reports a count of zero.
TEST(AccumulateStatsTest, EmptySpanGivesZeroCount)
{
    const auto stats{ stats::accumulateRunningStats({}, FuelType::E5) };

    EXPECT_EQ(stats.count, 0);
}

/// \brief Test that count, sum, sum of squares, min, and max are correct when every measurement has a price.
TEST(AccumulateStatsTest, AllPresentPricesAreAggregatedCorrectly)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(start + std::chrono::hours(1), 200),
        ::makeMeasurement(start + std::chrono::hours(2), 300),
    };

    const auto stats{ stats::accumulateRunningStats(measurements, FuelType::E5) };

    EXPECT_EQ(stats.count, 3);
    EXPECT_EQ(stats.sum, 600);
    EXPECT_EQ(stats.sumSq, 140000.0);
    EXPECT_EQ(stats.min, 100);
    EXPECT_EQ(stats.max, 300);
}

/// \brief Test that measurements that are missing a price are excluded from statistical analysis.
TEST(AccumulateStatsTest, MissingPricesAreExcluded)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(start + std::chrono::hours(1), std::nullopt),
        ::makeMeasurement(start + std::chrono::hours(2), 300),
    };

    const auto stats{ stats::accumulateRunningStats(measurements, FuelType::E5) };

    EXPECT_EQ(stats.count, 2);
    EXPECT_EQ(stats.sum, 400);
    EXPECT_EQ(stats.sumSq, 100000.0);
    EXPECT_EQ(stats.min, 100);
    EXPECT_EQ(stats.max, 300);
}

/// \brief Test that a measurement that never has a price for fuel results in a zero count.
TEST(AccumulateStatsTest, AllMissingPricesGiveZeroCount)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, std::nullopt),
        ::makeMeasurement(start + std::chrono::hours(1), std::nullopt),
    };

    const auto stats{ stats::accumulateRunningStats(measurements, FuelType::E5) };

    EXPECT_EQ(stats.count, 0);
}

/// \brief Test that a single present price becomes both min and max.
TEST(AccumulateStatsTest, SingleElementIsItsOwnMinAndMax)
{
    const std::vector<Measurement> measurements{
        ::makeMeasurement(std::chrono::system_clock::now(), 100),
    };

    const auto stats{ stats::accumulateRunningStats(measurements, FuelType::E5) };

    EXPECT_EQ(stats.count, 1);
    EXPECT_EQ(stats.sum, 100);
    EXPECT_EQ(stats.sumSq, 10000.0);
    EXPECT_EQ(stats.min, 100);
    EXPECT_EQ(stats.max, 100);
}

/// \brief Test that an empty span reports a median of 0.
TEST(CalculateMedianTest, EmptySpanReturnsZero)
{
    EXPECT_EQ(stats::calculateMedian({}, FuelType::E5), 0);
}

/// \brief Test that a single value is its own median.
TEST(CalculateMedianTest, SingleValueReturnsThatValue)
{
    const std::vector<Measurement> measurements{ ::makeMeasurement(std::chrono::system_clock::now(), 100) };

    EXPECT_EQ(stats::calculateMedian(measurements, FuelType::E5), 100);
}

/// \brief Test that an odd count returns the middle value by price, regadless of the input's temporal order.
TEST(CalculateMedianTest, OddCountReturnsMiddleValueRegardlessofInputOrder)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 300),
        ::makeMeasurement(start + std::chrono::hours(1), 100),
        ::makeMeasurement(start + std::chrono::hours(2), 200),
    };

    EXPECT_EQ(stats::calculateMedian(measurements, FuelType::E5), 200);
}

/// \brief Test that an even count averages the two middle values.
TEST(CalculateMedianTest, EvenCountReturnsAverageOfTwoMiddleValues)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(start + std::chrono::hours(1), 200),
        ::makeMeasurement(start + std::chrono::hours(2), 300),
        ::makeMeasurement(start + std::chrono::hours(3), 400),
    };

    EXPECT_EQ(stats::calculateMedian(measurements, FuelType::E5), 250);
}

/// \brief Test that measurements missing a price are excluded before calculating the median.
TEST(CalculateMedianTest, MissingPricesAreExcludedBeforeComputingMedian)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(start + std::chrono::hours(1), std::nullopt),
        ::makeMeasurement(start + std::chrono::hours(2), 300),
        ::makeMeasurement(start + std::chrono::hours(3), std::nullopt),
        ::makeMeasurement(start + std::chrono::hours(4), 200),
    };

    EXPECT_EQ(stats::calculateMedian(measurements, FuelType::E5), 200);
}

/// \brief Test that duplicate/tied prices are correctly handled by the partial-sort based median.
TEST(CalculateMedianTest, DuplicateValuesAreHandeledCorrectly)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(start + std::chrono::hours(1), 100),
        ::makeMeasurement(start + std::chrono::hours(2), 100),
        ::makeMeasurement(start + std::chrono::hours(3), 100),
    };

    EXPECT_EQ(stats::calculateMedian(measurements, FuelType::E5), 100);
}

/// \brief Test that an empty or single-element span reports zero changes.
TEST(CountPriceChangesTest, EmptyOrSingleElementReturnsZero)
{
    const auto start{ std::chrono::system_clock::now() };

    EXPECT_EQ(stats::countPriceChanges({}, FuelType::E5), 0);
    EXPECT_EQ(stats::countPriceChanges(std::vector{ ::makeMeasurement(start, 100) }, FuelType::E5), 0);
}

/// \brief Test that a constant price reports zero price changes.
TEST(CountPriceChangesTest, ConstantPricesReturnsZero)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(start + std::chrono::hours(1), 100),
        ::makeMeasurement(start + std::chrono::hours(2), 100),
    };

    EXPECT_EQ(stats::countPriceChanges(measurements, FuelType::E5), 0);
}

/// \brief Test that a price alternating every step reports n-1 changes.
TEST(CountPriceChangesTest, AlternatingPricesReturnNMinusOne)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(start + std::chrono::hours(1), 200),
        ::makeMeasurement(start + std::chrono::hours(2), 100),
        ::makeMeasurement(start + std::chrono::hours(2), 200),
    };

    EXPECT_EQ(stats::countPriceChanges(measurements, FuelType::E5), 3);
}

/// \brief Test that a transition between a present price and a missing one counts as a change.
TEST(CountPriceChangesTest, TransitionToOrFromMissingPriceCountsAsChange)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(start + std::chrono::hours(1), std::nullopt),
    };

    EXPECT_EQ(stats::countPriceChanges(measurements, FuelType::E5), 1);
}

/// \brief Test that a zero count reports a standard deviation of 0.0.
TEST(StdDeviationFromStatsTest, ZeroCountReturnsZero)
{
    EXPECT_EQ(stats::calculateStdDeviation({}), 0.0);
}

/// \brief Test that constant values report an approximately zero standard deviation.
TEST(StdDeviationFromStatsTest, ConstantValuesGiveApproximatelyZero)
{
    const stats::RunningStats stats{ .count = 5, .sum = 500, .sumSq = 5.0 * 100 * 100 };

    EXPECT_NEAR(stats::calculateStdDeviation(stats), 0.0, 1e-9);
}

/// \brief Test that the standard deviation matches a hand-computed population standard deviation.
TEST(StdDeviationFromStatsTest, MatchesHandComputedPopilationStdDev)
{
    // prices: 1,2,3,4,5 -> mean of 3, population variance of 2, stddev = sqrt(2)
    const stats::RunningStats stats{ .count = 5, .sum = 15, .sumSq = 1 + 4 + 9 + 16 + 25 };

    EXPECT_NEAR(stats::calculateStdDeviation(stats), std::sqrt(2.0), 1e-9);
}

/// \brief Test that fewer than two measurements report a time-weight average of 0.
TEST(CalculateTimeWeightAverageTest, FewerThanTwoMeasurementsReturnsZero)
{
    const auto start{ std::chrono::system_clock::now() };

    EXPECT_EQ(stats::calculateTimeWeightAverage({}, FuelType::E5), 0);
    EXPECT_EQ(stats::calculateTimeWeightAverage(std::vector{ ::makeMeasurement(start, 100) }, FuelType::E5), 0);
}

/// \brief Test that equal-length intervals collapse to the plan average of their trailing prices.
TEST(CalculateTimeWeightAverageTest, EqualIntervalsGiveSimpleAverageOfTrailingPrices)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 50),
        ::makeMeasurement(start + std::chrono::hours(1), 100),
        ::makeMeasurement(start + std::chrono::hours(2), 200),
        ::makeMeasurement(start + std::chrono::hours(3), 300),
    };

    EXPECT_EQ(stats::calculateTimeWeightAverage(measurements, FuelType::E5), 200);
}

/// \brief Test that longer intervals contribute proportionally more weight to the average.
TEST(CalculateTimeWeightAverageTest, LongerIntervalWeighsMore)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 50),
        ::makeMeasurement(start + std::chrono::hours(1), 100),
        ::makeMeasurement(start + std::chrono::hours(4), 200),
    };

    EXPECT_EQ(stats::calculateTimeWeightAverage(measurements, FuelType::E5), 175);
}

/// \brief Test that an interval whose price is missing is skipped entirely, not diluting the average with its duration.
TEST(CalculateTimeWeightAverageTest, IntervalWithMissingPricesIsSkippedEntirely)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 50),
        ::makeMeasurement(start + std::chrono::hours(1), std::nullopt),
        ::makeMeasurement(start + std::chrono::hours(3), 200),
    };

    EXPECT_EQ(stats::calculateTimeWeightAverage(measurements, FuelType::E5), 200);
}

/// \brief Test that when every subsequent interval's price is missing, the guard against zero total weight returns 0.
TEST(CalculateTimeWeightAverageTest, AllSubsequentPricesMissingReturnsZero)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 50),
        ::makeMeasurement(start + std::chrono::hours(1), std::nullopt),
    };

    EXPECT_EQ(stats::calculateTimeWeightAverage(measurements, FuelType::E5), 0);
}

/// \brief Test that fewer than two measurements report zero changes per hour.
TEST(CalculatePriceChangesPerHourTest, FewerThanTwoMeasurementsReturnsZero)
{
    EXPECT_EQ(stats::calculatePriceChangesPerHour({}, 5), 0.0);
}

/// \brief Test that a zero timespan (identical timestamps) reports zero changes per hour rather than dividing by zero.
TEST(CalculatePriceChangesPerHourTest, ZeroTimeSpanReturnsZero)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(start, 200),
    };

    EXPECT_EQ(stats::calculatePriceChangesPerHour(measurements, 5), 0.0);
}

/// \brief Test that a known span and change count divide exactly.
TEST(CalculatePriceChangesPerHourTest, KnownSpanAndCountDivideExactly)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(start + std::chrono::hours(2), 200),
    };

    EXPECT_EQ(stats::calculatePriceChangesPerHour(measurements, 5), 2.5);
}

/// \brief Test that fewer than two present values report a slope of 0.
TEST(CalculateTrendSlopePerHourTest, FewerThanTwoPresentValuesReturnsZero)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(start + std::chrono::hours(1), std::nullopt),
    };

    EXPECT_EQ(stats::calculateTrendSlopePerHour(measurements, FuelType::E5), 0.0);
}

/// \brief Test that a perfectly linear price series gives the exact slope.
TEST(CalculateTrendSlopePerHourTest, PerfectlyLinearSeriesGivesExactSlope)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 1000),
        ::makeMeasurement(start + std::chrono::hours(1), 1010),
        ::makeMeasurement(start + std::chrono::hours(2), 1020),
    };

    EXPECT_NEAR(stats::calculateTrendSlopePerHour(measurements, FuelType::E5), 10.0, 1e-9);
}

/// \brief Test that a flat price series gives a slope of 0.
TEST(CalculateTrendSlopePerHourTest, FlatSeriesGivesZeroSlope)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 1000),
        ::makeMeasurement(start + std::chrono::hours(1), 1000),
        ::makeMeasurement(start + std::chrono::hours(2), 1000),
    };

    EXPECT_NEAR(stats::calculateTrendSlopePerHour(measurements, FuelType::E5), 0.0, 1e-9);
}

/// \brief Test that measurements sharing an identical timestamp return 0 instead of NaN.
TEST(CalculateTrendSlopePerHourTest, IdenticalTimestampReturnsZeroInsteadOfNaN)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(start, 200),
    };

    EXPECT_EQ(stats::calculateTrendSlopePerHour(measurements, FuelType::E5), 0.0);
}

/// \brief Test that no data at all defaults to hour 0 indicating that no data was available.
TEST(CalculateCheapestHourOfDayTest, NoDataDefaultsToHourZero)
{
    EXPECT_EQ(stats::calculateCheapestHourOfDay({}, FuelType::E5), 0);
}

/// \brief Test that the hour with the lowest average price is found correctly
TEST(CalculateCheapestHourOfDayTest, KnownCheapestIsFound)
{
    const std::vector<Measurement> measurements{
        ::makeMeasurement(::atHour(5), 100),
        ::makeMeasurement(::atHour(10), 50),
        ::makeMeasurement(::atHour(15), 200),
    };

    EXPECT_EQ(stats::calculateCheapestHourOfDay(measurements, FuelType::E5), 10);
}

/// \brief Test that a tie between two hours resolves to the earlier hour of the day.
TEST(CalculateCheapestHourOfDayTest, TieBetweenHoursReturnsEarlierHourOfDay)
{
    const std::vector<Measurement> measurements{
        ::makeMeasurement(::atHour(7), 100),
        ::makeMeasurement(::atHour(3), 100),
    };

    EXPECT_EQ(stats::calculateCheapestHourOfDay(measurements, FuelType::E5), 3);
}

/// \brief Test that an empty span returns a default window with price 0.
TEST(AnalyzeCheapestWindowTest, EmptySpanReturnsZeroPriceDefaultWindow)
{
    const auto window{ stats::analyzeCheapestWindow({}, FuelType::E5) };

    EXPECT_EQ(window.price, std::numeric_limits<PriceCents>::max());
    EXPECT_EQ(window.start, std::chrono::system_clock::time_point{});
    EXPECT_EQ(window.end, std::chrono::system_clock::time_point{});
}

/// \brief Test that a single measurement is its own window.
TEST(AnalyzeCheapestWindowTest, SingleMeasurementIsItsOwnWindow)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{ ::makeMeasurement(start, 100) };

    const auto window{ stats::analyzeCheapestWindow(measurements, FuelType::E5) };

    EXPECT_EQ(window.price, 100);
    EXPECT_EQ(window.start, start);
    EXPECT_EQ(window.end, start);
}

/// \brief Test that when two runs share the same minimum price, the first (earliest) run is kept.
TEST(AnalyzeCheapestWindowTest, KeepsFirstRunWhenMultipleRunsShareMinimumPrice)
{
    const auto start{ std::chrono::system_clock::now() };
    const auto t1{ start + std::chrono::hours(1) };
    const auto t2{ start + std::chrono::hours(2) };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(t1, 200),
        ::makeMeasurement(t2, 100),
    };

    const auto window{ stats::analyzeCheapestWindow(measurements, FuelType::E5) };

    EXPECT_EQ(window.price, 100);
    EXPECT_EQ(window.start, start);
    EXPECT_EQ(window.end, t1);
}

/// \brief Test that no measurement having a price for the fuel returns a zero-price default window.
TEST(AnalyzeCheapestWindowTest, AllMissingReturnsZeroPrice)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, std::nullopt),
    };

    const auto window{ stats::analyzeCheapestWindow(measurements, FuelType::E5) };

    EXPECT_EQ(window.price, std::numeric_limits<PriceCents>::max());
    EXPECT_EQ(window.start, std::chrono::system_clock::time_point{});
    EXPECT_EQ(window.end, std::chrono::system_clock::time_point{});
}

/// \brief Test that opening/closing skip leading/trailing measurements that are missing price data for the fuel.
TEST(AnalyzeBasicStatsTest, OpeningAndClosingSkipLeadingAndTrailingMissingPrices)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, std::nullopt),
        ::makeMeasurement(start + std::chrono::hours(1), 100),
        ::makeMeasurement(start + std::chrono::hours(2), 200),
        ::makeMeasurement(start + std::chrono::hours(3), std::nullopt),
    };

    const auto stats{ stats::accumulateRunningStats(measurements, FuelType::E5) };
    const auto changes{ stats::countPriceChanges(measurements, FuelType::E5) };
    const auto basic{ stats::analyzeBasicStats(measurements, FuelType::E5, stats, changes) };

    EXPECT_EQ(basic.opening, 100);
    EXPECT_EQ(basic.closing, 200);
    EXPECT_EQ(basic.sampleCount, 4);
}

/// \brief Test that percentile rank and previous-day delta are always left unset at single-station scope.
TEST(AnalyzeAdvancedStatsTest, PercentileRankAndDeltaAreLeftAsNulloptAtThisScope)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100),
        ::makeMeasurement(start + std::chrono::hours(1), 200),
    };

    const auto stats{ stats::accumulateRunningStats(measurements, FuelType::E5) };
    const auto changes{ stats::countPriceChanges(measurements, FuelType::E5) };
    const auto advanced{ stats::analyzeAdvancedStats(measurements, FuelType::E5, stats, changes) };

    EXPECT_FALSE(advanced.percentileRankAmongStations.has_value());
    EXPECT_FALSE(advanced.deltaFromPreviousDay.has_value());
}

/// \brief Test that multiple stations get independent per-fuel means.
TEST(ComputeMeansPerStationTest, MultipleStationsGetIndependentMeans)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100, std::nullopt, std::nullopt, "station-a"),
        ::makeMeasurement(start + std::chrono::hours(1), 200, std::nullopt, std::nullopt, "station-a"),
        ::makeMeasurement(start, 10, std::nullopt, std::nullopt, "station-b"),
        ::makeMeasurement(start + std::chrono::hours(1), 20, std::nullopt, std::nullopt, "station-b"),
        ::makeMeasurement(start + std::chrono::hours(2), 30, std::nullopt, std::nullopt, "station-b"),
    };

    const auto means{ stats::computeMeansPerStation(measurements) };

    ASSERT_TRUE(means.contains("station-a"));
    ASSERT_TRUE(means.contains("station-b"));
    EXPECT_EQ(means.at("station-a").e5, 150);
    EXPECT_EQ(means.at("station-b").e5, 20);
}

/// \brief Test that a station never reporting a fuel's price gets a mean of 0 for it.
TEST(ComputeMeansPerStationTest, StationMissingFuelEntirelyGetsZeroMean)
{
    const auto start{ std::chrono::system_clock::now() };
    const std::vector<Measurement> measurements{
        ::makeMeasurement(start, 100, std::nullopt, std::nullopt, "station-a"),
    };

    const auto means{ stats::computeMeansPerStation(measurements) };

    ASSERT_TRUE(means.contains("station-a"));
    EXPECT_EQ(means.at("station-a").diesel, 0);
}

/// \brief Test that a single station is left with no percentile rank, since ranking needs at least two stations.
TEST(AssignPercentileRanksTest, SingleStationLeavesRankAsNullopt)
{
    constexpr auto E5_PRICE{ 100 };

    std::vector<StationAnalysis> results{ makeAnalysis("station-1", E5_PRICE) };

    stats::assignPercentileRanks(results);

    EXPECT_FALSE(results.front().advancedStats.e5.percentileRankAmongStations.has_value());
}

/// \brief Test that three stations with distinct means get the expected 0%/50%/100% ranks.
TEST(AssignPercentileRanksTest, ThreeStationsWithDistinctMeansGetExpectedRanks)
{
    constexpr auto CHEAP_PRICE{ 100 };
    constexpr auto MIDDLE_PRICE{ 200 };
    constexpr auto EXPENSIVE_PRICE{ 300 };

    std::vector<StationAnalysis> results{
        ::makeAnalysis("cheapest", CHEAP_PRICE),
        ::makeAnalysis("middle", MIDDLE_PRICE),
        ::makeAnalysis("expensive", EXPENSIVE_PRICE),
    };

    stats::assignPercentileRanks(results);

    /// NOLINTBEGIN(bugprone-unchecked-optional-access): Not technically unchecked
    ASSERT_TRUE(results.at(0).advancedStats.e5.percentileRankAmongStations.has_value());
    ASSERT_TRUE(results.at(1).advancedStats.e5.percentileRankAmongStations.has_value());
    ASSERT_TRUE(results.at(2).advancedStats.e5.percentileRankAmongStations.has_value());
    EXPECT_EQ(*results.at(0).advancedStats.e5.percentileRankAmongStations, 0.0);
    EXPECT_EQ(*results.at(1).advancedStats.e5.percentileRankAmongStations, 50.0);
    EXPECT_EQ(*results.at(2).advancedStats.e5.percentileRankAmongStations, 100.0);
    /// NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Test that stations sharing a tied mean receive the same percentile rank.
TEST(AssignPercentileRanksTest, TiedMeansShareTheSameRank)
{
    constexpr auto FUEL_PRICE{ 100 };

    std::vector<StationAnalysis> results{
        makeAnalysis("tied-a", FUEL_PRICE),
        makeAnalysis("tied-b", FUEL_PRICE),
        makeAnalysis("expensive", FUEL_PRICE * 2),
    };

    stats::assignPercentileRanks(results);

    /// NOLINTBEGIN(bugprone-unchecked-optional-access): Not technically unchecked
    ASSERT_TRUE(results[0].advancedStats.e5.percentileRankAmongStations.has_value());
    ASSERT_TRUE(results[1].advancedStats.e5.percentileRankAmongStations.has_value());
    ASSERT_TRUE(results[2].advancedStats.e5.percentileRankAmongStations.has_value());
    EXPECT_EQ(*results[0].advancedStats.e5.percentileRankAmongStations, 50.0);
    EXPECT_EQ(*results[1].advancedStats.e5.percentileRankAmongStations, 50.0);
    EXPECT_EQ(*results[2].advancedStats.e5.percentileRankAmongStations, 100.0);
    /// NOLINTEND(bugprone-unchecked-optional-access)
}

} // namespace ful::testing
