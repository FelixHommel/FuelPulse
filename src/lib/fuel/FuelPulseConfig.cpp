#include "FuelPulseConfig.hpp"

#include <nlohmann/json_fwd.hpp>

#include <chrono>
#include <filesystem>

namespace ful::fuel
{

void from_json(const nlohmann::json& j, FuelPulseConfig& config)
{
    if(j.contains("max_stations"))
        j.at("max_stations").get_to<unsigned int>(config.maxStations);
    if(j.contains("postal_code"))
        j.at("postal_code").get_to<unsigned int>(config.postalCode);
    if(j.contains("search_radius"))
        j.at("search_radius").get_to<float>(config.searchRadius);
    if(j.contains("database_path"))
        j.at("database_path").get_to<std::filesystem::path>(config.databasePath);
    if(j.contains("report_dir"))
        j.at("report_dir").get_to<std::filesystem::path>(config.reportDir);
    if(j.contains("collection_interval"))
        config.collectionInterval = std::chrono::minutes(j.at("collection_interval").get<unsigned int>());
}

} // namespace ful::fuel
