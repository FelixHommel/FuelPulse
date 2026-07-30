#ifndef FUL_SRC_LIB_FUEL_CONFIG_LOADER_HPP
#define FUL_SRC_LIB_FUEL_CONFIG_LOADER_HPP

#include "fuel/FuelPulseConfig.hpp"

#include <filesystem>

namespace ful::fuel
{

/// \brief Load a configuration file from the disk.
///
/// \param configPath The \ref std::filesystem::path to the location of the configuration file on the disk
///
/// \throws std::exception If loading the config from disk failed for some reason
///
/// \returns \ref FuelPulseConfig with the loaded values
FuelPulseConfig loadConfig(const std::filesystem::path& configPath);
FuelPulseConfig loadConfig(const std::string& json);

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_CONFIG_LOADER_HPP
