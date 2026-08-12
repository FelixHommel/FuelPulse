#include "ConfigLoader.hpp"

#include "fuel/FuelPulseConfig.hpp"
#include "utility/env/EnvironmentVariableHelper.hpp"
#include "utility/exception/ConfigValidationException.hpp"
#include "utility/exception/FileIOException.hpp"
#include "utility/file/FileIO.hpp"
#include "utility/validation/Validator.hpp"

#include <nlohmann/detail/exceptions.hpp>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>

#include <filesystem>
#include <optional>
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

namespace detail
{

void validateConfigDoc(const nlohmann::json& doc)
{
    static Validator validator{ FUL_ROOT / std::filesystem::path("resources/ConfigSchema.json") };

    if(auto results{ validator.validate(doc) }; results.has_value())
        throw ConfigValidationException::create(std::move(*results));
}

} // namespace detail

FuelPulseConfig loadConfig(const std::filesystem::path& configPath)
{
    std::optional<std::string> doc;

    try
    {
        doc = std::make_optional(file::readFromFile(configPath));
    }
    catch(const FileIOException& e)
    {
        spdlog::warn(e.what());
        spdlog::warn("Proceeding with default values in the config.");
        doc = std::nullopt;
    }

    return doc.has_value() ? loadConfig(*doc) : FuelPulseConfig{};
}

FuelPulseConfig loadConfig(const std::string& json)
{
    FuelPulseConfig config;
    try
    {
        const auto configJson = nlohmann::json::parse(json);

        detail::validateConfigDoc(configJson);

        config = configJson.get<FuelPulseConfig>();
    }
    catch(const nlohmann::detail::parse_error& e)
    {
        spdlog::warn("Failed to load the configuration file for the following reason: {}", e.what());
        spdlog::warn("Proceeding with default values in the config.");
        config = {};
    }
    catch(const ConfigValidationException& e)
    {
        spdlog::warn("{}", e.what());
        spdlog::info("Proceeding with default values in the config.");
        config = {};
    }

    config.apiKey = loadApiKeyFromEnv();

    return config;
}

} // namespace ful::fuel
