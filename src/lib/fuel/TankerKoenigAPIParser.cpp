#include "TankerKoenigAPIParser.hpp"

#include "fuel/Domain.hpp"

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <spdlog/spdlog.h>

#include <chrono>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace ful::fuel
{

namespace detail
{

std::chrono::system_clock::time_point parseTimestamp(std::string timestamp)
{
    std::istringstream stream{ std::move(timestamp) };

    std::chrono::sys_time<std::chrono::seconds> result;
    stream >> std::chrono::parse("%FT%T%Ez", result);

    if(stream.fail())
    {
        spdlog::warn("Unable to find a timetamp in the API response. Using placeholder value");
        return std::chrono::system_clock::time_point::min();
    }

    return result;
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
