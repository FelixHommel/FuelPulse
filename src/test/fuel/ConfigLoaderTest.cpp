#include "fuel/ConfigLoader.hpp"
#include "fuel/FuelPulseConfig.hpp"
#include "testUtility/EnvVarGuard.hpp"
#include "testUtility/TemporaryFile.hpp"
#include "utility/env/EnvironmentVariableHelper.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json_fwd.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <stdlib.h> // NOLINT(misc-include-cleaner, modernize-deprecated-headers): For some reason unsetenv(…) needs this header
#include <string>

namespace ful::testing
{

namespace
{

constexpr auto BASIC_TEST_JSON{ R"(
{
    "max_stations": 1,
    "postal_code": 55555,
    "search_radius": 1.0,
    "collection_interval": 1,
    "database_path": "test.db3",
    "report_dir": "reports"
}
)" };
constexpr auto TEST_INVALID_JSON{ R"(
{
    "max_stations": 1
    "postal_code": 55555,
    "search_radius": 1.0,
    "collection_interval": 1,
    "database_path": "test.db3",
    "report_dir": "reports"
}
)" };
constexpr auto NOT_SCHEMA_CONFORM_JSON{ R"(
{
    "max_stations": -1,
    "postal_code": -1,
    "search_radius": -1.0,
    "collection_interval": -1,
    "database_path": 12,
    "report_dir": 13
}
)" };
const fuel::FuelPulseConfig BASIC_TEST_JSON_REFERENCE{
    .maxStations = 1,
    .postalCode = 55555,
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

/// \brief Test that an empty input string is not causing fatal issues but instead returns a default config.
TEST(ConfigLoaderTest, LoadingEmptyStringFallsBackToDefault)
{
    fuel::FuelPulseConfig config;

    ASSERT_NO_THROW(config = fuel::loadConfig(std::string{}));
    EXPECT_EQ(config, fuel::FuelPulseConfig{});
}

/// \brief Test that trying to load a JSON string that is not valid JSON format should not throw but return a default \ref FuelPulseConfig.
TEST(ConfigLoaderTest, LoadingInvalidConfigThrows)
{
    fuel::FuelPulseConfig config;

    ASSERT_NO_THROW(config = fuel::loadConfig(std::string(TEST_INVALID_JSON)));
    EXPECT_EQ(config, fuel::FuelPulseConfig{});
}

/// \brief Test that a JSON document missing a key entirely is still handled gracefully by falling back to the defaults
///     instead of propagating the \ref json::out_of_range exception thrown by from_json's at(…) method.
TEST(ConfigLoaderTest, DocumentMissingKeyNotCaughtBySchemaFallsBackToDefault)
{
    constexpr auto MISSING_KEY_JSON{ R"(
{
    "postal_code": 55555,
    "search_radius": 1.0,
    "collection_interval": 1,
    "database_path": "test.db3",
    "report_dir": "reports"
}
    )" };

    fuel::FuelPulseConfig config;

    ASSERT_NO_THROW(config = fuel::loadConfig(std::string(MISSING_KEY_JSON)));
    EXPECT_EQ(config.maxStations, fuel::FuelPulseConfig{}.maxStations);
}

/// \brief Test loading the API key from an environment variable where the environment variable exists should populate
///     the optional in \ref FuelPulseConfig.
TEST(ConfigLoaderApiKeyTest, ApiKeyIsPopulatedFromEnvironmentWhenPresent)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Not unchecked
    EnvVarGuard guard{ "TANKER_KOENIG_API_KEY", "test-key-1" };

    const auto config{ fuel::loadConfig(std::string(BASIC_TEST_JSON)) };

    EXPECT_TRUE(config.apiKey.has_value());
    EXPECT_EQ(config.apiKey.value(), "test-key-1");
    // NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Test loading the API key from an environment variable where the environment variable does not exist, should
///     leave the optional in \ref FuelPulseConfig as a \ref std::nullopt.
TEST(ConfigLoaderApiKeyTest, ApiKeyIsNotPopulatedFromEnvironmentWhenNotPresent)
{
    if(const auto existing{ env::getVar("TANKER_KOENIG_API_KEY") }; existing.has_value())
        env::unsetVar("TANKER_KOENIG_API_KEY");

    const auto config{ fuel::loadConfig(std::string(BASIC_TEST_JSON)) };

    EXPECT_FALSE(config.apiKey.has_value());
}

/// \brief Test that API key is still populated from the environment even when the rest of the JSON fails to parse.
TEST(ConfigLoaderApiKeyTest, ApiKeyIsPopulatedFromEnvironmentWhenJsonParsingFails)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Not unchecked
    constexpr auto TEST_API_KEY_VALUE{ "test-key-error-path" };
    EnvVarGuard guard{ "TANKER_KOENIG_API_KEY", TEST_API_KEY_VALUE };

    const auto config{ fuel::loadConfig(std::string(TEST_INVALID_JSON)) };

    ASSERT_TRUE(config.apiKey.has_value());
    EXPECT_EQ(*config.apiKey, TEST_API_KEY_VALUE);
    EXPECT_EQ(config.maxStations, fuel::FuelPulseConfig{}.maxStations);
    // NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Loading a config from a JSON document that does not adhere to the schema should not throw any exceptions and
///     default to the default \ref FuelPulseConfig.
TEST(ConfigLoaderValidationTest, TryLoadNonSchemaConformJson)
{
    fuel::FuelPulseConfig config;

    ASSERT_NO_THROW(config = fuel::loadConfig(std::string(NOT_SCHEMA_CONFORM_JSON)));
    EXPECT_EQ(config, fuel::FuelPulseConfig{});
}

/// \brief Test config loading from a file.
///
/// \author Felix Hommel
/// \date 8/1/26
class ConfigLoaderFromFileTest : public ::testing::Test
{
public:
    ConfigLoaderFromFileTest() = default;
    ~ConfigLoaderFromFileTest() override { std::filesystem::remove(m_filePath); }

    ConfigLoaderFromFileTest(const ConfigLoaderFromFileTest&) = delete;
    ConfigLoaderFromFileTest& operator=(const ConfigLoaderFromFileTest&) = delete;
    ConfigLoaderFromFileTest(ConfigLoaderFromFileTest&&) = delete;
    ConfigLoaderFromFileTest& operator=(ConfigLoaderFromFileTest&&) = delete;

    void SetUp() override
    {
        m_filePath = std::filesystem::path(TEST_RESOURCE_DIR)
                   / (std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".json");

        const auto j = nlohmann::json::parse(BASIC_TEST_JSON);
        std::ofstream ofs{ m_filePath };
        ofs << std::setw(4) << j.dump() << '\n'; // NOLINT(readability-magic-numbers): width of 4
    }

protected:
    std::filesystem::path m_filePath;
};

/// \brief Test Loading a config from a file that exists on the disk should return a valid \ref FuelPulseConfig.
TEST_F(ConfigLoaderFromFileTest, LoadingFromValidFileGivesValidConfig)
{
    const auto config{ fuel::loadConfig(m_filePath) };

    EXPECT_EQ(config, BASIC_TEST_JSON_REFERENCE);
}

/// \brief Test loading a config from a file that does not exist on the disk should not throw an exception but return a
///     default \ref FuelPulseConfig.
TEST_F(ConfigLoaderFromFileTest, LoadingFromNonExistingFileThrowsException)
{
    std::filesystem::remove(m_filePath);

    fuel::FuelPulseConfig config;

    ASSERT_NO_THROW(config = fuel::loadConfig(m_filePath));
    EXPECT_EQ(config, fuel::FuelPulseConfig{});
}

/// \brief Test that loading a config from a file that exists but contains invalid JSON content falls back to defaults.
TEST_F(ConfigLoaderFromFileTest, LoadingFileWithInvalidJsonContentFallsBackToDefault)
{
    TemporaryFile tempFile{ "invalidJsonFile.json" };

    {
        std::ofstream out{ tempFile.path() };
        out << "{ this is invalid json ";
    }

    fuel::FuelPulseConfig config;
    EXPECT_NO_THROW(config = fuel::loadConfig(tempFile.path()));
    EXPECT_EQ(config, fuel::FuelPulseConfig{});
}

} // namespace ful::testing
