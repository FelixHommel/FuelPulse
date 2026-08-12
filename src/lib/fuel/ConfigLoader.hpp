#ifndef FUL_SRC_LIB_FUEL_CONFIG_LOADER_HPP
#define FUL_SRC_LIB_FUEL_CONFIG_LOADER_HPP

#include "fuel/FuelPulseConfig.hpp"

#include <nlohmann/json_fwd.hpp>

#include <filesystem>

namespace ful::fuel
{

namespace detail
{

/// \brief Validate \p doc against the schema file for \ref FuelPulseConfig.
///
/// \param doc The JSON document that is validated
///
/// \throws A \ref ConfigSchemaValidationException if the validation fails
void validateConfigDoc(const nlohmann::json& doc);

} // namespace detail

/// \brief Load a configuration file from the disk.
///
/// \param configPath The \ref std::filesystem::path to the location of the configuration file on the disk
///
/// \returns \ref FuelPulseConfig with the loaded values
[[nodiscard]] FuelPulseConfig loadConfig(const std::filesystem::path& configPath);
/// \brief Load a configuration file from a JSON string.
///
/// \param json The \ref std::string containing the JSON
///
/// \returns \ref FuelPulseConfig with the loaded values
[[nodiscard]] FuelPulseConfig loadConfig(const std::string& json);

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_CONFIG_LOADER_HPP
