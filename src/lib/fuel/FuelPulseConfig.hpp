#ifndef FUL_SRC_LIB_FUEL_FUEL_PULSE_CONFIG_HPP
#define FUL_SRC_LIB_FUEL_FUEL_PULSE_CONFIG_HPP

#include <nlohmann/json.hpp>

#include <chrono>
#include <filesystem>
#include <optional>
#include <string>

namespace ful::fuel
{

namespace
{

constexpr auto DEFAULT_MAX_STATIONS{ 10u };
constexpr auto DEFAULT_POSTAL_CODE{ 70190u };
constexpr auto DEFAULT_SEARCH_RADIUS{ 10.f };
constexpr auto DEFAULT_COLLECTION_INTERVAL{ std::chrono::minutes(10) };

const auto DEFAULT_DATABASE_PATH{ FUL_ROOT / std::filesystem::path("resources/FuelPulse.db3") };
const auto DEFAULT_REPORT_DIR{ FUL_ROOT / std::filesystem::path("reports/") };

} // namespace

/// \brief Configuration for the FuelPulse runtime.
///
/// \author Felix Hommel
/// \date 07/30/26
struct FuelPulseConfig
{
    std::optional<std::string> apiKey{ std::nullopt };

    unsigned int maxStations{ DEFAULT_MAX_STATIONS };
    unsigned int postalCode{ DEFAULT_POSTAL_CODE };
    float searchRadius{ DEFAULT_SEARCH_RADIUS };
    std::chrono::minutes collectionInterval{ DEFAULT_COLLECTION_INTERVAL };
    std::filesystem::path databasePath{ DEFAULT_DATABASE_PATH };
    std::filesystem::path reportDir{ DEFAULT_REPORT_DIR };
};

/// \brief nlohmann::json utility to allow for json::get<FuelPulseConfig>.
void from_json(const nlohmann::json& j, FuelPulseConfig& config)
{
    j.at("max_stations").get_to<unsigned int>(config.maxStations);
    j.at("postal_code").get_to<unsigned int>(config.postalCode);
    j.at("search_radius").get_to<float>(config.searchRadius);
    j.at("database_path").get_to<std::filesystem::path>(config.databasePath);
    j.at("output_dir").get_to<std::filesystem::path>(config.reportDir);

    const auto collectionInterval{ j.at("collection_interval").get<unsigned int>() };
    config.collectionInterval = std::chrono::minutes(collectionInterval);
}

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_FUEL_PULSE_CONFIG_HPP
