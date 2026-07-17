#include "CommandRegistry.hpp"

#include "utility/LoggerFactory.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <utility>

namespace ful
{

CommandRegistry::CommandRegistry()
{
    LoggerFactory factory{};
    m_logger = factory.create("CommandInterpreter", LoggerProfile::Console);
}

void CommandRegistry::registerCommand(std::string name, CommandHandler handler)
{
    if(const auto [it, result] = m_commands.insert_or_assign(std::move(name), std::move(handler)); !result)
        m_logger->warn(
            "Registered command with name '{}' replaced a previously registered command with the same name", it->first
        );
}

std::optional<const CommandRegistry::CommandHandler*> CommandRegistry::find(std::string_view name) const
{
    if(const auto it{ m_commands.find(std::string(name)) }; it != m_commands.cend())
        return std::make_optional(&it->second);

    m_logger->error("Could not find a command registered under the name '{}'", name);
    return std::nullopt;
}

} // namespace ful
