#include "core/CommandInterpreter.hpp"

#include "core/CommandRegistry.hpp"
#include "core/EventBus.hpp"
#include "utility/StringOperations.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <cstddef>
#include <memory>
#include <ranges>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace ful
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

} // namespace ful
