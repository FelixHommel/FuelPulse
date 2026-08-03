#ifndef FUL_SRC_LIB_UTILITY_VALIDATION_VALIDATOR_HPP
#define FUL_SRC_LIB_UTILITY_VALIDATION_VALIDATOR_HPP

#include <nlohmann/json_fwd.hpp>
#include <valijson/schema.hpp>
#include <valijson/validation_results.hpp>

#include <filesystem>
#include <string>

namespace ful
{

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

#endif // FUL_SRC_LIB_UTILITY_VALIDATION_VALIDATOR_HPP
