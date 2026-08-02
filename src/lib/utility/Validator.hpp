#ifndef FUL_SRC_LIB_UTILITY_VALIDATOR_HPP
#define FUL_SRC_LIB_UTILITY_VALIDATOR_HPP

#include <nlohmann/json_fwd.hpp>
#include <valijson/schema.hpp>
#include <valijson/validation_results.hpp>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

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
    Validator(const std::string& jsonString);
    Validator(const std::filesystem::path& schemaPath);

    /// \brief Validate the \p doc against the loaded schema.
    ///
    /// \param doc The \ref nlohmann::json document that is validated
    ///
    /// \returns A \ref std::optional containing \ref valijson::ValidationResults if the validation failed, otherwise
    ///     contains a \ref std::nullopt
    std::optional<valijson::ValidationResults> validate(const nlohmann::json& doc);

private:
    valijson::Schema schema;
};

} // namespace ful

#endif // FUL_SRC_LIB_UTILITY_VALIDATOR_HPP
