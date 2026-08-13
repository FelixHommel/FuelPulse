#include "fuel/FuelPulseConfig.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json_fwd.hpp>

#include <chrono>
#include <filesystem>

namespace ful::testing
{

/// \brief Test that a document that specifies every field overrides every field of the default config.
TEST(FuelPulseConfigFromJsonTest, AllFieldsAreOverridenWhenPresent)
{
    const auto json{ nlohmann::json::parse(R"(
{
    "max_stations": 3,
    "postal_code": 12345,
    "search_radius": 2.5,
    "collection_interval": 15,
    "database_path": "custom.db3",
    "report_dir": "customReports"
}
    )") };

    const auto config{ json.get<fuel::FuelPulseConfig>() };

    EXPECT_EQ(config.maxStations, 3);
    EXPECT_EQ(config.postalCode, 12345);
    EXPECT_EQ(config.searchRadius, 2.5f);
    EXPECT_EQ(config.collectionInterval, std::chrono::minutes(15));
    EXPECT_EQ(config.databasePath, std::filesystem::path("custom.db3"));
    EXPECT_EQ(config.reportDir, std::filesystem::path("customReports"));
}

/// \brief Test that an empty document leaves every field at its default value.
TEST(FuelPulseConfigFromJsonTest, EmptyDocumentKeepsDefaults)
{
    EXPECT_EQ(nlohmann::json::parse("{}").get<fuel::FuelPulseConfig>(), fuel::FuelPulseConfig{});
}

/// \brief Test that a document specifying only a single field overrides just that field and leaves everything else at
///     the default value.
TEST(FuelPulseConfigFromJsonTest, PartialDocumentOnlyOverridesSpecifiedFields)
{
    const fuel::FuelPulseConfig DEFAULTS{};
    const auto config{ nlohmann::json::parse(R"({ "max_stations": 7 })").get<fuel::FuelPulseConfig>() };

    EXPECT_EQ(config.maxStations, 7);
    EXPECT_NE(config.maxStations, DEFAULTS.maxStations);
    EXPECT_EQ(config.postalCode, DEFAULTS.postalCode);
    EXPECT_EQ(config.searchRadius, DEFAULTS.searchRadius);
    EXPECT_EQ(config.collectionInterval, DEFAULTS.collectionInterval);
    EXPECT_EQ(config.databasePath, DEFAULTS.databasePath);
    EXPECT_EQ(config.reportDir, DEFAULTS.reportDir);
}

/// \brief Test that \p "collection_interval" is interpreted as whole minutes.
TEST(FuelPulseConfigFromJsonTest, CollectionIntercalIsInterpretedAsMinutes)
{
    const auto config{ nlohmann::json::parse(R"({ "collection_interval": 42 })").get<fuel::FuelPulseConfig>() };

    EXPECT_EQ(config.collectionInterval, std::chrono::minutes(42));
}

} // namespace ful::testing
