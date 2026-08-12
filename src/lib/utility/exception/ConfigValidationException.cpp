#include "ConfigValidationException.hpp"

#include "utility/exception/Exception.hpp"
#include "utility/validation/Validator.hpp"

#include <format>
#include <source_location>
#include <string>
#include <utility>

namespace ful
{

ConfigValidationException::ConfigValidationException(Exception exception, Validator::ValidationErrors errors)
    : Exception{ std::move(exception) }, m_errors{ std::move(errors) }
{}

ConfigValidationException ConfigValidationException::create(
    Validator::ValidationErrors errors, std::source_location loc
)
{
    std::string message{ "Failed to validate config with the following errors:\n" };
    for(const auto& e : errors)
        message.append(std::format("\t{}: {}\n", e.path.to_string(), e.message));

    return ConfigValidationException{
        Exception{ std::move(message), loc },
        std::move(errors)
    };
}

} // namespace ful
