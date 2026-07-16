#ifndef FUL_SRC_LIB_UTILITY_LOGGER_FACTORY_HPP
#define FUL_SRC_LIB_UTILITY_LOGGER_FACTORY_HPP

#include <spdlog/logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace ful
{

enum class LoggerProfile : std::uint8_t
{
    Console,
    File,
    ConsoleAndFile
};

/// \brief Factory to create new logger instances.
///
/// \author Felix Hommel
/// \date 7/16/2026
class LoggerFactory
{
public:
    LoggerFactory();
    ~LoggerFactory() = default;

    LoggerFactory(const LoggerFactory&) = default;
    LoggerFactory& operator=(const LoggerFactory&) = default;
    LoggerFactory(LoggerFactory&&) = default;
    LoggerFactory& operator=(LoggerFactory&&) = default;

    /// \brief Create a logger with specified \p name and \p profile.
    ///
    /// \param name The name of the logger
    /// \param profile \ref LoggerProfile of the new logger
    ///
    /// \returns \ref std::unique_ptr to the created logger
    std::unique_ptr<spdlog::logger> create(std::string name, LoggerProfile profile);

private:
    std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> m_consoleSink;
    std::shared_ptr<spdlog::sinks::basic_file_sink_mt> m_fileSink;

    [[nodiscard]] std::vector<std::shared_ptr<spdlog::sinks::sink>> getSinksForProfile(LoggerProfile profile);
};

} // namespace ful

#endif // !FUL_SRC_LIB_UTILITY_LOGGER_FACTORY_HPP
