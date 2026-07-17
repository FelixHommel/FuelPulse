#include "core/CommandRegistry.hpp"
#include "core/EventBus.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace ful::testing
{

/// \brief Test the functionality of the \ref CommandRegistry.
///
/// \author Felix Hommel
/// \date 7/16/2026
class CommandRegistryTest : public ::testing::Test
{
public:
    CommandRegistryTest() = default;
    ~CommandRegistryTest() override = default;

    CommandRegistryTest(const CommandRegistryTest&) = delete;
    CommandRegistryTest& operator=(const CommandRegistryTest&) = delete;
    CommandRegistryTest(CommandRegistryTest&&) = delete;
    CommandRegistryTest& operator=(CommandRegistryTest&&) = delete;

protected:
    CommandRegistry registry;
    EventBus bus;
};

// NOLINTBEGIN(bugprone-unchecked-optional-access): Even though this is technically checked access, clang-tidy can't
//  prove that and for test purposes the checks need to happen in that sequence for ASSERT and EXPECT to check the right
//  conditions.

/// \brief Registering a new command means that it should be possible to retrieve the command from the \ref CommandRegistry
TEST_F(CommandRegistryTest, RegisterNewCommand)
{
    constexpr auto COMMAND_NAME{ "test" };

    bool called{ false };

    registry.registerCommand(COMMAND_NAME, [&calledFlag = called](std::vector<std::string>, EventBus&) {
        calledFlag = true;
    });

    const auto fn{ registry.find(COMMAND_NAME) };

    ASSERT_TRUE(fn.has_value());

    (**fn)({}, bus);

    EXPECT_TRUE(called);
}

/// \brief Re-registering a command with a duplicate name should not cause any issues since it is simply replacing the
///     \ref CommandHandler.
TEST_F(CommandRegistryTest, ReRegisterSameCommand)
{
    constexpr auto COMMAND_NAME{ "test" };

    bool called{ false };

    registry.registerCommand(COMMAND_NAME, [&calledFlag = called](std::vector<std::string>, EventBus&) {
        calledFlag = true;
    });
    registry.registerCommand(COMMAND_NAME, [&calledFlag = called](std::vector<std::string>, EventBus&) {
        calledFlag = true;
    });

    const auto fn{ registry.find(COMMAND_NAME) };

    ASSERT_TRUE(fn.has_value());

    (**fn)({}, bus);

    EXPECT_TRUE(called);
}

/// \brief When a command is attempted to be found in the \ref CommandRegistry that was never registered, the registry
///     should return a nullptr.
TEST_F(CommandRegistryTest, FindCommandThatWasNotRegistered)
{
    constexpr auto COMMAND_NAME{ "test" };

    EXPECT_FALSE(registry.find(COMMAND_NAME).has_value());
}

// NOLINTEND(bugprone-unchecked-optional-access)

} // namespace ful::testing
