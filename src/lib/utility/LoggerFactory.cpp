#include "LoggerFactory.hpp"

#include <spdlog/common.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace ful
{

LoggerFactory::LoggerFactory()
    : m_consoleSink{ std::make_shared<spdlog::sinks::stdout_color_sink_mt>() }
    , m_fileSink{ std::make_shared<spdlog::sinks::basic_file_sink_mt>("FuelPulse.log", true) }
{}

std::unique_ptr<spdlog::logger> LoggerFactory::create(std::string name, LoggerProfile profile)
{
    const auto sinks{ getSinksForProfile(profile) };

    auto logger{ std::make_unique<spdlog::logger>(std::move(name), sinks.cbegin(), sinks.cend()) };

    logger->set_level(spdlog::level::trace);
    logger->flush_on(spdlog::level::warn);

    return logger;
}

/// \brief Determine which sinks are needed for each \ref LoggerProfile.
///
/// \param profile \ref LoggerProfile of which the sinks are queried
///
/// \returns \ref std::vector with \ref std::shared_ptr to the sinks
std::vector<std::shared_ptr<spdlog::sinks::sink>> LoggerFactory::getSinksForProfile(LoggerProfile profile)
{
    switch(profile)
    {
        using enum LoggerProfile;
    case Console:
        return { m_consoleSink };
    case File:
        return { m_fileSink };
    case ConsoleAndFile:
        return { m_consoleSink, m_fileSink };
    }
}

} // namespace ful
