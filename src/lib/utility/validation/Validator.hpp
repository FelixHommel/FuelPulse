#ifndef FUL_SRC_LIB_UTILITY_VALIDATION_VALIDATOR_HPP
#define FUL_SRC_LIB_UTILITY_VALIDATION_VALIDATOR_HPP

#include <nlohmann/json-schema.hpp>
#include <nlohmann/json_fwd.hpp>

#include <filesystem>
#include <string>
#include <vector>

namespace ful
{

/// \brief Describe an error that occurred while validating a JSON document against a schema.
///
/// \author Felix Hommel
/// \date 08/03/2026
struct ValidationError
{
    nlohmann::json::json_pointer path;
    std::string message;
};

/// \brief Simple wrapper class to load a JSON schema and help validate \ref nlohmann::json.
///
/// \author Felix Hommel
/// \date 8/1/2026
class Validator
{
public:
    using ValidationErrors = std::vector<ValidationError>;

    Validator(const std::string& jsonString);
    Validator(const std::filesystem::path& schemaPath);

    /// \brief Validate the \p doc against the loaded schema.
    ///
    /// \param doc The \ref nlohmann::json document that is validated
    ///
    /// \returns A \ref std::optional containing \ref valijson::ValidationResults if the validation failed, otherwise
    ///     contains a \ref std::nullopt
    [[nodiscard]] std::optional<ValidationErrors> validate(const nlohmann::json& doc) const;

private:
    /// \brief Handler callback for \ref nlohmann::json_schema::json_validator that saves the schema errors as \ref ValidationError
    ///     and collects them in a \ref std::vector
    ///
    /// \brief Felix Hommel
    /// \date 08/03/2026
    struct ValidationErrorHandler final : public nlohmann::json_schema::basic_error_handler
    {
        void error(
            const nlohmann::json::json_pointer& ptr, const nlohmann::json& j, const std::string& message
        ) override;


        ValidationErrors errors;
    };

    nlohmann::json_schema::json_validator m_validator;
    bool m_schemaLoaded{ false };
};

} // namespace ful

#endif // FUL_SRC_LIB_UTILITY_VALIDATION_VALIDATOR_HPP
