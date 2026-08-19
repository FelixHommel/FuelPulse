#include "TankerKoenigAPIParser.hpp"

#include "fuel/Domain.hpp"

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>

#include <charconv>
#include <chrono>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace ful::fuel
{

namespace detail
{

std::chrono::system_clock::time_point parseTimestamp(const std::string& timestamp)
{
    // NOTE: Fixed format from the API: "YYYY-MM-DDTHH:MM:SS±HH:MM", e.g. "2020-01-04T22:14:06+01:00"
    constexpr std::size_t MIN_LENGTH{ 19 }; // NOTE: Because "YYYY-MM-DDTHH:MM:SS"

    const auto parseInt = [&timestamp](std::size_t offset, std::size_t len) -> int {
        const auto* begin{ std::next(timestamp.data(), static_cast<long>(offset)) };
        int value{};
        const auto [ptr, ec]{ std::from_chars(begin, std::next(begin, static_cast<long>(len)), value) };

        if(ec != std::errc{} || ptr != std::next(begin, static_cast<long>(len)))
            throw std::runtime_error("Failed to parse timestamp: " + timestamp);

        return value;
    };

    // NOLINTBEGIN(readability-magic-numbers): All array indices are directly based on the ISO timestamp
    if(timestamp.size() < MIN_LENGTH || timestamp[4] != '-' || timestamp[7] != '-' || timestamp[10] != 'T'
       || timestamp[13] != ':' || timestamp[16] != ':')
    {
        spdlog::warn("Failed to parse the following timestamp '{}', proceeding with default timestamp");
        return std::chrono::system_clock::time_point::min();
    }
    // NOLINTEND(readability-magic-numbers)

    const std::chrono::year_month_day ymd{
        std::chrono::year{ parseInt(0, 4) },
        std::chrono::month{ static_cast<unsigned>(parseInt(5, 2)) },
        std::chrono::day{ static_cast<unsigned>(parseInt(8, 2)) },
    };

    if(!ymd.ok())
    {
        spdlog::warn("Failed to parse the following timestamp '{}', proceeding with default timestamp");
        return std::chrono::system_clock::time_point::min();
    }

    const auto localTime{ std::chrono::sys_days{ ymd } + std::chrono::hours{ parseInt(11, 2) }
                          + std::chrono::minutes{ parseInt(14, 2) } + std::chrono::seconds{ parseInt(17, 2) } };

    std::chrono::minutes offset{ 0 };
    if(timestamp.size() > MIN_LENGTH)
    {
        const auto sign{ timestamp[MIN_LENGTH] };
        if(sign != '+' && sign != '-')
        {
            spdlog::warn("Failed to parse the following timestamp '{}', proceeding with default timestamp");
            return std::chrono::system_clock::time_point::min();
        }

        auto pos{ MIN_LENGTH + 1 };
        const auto offsetHours{ parseInt(pos, 2) };
        pos += (pos + 2 < timestamp.size() && timestamp[pos + 2] == ':') ? 3 : 2;
        const auto offsetMinutes{ pos + 2 <= timestamp.size() ? parseInt(pos, 2) : 0 };

        offset = std::chrono::hours{ offsetHours } + std::chrono::minutes{ offsetMinutes };
        if(sign == '-')
            offset = -offset;
    }

    return std::chrono::time_point_cast<std::chrono::system_clock::duration>(localTime - offset);
}

} // namespace detail

std::vector<Measurement> parseStationPrices(const std::string& jsonRaw)
{
    using namespace detail;

    const auto json = nlohmann::json::parse(jsonRaw);

    if(!json.contains("stations"))
    {
        spdlog::warn("API Response did not include a stations field");
        return {};
    }

    const auto timestamp{ detail::parseTimestamp(json["timestamp"].get<std::string>()) };

    std::vector<Measurement> result;
    for(const auto& s : json["stations"])
    {
        result.emplace_back(
            s["id"],
            timestamp,
            parseFuelPrice<FuelType::E5>(s),
            parseFuelPrice<FuelType::E10>(s),
            parseFuelPrice<FuelType::Diesel>(s)
        );
    }

    return result;
}

} // namespace ful::fuel
