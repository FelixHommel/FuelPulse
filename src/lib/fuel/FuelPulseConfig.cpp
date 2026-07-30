#include "FuelPulseConfig.hpp"

#include <nlohmann/json_fwd.hpp>

#include <chrono>
#include <filesystem>

namespace ful::fuel
{

void from_json(const nlohmann::json& j, FuelPulseConfig& config)
{
    j.at("max_stations").get_to<unsigned int>(config.maxStations);
    j.at("postal_code").get_to<unsigned int>(config.postalCode);
    j.at("search_radius").get_to<float>(config.searchRadius);
    j.at("database_path").get_to<std::filesystem::path>(config.databasePath);
    j.at("report_dir").get_to<std::filesystem::path>(config.reportDir);

    const auto collectionInterval{ j.at("collection_interval").get<unsigned int>() };
    config.collectionInterval = std::chrono::minutes(collectionInterval);
}

} // namespace ful::fuel
