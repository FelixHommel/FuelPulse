#ifndef FUL_SRC_LIB_UTILITY_ENVIRONMENT_VARIABLE_HELPER_HPP
#define FUL_SRC_LIB_UTILITY_ENVIRONMENT_VARIABLE_HELPER_HPP

#include <optional>
#include <string>
#include <string_view>

namespace ful::env
{

/// \brief Retrieve the value of an environment variable.
///
/// \param varName The name of the environment variable
///
/// \returns A \ref std::optional containing the value if one exists, \ref std::nullopt if the environment variable does
///     not have a value
[[nodiscard]] std::optional<std::string> getVar(std::string_view varName);

/// \brief Platform aware wrapper to write/replace an environment variable.
///
/// \param varName The name of the environment variable
/// \param value The new value of the environment variable
///
/// \returns the return code of the platforms function
int writeVar(std::string_view varName, std::string_view value);

/// \brief Platform aware wrapper to remove an environment variable.
///
/// \param varName The name of the environment variable
///
/// \returns the return code of the platforms function
int unsetVar(std::string_view varName);

} // namespace ful::env

#endif // !FUL_SRC_LIB_UTILITY_ENVIRONMENT_VARIABLE_HELPER_HPP
