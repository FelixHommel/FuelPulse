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
#include <string_view>

namespace ful::env
{

std::optional<std::string> getVar(std::string_view varName)
{
#ifdef _WIN32
    // NOLINTBEGIN(cppcoreguidelines-no-malloc): Windows API mandates the memory allocation
    std::unique_ptr<char, decltype(&free)> buf{ nullptr, &free };

    char* raw{ nullptr };
    std::size_t size{ 0 };
    if(_dupenv_s(&raw, &size, varName.data()) != 0 || raw == nullptr)
        return std::nullopt;

    buf.reset(raw);

    return std::make_optional(buf.get());
    // NOLINTEND(cppcoreguidelines-no-malloc)
#else
    if(const char* value{ std::getenv(varName.data()) }; value != nullptr)
        return std::make_optional(value);

    return std::nullopt;
#endif
}

int writeVar(std::string_view varName, std::string_view value)
{
#ifdef _WIN32
    return _putenv_s(varName.data(), value.data());
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
