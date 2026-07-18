#include "core/CommandInterpreter.hpp"

#include "core/CommandRegistry.hpp"
#include "core/EventBus.hpp"
#include "utility/StringOperations.hpp"

#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace
{

constexpr auto DELAY_20_MS{ std::chrono::milliseconds(20) };
constexpr auto DEFAULT_TIMEOUT{ std::chrono::milliseconds(2000) };

/// \brief Poll \p pred until it returns true or \p timeout elapses.
///
/// \param pred The predicate function
/// \param timeout (optional) Timeout after which to abort in case the predicate still has not evaluated to true
bool waitOn(std::function<bool()> pred, std::chrono::milliseconds timeout = ::DEFAULT_TIMEOUT)
{
    const auto deadline{ std::chrono::steady_clock::now() + timeout };

    while(std::chrono::steady_clock::now() < deadline)
    {
        if(pred())
            return true;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return pred();
}

} // namespace

namespace ful::testing
{

// NOLINTBEGIN(bugprone-unchecked-optional-access): Even though this is technically checked access, clang-tidy can't
//  prove that and for test purposes the checks need to happen in that sequence for ASSERT and EXPECT to check the right
//  conditions.

/// \brief Test the parsing of a simple command that has no parameters which should give the command name and en empty
///     argument list.
TEST(CommandInterpreterParserTest, ParseLineOfInputNoArgs)
{
    constexpr auto COMMAND_NAME{ "commandName" };

    const auto out{ CommandInterpreter::parse(COMMAND_NAME) };

    ASSERT_TRUE(out.has_value());

    const auto& val{ *out };

    EXPECT_EQ(COMMAND_NAME, val.name);
    EXPECT_TRUE(val.args.empty());
}

/// \brief Test that the parser can separate command name and provided arguments and then save the arguments appropriately.
TEST(CommandInterpreterParserTest, ParseLineOfInputWithArgs)
{
    const std::vector<std::string> COMMAND{ "commandName", "arg1", "arg2" };
    const auto parseOutput{
        CommandInterpreter::parse(COMMAND | std::views::join_with(' ') | std::ranges::to<std::string>())
    };

    ASSERT_TRUE(parseOutput.has_value());

    const auto& val{ *parseOutput };

    EXPECT_EQ(COMMAND[0], val.name);
    ASSERT_EQ(val.args.size(), COMMAND.size() - 1);

    for(std::size_t idx{ 1 }; const auto& arg : val.args)
        EXPECT_EQ(arg, COMMAND[idx++]);
}

/// \brief Test that the parser trims unnecessary whitespaces from user input.
TEST(CommandInterpreterParserTest, ParseLineOfInputWithExtraWhitespaces)
{
    const std::vector<std::string> COMMAND{ " commandName", "arg1", "arg2 " };
    const auto parseOutput{
        CommandInterpreter::parse(COMMAND | std::views::join_with(' ') | std::ranges::to<std::string>())
    };

    ASSERT_TRUE(parseOutput.has_value());

    const auto& val{ *parseOutput };

    EXPECT_EQ(trimView(COMMAND[0]), val.name);
    ASSERT_EQ(val.args.size(), COMMAND.size() - 1);

    for(std::size_t idx{ 1 }; const auto& arg : val.args)
        EXPECT_EQ(arg, trimView(COMMAND[idx++]));
}

/// \brief Test that the parser treats empty input correctly.
TEST(CommandInterpreterParserTest, ParseEmptyInput)
{
    const auto parseOutput{ CommandInterpreter::parse("") };

    EXPECT_FALSE(parseOutput.has_value());
}

/// \brief Test that the parser treats input that only consists of whitespaces correctly.
TEST(CommandInterpreterParserTest, ParseWithespaceOnlyInput)
{
    const auto parseOutput{ CommandInterpreter::parse("    ") };

    EXPECT_FALSE(parseOutput.has_value());
}

// NOLINTEND(bugprone-unchecked-optional-access)

class CommandInterpreterTest : public ::testing::Test
{
public:
    CommandInterpreterTest() = default;
    ~CommandInterpreterTest() override = default;

    CommandInterpreterTest(const CommandInterpreterTest&) = delete;
    CommandInterpreterTest& operator=(const CommandInterpreterTest&) = delete;
    CommandInterpreterTest(CommandInterpreterTest&&) = delete;
    CommandInterpreterTest& operator=(CommandInterpreterTest&&) = delete;

protected:
    CommandRegistry registry;
    EventBus bus;
    std::stringstream input;

    std::unique_ptr<CommandInterpreter> interpreter;
};

using CommandInterpreterRunTest = CommandInterpreterTest;

/// \brief Test that nothing is dispatched if the interpreter was not started beforehand.
TEST_F(CommandInterpreterRunTest, NoDispatchWithoutStart)
{
    std::atomic<bool> called{ false };

    registry.registerCommand("test", [&flag = called](std::vector<std::string>, EventBus&) {
        flag.store(true, std::memory_order_release);
    });

    input << "test\n";
    interpreter = std::make_unique<CommandInterpreter>(std::move(registry), bus, input);

    std::this_thread::sleep_for(::DELAY_20_MS);

    EXPECT_FALSE(called.load(std::memory_order_acquire));
}

/// \brief Test that a single command is dispatched on the background loop.
TEST_F(CommandInterpreterRunTest, SingleCommandIsDispatched)
{
    std::atomic<bool> called{ false };

    registry.registerCommand("test", [&flag = called](std::vector<std::string>, EventBus&) {
        flag.store(true, std::memory_order_release);
    });

    input << "test\n";
    interpreter = std::make_unique<CommandInterpreter>(std::move(registry), bus, input, true);

    EXPECT_TRUE(::waitOn([&called]() { return called.load(std::memory_order_acquire); }));

    interpreter->requestStop();
}

/// \brief Test that multiple commands are read and interpreted and finally dispatched on the background loop.
TEST_F(CommandInterpreterRunTest, MultipleCommandsAreDispatchedInOrder)
{
    std::vector<std::string> order;
    std::mutex orderMutex;

    registry.registerCommand("first", [&order, &orderMutex](std::vector<std::string>, EventBus&) {
        std::scoped_lock lock(orderMutex);
        order.emplace_back("first");
    });
    registry.registerCommand("second", [&order, &orderMutex](std::vector<std::string>, EventBus&) {
        std::scoped_lock lock(orderMutex);
        order.emplace_back("second");
    });

    input << "first\nsecond\n";
    interpreter = std::make_unique<CommandInterpreter>(std::move(registry), bus, input, true);

    EXPECT_TRUE(::waitOn([&order, &orderMutex]() {
        std::scoped_lock lock(orderMutex);
        return order.size() == 2;
    }));

    interpreter->requestStop();

    {
        std::scoped_lock lock(orderMutex);

        ASSERT_EQ(order.size(), 2);
        EXPECT_EQ(order[0], "first");
        EXPECT_EQ(order[1], "second");
    }
}

/// \brief Test that the interpreter continues to work normally when users enter an unknown command.
TEST_F(CommandInterpreterRunTest, UnknownCommandIsIgnoredWithoutCrashing)
{
    std::atomic<bool> knownCalled{ false };

    registry.registerCommand("known", [&flag = knownCalled](std::vector<std::string>, EventBus&) {
        flag.store(true, std::memory_order_release);
    });

    input << "unknown\nknown\n";
    interpreter = std::make_unique<CommandInterpreter>(std::move(registry), bus, input, true);

    EXPECT_TRUE(::waitOn([&knownCalled]() { return knownCalled.load(std::memory_order_acquire); }));

    interpreter->requestStop();
}

/// \brief Test exception safety of the command dispatcher. A \ref CommandHandler throwing a \ref std::exception should
///     not affect execution of the program.
TEST_F(CommandInterpreterRunTest, HandlerThrowingStdExceptionDoesNotStopProcessing)
{
    std::atomic<bool> secondCalled{ false };

    registry.registerCommand("throw", [](std::vector<std::string>, EventBus&) { throw std::runtime_error("failure"); });
    registry.registerCommand("after", [&flag = secondCalled](std::vector<std::string>, EventBus&) {
        flag.store(true, std::memory_order_release);
    });

    input << "throw\nafter\n";
    interpreter = std::make_unique<CommandInterpreter>(std::move(registry), bus, input, true);

    EXPECT_TRUE(::waitOn([&secondCalled]() { return secondCalled.load(std::memory_order_acquire); }));

    interpreter->requestStop();
}

/// \brief Test exception safety of the command dispatcher. A \ref CommandHandler throwing a non \ref std::exception
///     should not affect the execution of the program.
TEST_F(CommandInterpreterRunTest, HandlerThrowingNonStdExceptionDoesNotStopProcessing)
{
    std::atomic<bool> secondCalled{ false };

    registry.registerCommand("throw", [](std::vector<std::string>, EventBus&) { throw 1; });
    registry.registerCommand("after", [&flag = secondCalled](std::vector<std::string>, EventBus&) {
        flag.store(true, std::memory_order_release);
    });

    input << "throw\nafter\n";
    interpreter = std::make_unique<CommandInterpreter>(std::move(registry), bus, input, true);

    EXPECT_TRUE(::waitOn([&secondCalled]() { return secondCalled.load(std::memory_order_acquire); }));

    interpreter->requestStop();
}

/// \brief Test that the background loops exits within a reasonable time after it was requested to stop.
TEST_F(CommandInterpreterRunTest, LoopExitsOnStreamExhaustionWithoutExplicitStop)
{
    constexpr auto MAX_STOP_TIME{ std::chrono::milliseconds(1000) };

    std::atomic<bool> called{ false };

    registry.registerCommand("test", [&flag = called](std::vector<std::string>, EventBus&) {
        flag.store(true, std::memory_order_release);
        flag.notify_all();
    });

    input << "test\n";
    interpreter = std::make_unique<CommandInterpreter>(std::move(registry), bus, input, true);

    called.wait(false);

    const auto start{ std::chrono::steady_clock::now() };
    interpreter.reset();
    const auto elapsed{ std::chrono::steady_clock::now() - start };

    EXPECT_LT(elapsed, MAX_STOP_TIME);
}

using CommandInterpreterRunDeathTest = CommandInterpreterRunTest;

/// \brief Test that waiting for a thread to stop that was never started does not cause any critical errors or deadlocks
///     the program.
TEST_F(CommandInterpreterRunDeathTest, TryWaitingForNonRunningThreadToStop)
{
    interpreter = std::make_unique<CommandInterpreter>(std::move(registry), bus, input);

    EXPECT_NO_FATAL_FAILURE(interpreter->waitUntilStopped());
}

/// \brief Test that attempting to stop the interpreter before it was ever started does not cause any fatal errors.
TEST_F(CommandInterpreterRunDeathTest, StopBeforeStartDoesNotCrash)
{
    interpreter = std::make_unique<CommandInterpreter>(std::move(registry), bus, input);

    EXPECT_NO_FATAL_FAILURE(interpreter->requestStop());
}

/// \brief Test that a user calling \ref CommandInterpreter::start() for a second time does not cause any critical issues.
TEST_F(CommandInterpreterRunDeathTest, DoubleStartIsSafeAndDoesNotDeadlock)
{
    std::atomic<int> callCount{ 0 };

    registry.registerCommand("test", [&counter = callCount](std::vector<std::string>, EventBus&) {
        counter.fetch_add(1, std::memory_order_relaxed);
    });

    input << "test\n";
    interpreter = std::make_unique<CommandInterpreter>(std::move(registry), bus, input, true);

    EXPECT_TRUE(::waitOn([&callCount]() { return callCount.load(std::memory_order_relaxed) >= 1; }));

    EXPECT_NO_FATAL_FAILURE(interpreter->start());

    interpreter->requestStop();
}

} // namespace ful::testing
