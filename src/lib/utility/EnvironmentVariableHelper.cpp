#include "EnvironmentVariableHelper.hpp"

#ifdef _WIN32
#    include <cstddef>
#    include <utility>
#else
#    include <stdlib.h> // NOLINT(modernize-deprecated-headers): For some reason setenv(), unsetenv need the C header
#endif
#include <cstdlib>
#include <optional>
#include <string>
#include <string_view>

namespace ful::env
{

std::optional<std::string> getVar(std::string_view varName)
{
#ifdef _WIN32
    // NOLINTBEGIN(cppcoreguidelines-no-malloc): Windows API mandates the memory allocation
    char* buf{ nullptr };
    std::size_t size{ 0 };
    if(_dupenv_s(&buf, &size, varName.data()) == 0 || buf != nullptr)
        return std::nullopt;

    std::string result(buf, size);
    free(buf);

    return std::make_optional(std::move(result));
    // NOLINTEND(cppcoreguidelines-no-malloc)
#else
    const char* value{ std::getenv(varName.data()) };

    return value == nullptr ? std::nullopt : std::make_optional(value);
#endif
}

int writeVar(std::string_view varName, std::string_view value)
{
#ifdef _WIN32
    return _putenv_s(varName.data(), value);
#else
    return setenv(varName.data(), value.data(), 1);
#endif
}

int unsetVar(std::string_view varName)
{
#ifdef _WIN32
    return _putenv_s(varName.data(), "");
#else
    return unsetenv(varName.data());
#endif
}

} // namespace ful::env
