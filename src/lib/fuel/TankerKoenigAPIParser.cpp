#include "TankerKoenigAPIParser.hpp"

#include "fuel/Domain.hpp"

#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

#include <chrono>
#include <sstream>
#include <stdexcept>
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
        throw std::runtime_error(""); // TODO: Implement custom exception

    return result;
}

} // namespace detail

std::vector<Measurement> parseStationPrices(const std::string& jsonRaw)
{
    using namespace detail;

    const auto json = nlohmann::json::parse(jsonRaw);

    if(!json.contains("stations"))
        return {};

    std::vector<Measurement> result;

    const auto timestamp{ detail::parseTimestamp(json["timestamp"].get<std::string>()) };
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
