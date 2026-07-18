#ifndef FUL_SRC_LIB_CORE_COMMAND_INTERPRETER_HPP
#define FUL_SRC_LIB_CORE_COMMAND_INTERPRETER_HPP

#include "core/CommandRegistry.hpp"
#include "core/EventBus.hpp"
#include "utility/Threading.hpp"

#include <spdlog/logger.h>

#include <iostream>
#include <istream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace ful
{

/// \brief Result of parsing a single line of user input.
///
/// \author Felix Hommel
/// \date 7/15/2026
struct ParsedCommand
{
    std::string name;
    std::vector<std::string> args;
};

/// \brief The \ref CommandInterpreter is responsible for parsing, interpreting, and relaying commands issued by the user.
///
/// \author Felix Hommel
/// \date 7/17/2026
class CommandInterpreter
{
public:
    /// \brief Create a new \ref CommandInterpreter.
    ///
    /// \param registry The \ref CommandRegistry containing the available commands
    /// \param bus The \ref EventBus with which the interpreter can communicate with the rest of the system
    /// \param input The input source where the interpreter is reading from
    /// \param instantStart Whether or not the interpreter should start running instantly
    explicit CommandInterpreter(
        CommandRegistry registry, EventBus& bus, std::istream& input = std::cin, bool instantStart = false
    );
    ~CommandInterpreter() = default;

    CommandInterpreter(const CommandInterpreter&) = delete;
    CommandInterpreter& operator=(const CommandInterpreter&) = delete;
    CommandInterpreter(CommandInterpreter&&) = delete;
    CommandInterpreter& operator=(CommandInterpreter&&) = delete;

    /// \brief Starts the background read loop.
    ///
    /// Does nothing if the thread is already running.
    void start();
    /// \brief Request the background read loop to stop.
    ///
    /// This does not guarantee immediate exit. I.e., std::getline blocks on the input stream it is reading from.
    ///
    /// \param waitForStop (optional) If true, equivavlent to calling \ref CommandInterpreter::waitUntilStopped() right
    ///     after
    void requestStop(bool waitForStop = false);
    /// \brief Wait until the background thread actually finishes.
    ///
    /// This should only be called after \ref CommandInterpreter::stop() was called.
    void waitUntilStopped();

    /// \brief Parse one line of input into a command + args.
    ///
    /// \param line The line of input
    ///
    /// \returns \ref std::optional of a \ref ParsedCommand if the line could be parsed properly
    [[nodiscard]] static std::optional<ParsedCommand> parse(std::string line);

private:
    std::unique_ptr<spdlog::logger> m_logger;

    CommandRegistry m_registry;
    EventBus& m_bus;       // NOLINT
    std::istream& m_input; // NOLINT

    // NOTE: Needs to be last member in class, so that it gets constructed last and destroyed first (depends on the
    //  other class members)
    threading::thread_t m_thread; ///< The thread on which the interpreter is executed

    void run(threading::stop_token_t token) const;
    void dispatch(const ParsedCommand& command) const;
};

} // namespace ful

#endif // !FUL_SRC_LIB_CORE_COMMAND_INTERPRETER_HPP
