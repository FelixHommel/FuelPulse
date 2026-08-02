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
/// \date 07/30/2026
struct FuelPulseConfig
{
    std::optional<std::string> apiKey{ std::nullopt };

    unsigned int maxStations{ DEFAULT_MAX_STATIONS };
    unsigned int postalCode{ DEFAULT_POSTAL_CODE };
    float searchRadius{ DEFAULT_SEARCH_RADIUS };
    std::chrono::minutes collectionInterval{ DEFAULT_COLLECTION_INTERVAL };
    std::filesystem::path databasePath{ DEFAULT_DATABASE_PATH };
    std::filesystem::path reportDir{ DEFAULT_REPORT_DIR };

    constexpr auto operator<=>(const FuelPulseConfig&) const = default;
};

/// \brief nlohmann::json utility to allow for json::get<FuelPulseConfig>.
void from_json(const nlohmann::json& j, FuelPulseConfig& config);

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_FUEL_PULSE_CONFIG_HPP
