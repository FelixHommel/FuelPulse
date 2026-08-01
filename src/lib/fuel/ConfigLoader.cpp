#include "ConfigLoader.hpp"

#include "fuel/FuelPulseConfig.hpp"

#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

namespace ful::fuel
{

namespace
{

/// \brief Attempt to load the Tanker Koenig API Key from the environment.
///
/// \returns The API Key in string form wrapped in a \ref std::optional if possible, \ref std::nullopt otherwise
std::optional<std::string> loadApiKeyFromEnv()
{
    if(const char* pEnv = std::getenv("TANKER_KOENIG_API_KEY"))
        return std::make_optional(pEnv);

    spdlog::warn("Failed to find API Key in environment. Proceeding without being able to query.");
    return std::nullopt;
}

/// \brief Load the JSON part of the \ref FuelPulseConfig.
///
/// \throws \ref std::runtime_error If the file does not exist or it couldn't be opened
///
/// \param configPath a \ref std::filesystem::path to the location of the configuration file
std::string loadFileContent(const std::filesystem::path& configPath)
{
    if(!std::filesystem::exists(configPath))
        throw std::runtime_error(std::format("There is no file at the following location: {}", configPath.string()));

    std::ifstream ifs{ configPath };

    std::stringstream buffer{};
    buffer << ifs.rdbuf();

    return buffer.str();
}

} // namespace

FuelPulseConfig loadConfig(const std::filesystem::path& configPath)
{
    FuelPulseConfig config;
    try
    {
        config = loadConfig(loadFileContent(configPath));
    }
    catch(const std::exception& e)
    {
        spdlog::warn("Failed to load the configuration from the following location: {}", configPath.string());
    }

    return config;
}

FuelPulseConfig loadConfig(const std::string& json)
{
    FuelPulseConfig config;
    try
    {
        const auto j = nlohmann::json::parse(json);

        config = j.get<FuelPulseConfig>();
    }
    catch(const std::exception& e)
    {
        spdlog::warn("Failed to load the configuration for the following reason: {}", e.what());
        spdlog::info("Proceeding with default values in the config.");
        config = {};

        throw e;
    }

    config.apiKey = loadApiKeyFromEnv();

    return config;
}

} // namespace ful::fuel
