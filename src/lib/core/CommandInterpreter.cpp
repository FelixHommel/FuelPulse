#include "CommandInterpreter.hpp"

#include "core/CommandRegistry.hpp"
#include "core/EventBus.hpp"
#include "utility/LoggerFactory.hpp"
#include "utility/StringOperations.hpp"
#include "utility/Threading.hpp"

#include <exception>
#include <functional>
#include <istream>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ful
{

CommandInterpreter::CommandInterpreter(CommandRegistry registry, EventBus& bus, std::istream& input, bool instantStart)
    : m_registry{ std::move(registry) }, m_bus{ bus }, m_input{ input }
{
    LoggerFactory factory{};
    m_logger = factory.create("CommandInterpreter", LoggerProfile::Console);

    if(instantStart)
        start();
}

void CommandInterpreter::start()
{
    if(m_thread.joinable())
    {
        m_logger->warn("start() called even though interpreter is already running");
        return;
    }

    // NOTE: From: https://stackoverflow.com/a/65701431
    m_thread = threading::thread_t(std::bind_front(&CommandInterpreter::run, this));
    m_logger->info("Starting background thread");
}

void CommandInterpreter::requestStop(bool waitForStop)
{
    m_thread.request_stop();
    m_logger->info("Requested to stop background thread");

    if(waitForStop)
        waitUntilStopped();
}

void CommandInterpreter::waitUntilStopped()
{
    if(!m_thread.joinable())
        return;

    m_thread.join();
}

std::optional<ParsedCommand> CommandInterpreter::parse(std::string line)
{
    std::stringstream ss(std::move(line));
    std::string s;
    std::vector<std::string> splitCommand;

    while(std::getline(ss, s, ' '))
    {
        trim(s);
        splitCommand.push_back(s);
    }

    splitCommand = splitCommand | std::views::filter([](const std::string& str) { return !str.empty(); })
                 | std::ranges::to<std::vector>();

    if(splitCommand.empty())
        return std::nullopt;

    auto commandName{ *splitCommand.cbegin() };
    splitCommand.erase(splitCommand.cbegin());

    return std::make_optional<ParsedCommand>(std::move(commandName), std::move(splitCommand));
}

/// \brief Input source read loop.
///
/// \param token The \ref threading::stop_token_t to communicate stopping with the executing thread
void CommandInterpreter::run(threading::stop_token_t token) const
{
    std::string line{};
    while(!token.stop_requested())
    {
        if(!std::getline(m_input, line))
            break;

        if(const auto parsedCommand{ CommandInterpreter::parse(line) }; parsedCommand.has_value())
            dispatch(*parsedCommand);
    }
}

/// \brief Look up the command in the \ref CommandRegistry and invoke the handler.
///
/// Internally ensures that invalid or malformed commands do not crash or otherwise disturb the application state.
///
/// \param command The command that was parsed
void CommandInterpreter::dispatch(const ParsedCommand& command) const
{
    if(const auto entry{ m_registry.find(command.name) }; entry.has_value())
    {
        try
        {
            (**entry)(command.args, m_bus);
        }
        catch(const std::exception& e)
        {
            m_logger->error("CommandHandler threw a std::exception: {}", e.what());
        }
        catch(...)
        {
            m_logger->error("CommandHandler threw an unknown exception");
        }
    }
}

} // namespace ful
