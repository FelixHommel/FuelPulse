#ifndef FUL_SRC_LIB_FUEL_TANKER_KOENIG_API_PARSER_HPP
#define FUL_SRC_LIB_FUEL_TANKER_KOENIG_API_PARSER_HPP

#include "fuel/Domain.hpp"
#include "fuel/FuelType.hpp"

#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <optional>
#include <string>
#include <vector>

namespace ful::fuel
{

namespace detail
{

/// \brief Parse an ISO-8601 timestamp to \ref std::chrono::system_clock::time_point.
///
/// \param timestamp The \ref std::string containing the timestamp in ISO-8601 format
///
/// \returns A \ref std::chrono::system_clock::time_point from \p timestamp
std::chrono::system_clock::time_point parseTimestamp(const std::string& timestamp);

/// \brief Extract the fuel price for \p T from \p station.
///
/// \tparam The \ref FuelType for which the price is extracted
///
/// \param station A \ref nlohmann::json document that contains the data of a single station.
///
/// \returns A \ref std::optional containing the price of \p T, \ref std::nullopt if the station does not offer \p T
template<FuelType T>
std::optional<PriceCents> parseFuelPrice(const nlohmann::json& station)
{
    static constexpr auto TARGET_TYPE{ fuelTypeToString(T) };
    static constexpr auto TO_CENTS{ 1000u };

    if(!station.contains("fuels"))
    {
        spdlog::warn("Station with id '{}' does not have fuel information", station["id"].get<std::string>());
        return std::nullopt;
    }

    for(const auto& fuelObj : station["fuels"])
    {
        auto currentType{ fuelObj["name"].get<std::string>() };
        std::transform(currentType.begin(), currentType.end(), currentType.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        if(!currentType.contains(TARGET_TYPE))
            continue;

        return std::make_optional(static_cast<unsigned int>(std::round(fuelObj["price"].get<double>() * TO_CENTS)));
    }

    return std::nullopt;
}

} // namespace detail

/// \brief Extract the fuel prices from a JSON response from the TankerKoenig API.
///
/// \param jsonRaw The raw string response returned by the API
///
/// \return Parsed \ref std::vector of \ref Measurement extracted from \p json
std::vector<Measurement> parseStationPrices(const std::string& jsonRaw);

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_TANKER_KOENIG_API_PARSER_HPP
