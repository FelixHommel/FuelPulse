#ifndef FUL_SRC_TEST_TEST_UTILITY_ENV_VAR_GUARD_HPP
#define FUL_SRC_TEST_TEST_UTILITY_ENV_VAR_GUARD_HPP

#include "utility/env/EnvironmentVariableHelper.hpp"

#include <optional>
#include <string>
#include <utility>

namespace ful::testing
{

/// \brief RAII-style wrapper for an environment variable that preserves the state of the environment variable at
///     construction time and restores the state at destruction time.
///
/// \author Felix Hommel
/// \date 07/31/26
class EnvVarGuard
{
public:
    EnvVarGuard(std::string envVarName, const std::string& value)
        : m_envVarName{ std::move(envVarName) }, m_previousValue{ env::getVar(m_envVarName) }
    {
        env::writeVar(m_envVarName, value);
    }

    ~EnvVarGuard()
    {
        if(m_previousValue.has_value())
            env::writeVar(m_envVarName, *m_previousValue);
        else
            env::unsetVar(m_envVarName);
    }

    EnvVarGuard(const EnvVarGuard&) = delete;
    EnvVarGuard& operator=(const EnvVarGuard&) = delete;
    EnvVarGuard(EnvVarGuard&&) = delete;
    EnvVarGuard& operator=(EnvVarGuard&&) = delete;

private:
    std::string m_envVarName;
    std::optional<std::string> m_previousValue;
};

} // namespace ful::testing

#endif // !FUL_SRC_TEST_TEST_UTILITY_ENV_VAR_GUARD_HPP
