#ifndef FUL_SRC_LIB_UTILITY_VALIDATOR_HPP
#define FUL_SRC_LIB_UTILITY_VALIDATOR_HPP

#include <nlohmann/json.hpp>
#include <sstream>
#include <string>
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>

#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <utility>

namespace ful
{

namespace
{

std::string readSchemaFile(const std::filesystem::path& filePath)
{
    if(!std::filesystem::exists(filePath))
        throw std::runtime_error(
            std::format("There is no schema file at the following location: {}", filePath.string())
        );

    std::ifstream schemaFile{ filePath };
    std::stringstream fileContent;
    fileContent << schemaFile.rdbuf();

    return fileContent.str();
}

} // namespace

/// \brief Simple wrapper class to load a JSON schema and help validate \ref nlohmann::json.
///
/// \author Felix Hommel
/// \date 8/1/2026
class Validator
{
public:
    Validator(const std::string& jsonString)
    {
        const auto schemaDoc = nlohmann::json::parse(jsonString);

        valijson::SchemaParser parser;
        valijson::adapters::NlohmannJsonAdapter schemaAdapter{ schemaDoc };
        parser.populateSchema(schemaAdapter, schema);
    }
    Validator(const std::filesystem::path& schemaPath) : Validator{ readSchemaFile(schemaPath) } {}

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

} // namespace ful

#endif // FUL_SRC_LIB_UTILITY_VALIDATOR_HPP
