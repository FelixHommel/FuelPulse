#include "ConfigLoader.hpp"

#include "fuel/FuelPulseConfig.hpp"
#include "utility/env/EnvironmentVariableHelper.hpp"
#include "utility/file/FileIO.hpp"
#include "utility/validation/Validator.hpp"

#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <valijson/validation_results.hpp>

#include <exception>
#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>

namespace ful::fuel
{

namespace
{

/// \brief Attempt to load the Tanker Koenig API Key from the environment.
///
/// \returns The API Key in string form wrapped in a \ref std::optional if possible, \ref std::nullopt otherwise
std::optional<std::string> loadApiKeyFromEnv()
{
    auto key{ env::getVar("TANKER_KOENIG_API_KEY") };

    if(!key.has_value())
        spdlog::warn("Failed to find API Key in environment. Proceeding without being able to query.");

    return std::move(key);
}

} // namespace

FuelPulseConfig loadConfig(const std::filesystem::path& configPath)
{
    FuelPulseConfig config;
    try
    {
        config = loadConfig(file::readFromFile(configPath));
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
        const auto configJson = nlohmann::json::parse(json);

        Validator validator{ FUL_ROOT / std::filesystem::path("resources/ConfigSchema.json") };
        if(auto results{ validator.validate(configJson) }; results.has_value())
        {
            auto& errors{ *results };
            spdlog::error("Config validation failed.");

            valijson::ValidationResults::Error error;
            unsigned int errorNum{ 1 };
            while(errors.popError(error))
                spdlog::error("Error #{}\n\t@ {}\n\t- {}", errorNum++, error.context[0], error.description);

            throw std::runtime_error("");
        }

        config = configJson.get<FuelPulseConfig>();
    }
    catch(const std::exception& e)
    {
        spdlog::warn("Failed to load the configuration for the following reason: {}", e.what());
        spdlog::info("Proceeding with default values in the config.");
        config = {};
    }

    config.apiKey = loadApiKeyFromEnv();

    return config;
}

} // namespace ful::fuel
