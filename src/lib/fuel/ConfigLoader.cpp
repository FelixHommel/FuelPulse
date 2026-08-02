#include "ConfigLoader.hpp"

#include "fuel/FuelPulseConfig.hpp"

#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>

#include <cstdlib>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <optional>
#include <sstream>
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

/// \brief Simple wrapper class to load a JSON schema and help validate \ref nlohmann::json.
///
/// \author Felix Hommel
/// \date 8/1/2026
class Validator
{
public:
    Validator(std::filesystem::path schemaPath)
    {
        if(!std::filesystem::exists(schemaPath))
            throw std::runtime_error(
                std::format("There is no schema file at the following location: {}", schemaPath.string())
            );

        std::ifstream schemaFile{ schemaPath };
        const auto schemaDoc = nlohmann::json::parse(schemaFile);

        valijson::SchemaParser parser;
        valijson::adapters::NlohmannJsonAdapter schemaAdapter{ schemaDoc };
        parser.populateSchema(schemaAdapter, schema);
    }

    /// \brief Validate the \p doc against the loaded schema.
    ///
    /// \param doc The \ref nlohmann::json document that is validated
    ///
    /// \returns A \ref std::optional containing \ref valijson::ValidationResults if the validation failed, otherwise
    ///     contains a \ref std::nullopt
    std::optional<valijson::ValidationResults> validate(const nlohmann::json& doc)
    {
        valijson::adapters::NlohmannJsonAdapter docAdapter{ doc };
        valijson::ValidationResults results;
        const bool success{ valijson::Validator().validate(schema, docAdapter, &results) };

        return (success ? std::nullopt : std::make_optional(std::move(results)));
    }

private:
    valijson::Schema schema;
};

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
