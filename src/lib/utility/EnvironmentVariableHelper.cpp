#include "EnvironmentVariableHelper.hpp"

#ifdef _WIN32
#    include <cstddef>
#    include <memory>
#else
#    include <stdlib.h> // NOLINT(modernize-deprecated-headers): For some reason setenv(), unsetenv need the C header
#endif
#include <cstdlib>
#include <optional>
#include <string>

namespace ful::env
{

std::optional<std::string> getVar(const std::string& varName)
{
#ifdef _WIN32
    // NOLINTBEGIN(cppcoreguidelines-no-malloc): Windows API mandates the memory allocation
    std::unique_ptr<char, decltype(&free)> buf{ nullptr, &free };

    char* raw{ nullptr };
    std::size_t size{ 0 };
    if(_dupenv_s(&raw, &size, varName.c_str()) != 0 || raw == nullptr)
        return std::nullopt;

    buf.reset(raw);

    return std::make_optional(buf.get());
    // NOLINTEND(cppcoreguidelines-no-malloc)
#else
    if(const char* value{ std::getenv(varName.c_str()) }; value != nullptr)
        return std::make_optional(value);

    return std::nullopt;
#endif
}

int writeVar(const std::string& varName, const std::string& value)
{
#ifdef _WIN32
    return _putenv_s(varName.c_str(), value.c_str());
#else
    return setenv(varName.c_str(), value.c_str(), 1);
#endif
}

int unsetVar(const std::string& varName)
{
#ifdef _WIN32
    return _putenv_s(varName.c_str(), "");
#else
    return unsetenv(varName.c_str());
#endif
}

} // namespace ful::env
