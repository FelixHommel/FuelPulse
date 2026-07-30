#include "fuel/ConfigLoader.hpp"
#include "fuel/FuelPulseConfig.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <string>

namespace ful::testing
{

namespace
{

constexpr auto BASIC_TEST_JSON{ R"(
{
    "max_stations": 1,
    "postal_code": 1,
    "search_radius": 1.0,
    "collection_interval": 1,
    "database_path": "test.db3",
    "report_dir": "reports"
}
)" };
const fuel::FuelPulseConfig BASIC_TEST_JSON_REFERENCE{
    .maxStations = 1,
    .postalCode = 1,
    .searchRadius = 1.f,
    .collectionInterval = std::chrono::minutes(1),
    .databasePath = "test.db3",
    .reportDir = "reports",
};

} // namespace

TEST(ConfigLoaderTest, LoadConfigFromString)
{
    const auto conf{ fuel::loadConfig(std::string(BASIC_TEST_JSON)) };

    EXPECT_EQ(conf, BASIC_TEST_JSON_REFERENCE);
}

} // namespace ful::testing
