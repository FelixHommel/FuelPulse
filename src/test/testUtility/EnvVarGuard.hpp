#ifndef FUL_SRC_TEST_TEST_UTILITY_ENV_VAR_GUARD_HPP
#define FUL_SRC_TEST_TEST_UTILITY_ENV_VAR_GUARD_HPP

#include <cstdlib>
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
    EnvVarGuard(std::string envVarName, const std::string& value) : m_envVarName{ std::move(envVarName) }
    {
        if(const char* existing{ std::getenv(m_envVarName.c_str()) })
            m_previousValue = std::make_optional(existing);

        writeVar(value.c_str());
    }

    ~EnvVarGuard()
    {
        if(m_previousValue.has_value())
            writeVar((*m_previousValue).c_str());
        else
            unsetVar();
    }

    EnvVarGuard(const EnvVarGuard&) = delete;
    EnvVarGuard& operator=(const EnvVarGuard&) = delete;
    EnvVarGuard(EnvVarGuard&&) = delete;
    EnvVarGuard& operator=(EnvVarGuard&&) = delete;

private:
    std::string m_envVarName;
    std::optional<std::string> m_previousValue;

    /// \brief Platform aware wrapper to write/replace an environment variable.
    ///
    /// \param value The new value of the environment variable
    ///
    /// \returns the return code of the platforms function
    int writeVar(const char* value)
    {
#ifdef _WIN32
        return _putenv_s(m_envVarName.c_str(), value);
#else
        return setenv(m_envVarName.c_str(), value, 1);
#endif
    }

    /// \brief Platform aware wrapper to remove an environment variable.
    ///
    /// \returns the return code of the platforms function
    int unsetVar()
    {
#ifdef _WIN32
        return _putenv_s(m_envVarName.c_str(), "");
#else
        return unsetenv(m_envVarName.c_str());
#endif
    }
};

} // namespace ful::testing

#endif // !FUL_SRC_TEST_TEST_UTILITY_ENV_VAR_GUARD_HPP
