#include "Validator.hpp"

#include "utility/file/FileIO.hpp"

#include <nlohmann/json-schema.hpp>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <exception>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace ful
{

Validator::Validator(const std::string& jsonString)
{
    try
    {
        const auto schemaDoc = nlohmann::json::parse(jsonString);

        m_validator.set_root_schema(schemaDoc);
    }
    catch(const std::exception& e)
    {
        spdlog::error("Could not load the schema: {}", e.what());
        throw e;
    }
}

Validator::Validator(const std::filesystem::path& schemaPath) : Validator{ file::readFromFile(schemaPath) } {}

std::optional<Validator::ValidationErrors> Validator::validate(const nlohmann::json& doc) const
{
    ValidationErrorHandler handler;
    m_validator.validate(doc, handler);

    if(handler.errors.empty())
        return std::nullopt;

    std::ranges::sort(handler.errors, {}, [](const ValidationError& e) { return e.path.to_string(); });
    const auto [first, last]
        = std::ranges::unique(handler.errors, [](const auto& lhs, const auto& rhs) { return lhs.path == rhs.path; });
    handler.errors.erase(first, last);

    return std::make_optional(std::move(handler.errors));
}

void Validator::ValidationErrorHandler::error(
    const nlohmann::json::json_pointer& ptr, [[maybe_unused]] const nlohmann::json& j, const std::string& message
)
{
    errors.emplace_back(ptr, message);
}

} // namespace ful
