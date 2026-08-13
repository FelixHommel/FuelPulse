#include "utility/logging/LoggerFactory.hpp"

#include <gtest/gtest.h>
#include <spdlog/common.h>

namespace ful::testing
{

/// \brief Test the \ref LoggerFactory sink wiring and logger configuration.
///
/// \author Felix Hommel
/// \date 8/12/2026
class LoggerFactoryTest : public ::testing::Test
{
public:
    LoggerFactoryTest() = default;
    ~LoggerFactoryTest() override = default;

    LoggerFactoryTest(const LoggerFactoryTest&) = delete;
    LoggerFactoryTest& operator=(const LoggerFactoryTest&) = delete;
    LoggerFactoryTest(LoggerFactoryTest&&) = delete;
    LoggerFactoryTest& operator=(LoggerFactoryTest&&) = delete;

protected:
    LoggerFactory m_factory;
};

/// \brief Test that a logger created with the console profile has exactly one sink.
TEST_F(LoggerFactoryTest, ConsoleProfileHasSink)
{
    EXPECT_EQ(m_factory.create("test-console", LoggerProfile::Console)->sinks().size(), 1);
}

/// \brief Test that a logger created with the file profile has exactly one sink.
TEST_F(LoggerFactoryTest, FileProfileHasSingleSink)
{
    EXPECT_EQ(m_factory.create("test-file", LoggerProfile::File)->sinks().size(), 1);
}

/// \brief Test that a logger created with the console and file profile has exactly two sinks.
TEST_F(LoggerFactoryTest, ConsoleAndFileProfileHasSingleSink)
{
    EXPECT_EQ(m_factory.create("test-combined", LoggerProfile::ConsoleAndFile)->sinks().size(), 2);
}

/// \brief Test that the created logger keeps the user-given name.
TEST_F(LoggerFactoryTest, LoggerKeepsGivenName)
{
    constexpr auto NAME{ "test-name" };

    EXPECT_EQ(m_factory.create(NAME, LoggerProfile::Console)->name(), NAME);
}

/// \brief Test that the created logger is set to trace level.
TEST_F(LoggerFactoryTest, LoggerLevelisTrace)
{
    EXPECT_EQ(m_factory.create("level-test", LoggerProfile::Console)->level(), spdlog::level::trace);
}

/// \brief Test that repeated \ref LoggerFactory::create() calls reuse the same underlying sinks.
TEST_F(LoggerFactoryTest, RepeatedCreateCallsReuseSameSinkInstance)
{
    const auto first{ m_factory.create("first", LoggerProfile::Console) };
    const auto second{ m_factory.create("second", LoggerProfile::Console) };

    EXPECT_EQ(first->sinks().front(), second->sinks().front());
}

} // namespace ful::testing
