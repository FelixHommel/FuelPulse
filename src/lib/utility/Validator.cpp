#include "Validator.hpp"

#include "utility/FileIO.hpp"

#include <nlohmann/json_fwd.hpp>
#include <valijson/adapters/nlohmann_json_adapter.hpp>
#include <valijson/schema_parser.hpp>
#include <valijson/validation_results.hpp>
#include <valijson/validator.hpp>

#include <filesystem>
#include <optional>
#include <string>
#include <utility>

namespace ful
{

Validator::Validator(const std::string& jsonString)
{
    const auto schemaDoc = nlohmann::json::parse(jsonString);

    valijson::SchemaParser parser;
    valijson::adapters::NlohmannJsonAdapter schemaAdapter{ schemaDoc };
    parser.populateSchema(schemaAdapter, schema);
}

Validator::Validator(const std::filesystem::path& schemaPath) : Validator{ file::readFromFile(schemaPath) } {}

std::optional<valijson::ValidationResults> Validator::validate(const nlohmann::json& doc)
{
    valijson::adapters::NlohmannJsonAdapter docAdapter{ doc };
    valijson::ValidationResults results;
    const bool success{ valijson::Validator().validate(schema, docAdapter, &results) };

    return (success ? std::nullopt : std::make_optional(std::move(results)));
}

} // namespace ful
