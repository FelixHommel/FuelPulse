#ifndef FUL_SRC_LIB_UTILITY_ENV_ENVIRONMENT_VARIABLE_HELPER_HPP
#define FUL_SRC_LIB_UTILITY_ENV_ENVIRONMENT_VARIABLE_HELPER_HPP

#include <optional>
#include <string>

namespace ful::env
{

/// \brief Retrieve the value of an environment variable.
///
/// \param varName The name of the environment variable
///
/// \returns A \ref std::optional containing the value if one exists, \ref std::nullopt if the environment variable does
///     not have a value
[[nodiscard]] std::optional<std::string> getVar(const std::string& varName);

/// \brief Platform aware wrapper to write/replace an environment variable.
///
/// \param varName The name of the environment variable
/// \param value The new value of the environment variable
///
/// \returns the return code of the platforms function
int writeVar(const std::string& varName, const std::string& value);

/// \brief Platform aware wrapper to remove an environment variable.
///
/// \param varName The name of the environment variable
///
/// \returns the return code of the platforms function
int unsetVar(const std::string& varName);

} // namespace ful::env

#endif // !FUL_SRC_LIB_UTILITY_ENV_ENVIRONMENT_VARIABLE_HELPER_HPP
