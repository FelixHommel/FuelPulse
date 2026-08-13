#include "fuel/Domain.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <optional>
#include <string>
#include <tuple>
#include <unordered_set>
#include <utility>

namespace ful::testing
{

namespace
{

/// \brief Produce a \ref fuel::Station object.
///
/// \param id (optional) Specify the id of the \ref fuel::Station
///
/// \returns \ref fuel::Station object
fuel::Station makeStation(std::string id = "station-1")
{
    // NOLINTNEXTLINE(readability-magic-numbers): Arbitrary but fixed coordinates
    return { .id = std::move(id), .name = "Test", .brand = "TestBrand", .longitude = 9.1777, .latitude = 48.7758 };
}

} // namespace

/// \brief Test that equal \ref fuel::Station objects produce the same hash value.
TEST(DomainStationHashTest, EqualStationsHashEqually)
{
    EXPECT_EQ(std::hash<fuel::Station>{}(makeStation()), std::hash<fuel::Station>{}(makeStation()));
}

/// \brief Test that a \ref fuel::Station can be used as a key in \ref std::unordered_set.
TEST(DomainStationHashTest, StationIsUsableInUnorderedSet)
{
    std::unordered_set<fuel::Station> stations;

    stations.insert(makeStation());
    stations.insert(makeStation("station-2"));
    stations.insert(makeStation());

    EXPECT_EQ(stations.size(), 2);
}

/// \brief Test that equal \ref fuel::Measurement objects produce the same hash value.
TEST(DomainMeasurementHashTest, EqualMeasurementHashEqually)
{
    const auto timestamp{ std::chrono::system_clock::now() };
    const fuel::Measurement a{ .stationId = "station-1", .timestamp = timestamp, .e5 = 990, .e10 = 800, .diesel = 950 };
    const fuel::Measurement b{ .stationId = "station-1", .timestamp = timestamp, .e5 = 990, .e10 = 800, .diesel = 950 };

    EXPECT_EQ(std::hash<fuel::Measurement>{}(a), std::hash<fuel::Measurement>{}(b));
}

/// \brief Test that a \ref fuel::Measurement with all nullopt prices still hashes correctly.
TEST(DomainMeasurementHashTest, MeasurementsWithNulloptPricesHashSafely)
{
    const fuel::Measurement m{
        .stationId = "station-1",
        .timestamp = std::chrono::system_clock::now(),
        .e5 = std::nullopt,
        .e10 = std::nullopt,
        .diesel = std::nullopt,
    };

    EXPECT_NO_THROW(std::ignore = std::hash<fuel::Measurement>{}(m));
}

} // namespace ful::testing
