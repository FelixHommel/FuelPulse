#ifndef FUL_SRC_LIB_CORE_COMMAND_REGISTRY_HPP
#define FUL_SRC_LIB_CORE_COMMAND_REGISTRY_HPP

#include "core/EventBus.hpp"

#include <spdlog/logger.h>

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ful
{

/// \brief The \ref CommandRegistry is a collection of known commands that have defined behavior when executed.
///
/// The registry can be configured by registering commands. Once commands have been registered the \ref CommandRegistry
/// is passed to the \ref CommandInterpreter where it serves as read-only storage to get the reaction to user commands.
///
/// \author Felix Hommel
/// \date 7/16/2026
class CommandRegistry
{
public:
    using CommandHandler = std::function<void(const std::vector<std::string>&, EventBus&)>;

    CommandRegistry();
    ~CommandRegistry() = default;

    CommandRegistry(const CommandRegistry&) = delete;
    CommandRegistry& operator=(const CommandRegistry&) = delete;
    CommandRegistry(CommandRegistry&&) = default;
    CommandRegistry& operator=(CommandRegistry&&) = default;

    /// \brief Register a new command.
    ///
    /// When a command with the same \p name has been registered before, the previous handler will be replaced with the
    /// new \p handler.
    ///
    /// \param name The name of the new command
    /// \param handler \ref CommandHandler specifying the reaction behavior to the command
    void registerCommand(std::string name, CommandHandler handler);
    /// \brief Get a command for a provided \p name.
    ///
    /// \param name The name of the command
    ///
    /// \returns const-pointer to the \ref CommandHandler, nullptr if no command with \p name was registered before.
    [[nodiscard]] std::optional<const CommandHandler*> find(std::string_view name) const;

private:
    std::unique_ptr<spdlog::logger> m_logger;
    std::unordered_map<std::string, CommandHandler> m_commands;
};

} // namespace ful

#endif // !FUL_SRC_LIB_CORE_COMMAND_REGISTRY_HPP
