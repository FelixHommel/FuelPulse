#include "fuel/ConfigLoader.hpp"
#include "fuel/FuelPulseConfig.hpp"
#include "testUtility/EnvVarGuard.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <cstdlib>
#include <exception>
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
constexpr auto TEST_INVALID_JSON{ R"(
{
    "max_stations": 1
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

/// \brief Test that a valid config can be read from a string.
TEST(ConfigLoaderTest, LoadConfigFromString)
{
    const auto conf{ fuel::loadConfig(std::string(BASIC_TEST_JSON)) };

    EXPECT_EQ(conf, BASIC_TEST_JSON_REFERENCE);
}

/// \brief Test that trying to load a JSON string that is not valid JSON format will throw and return a default config.
TEST(ConfigLoaderTest, LoadingInvalidConfigThrows)
{
    fuel::FuelPulseConfig config;

    ASSERT_THROW(config = fuel::loadConfig(std::string(TEST_INVALID_JSON)), std::exception);
    EXPECT_EQ(config, fuel::FuelPulseConfig{});
}

TEST(ConfigLoaderApiKeyTest, ApiKeyIsPopulatedFromEnvironmentWhenPresent)
{
    EnvVarGuard guard{ "TANKER_KOENIG_API_KEY", "test-key-1" };

    const auto config{ fuel::loadConfig(std::string(BASIC_TEST_JSON)) };

    // NOLINTBEGIN(bugprone-unchecked-optional-access): Not unchecked
    EXPECT_TRUE(config.apiKey.has_value());
    EXPECT_EQ(config.apiKey.value(), "test-key-1");
    // NOLINTEND(bugprone-unchecked-optional-access)
}

TEST(ConfigLoaderApiKeyTest, ApiKeyIsNotPopulatedFromEnvironmentWhenNotPresent)
{
    if([[maybe_unused]] const char* existing{ std::getenv("TANKER_KOENIG_API_KEY") })
    {
#ifdef _WIN32
        _putenv_s("TANKER_KOENIG_API_KEY", "");
#else
        unsetenv("TANKER_KOENIG_API_KEY");
#endif
    }

    const auto config{ fuel::loadConfig(std::string(BASIC_TEST_JSON)) };

    EXPECT_FALSE(config.apiKey.has_value());
}

} // namespace ful::testing
