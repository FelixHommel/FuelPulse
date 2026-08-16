#ifndef FUL_SRC_LIB_FUEL_TANKER_KOENIG_API_PARSER_HPP
#define FUL_SRC_LIB_FUEL_TANKER_KOENIG_API_PARSER_HPP

#include "fuel/Domain.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace ful::fuel
{

namespace detail
{

enum class FuelType : std::uint8_t
{
    Diesel,
    E10,
    E5
};

[[nodiscard]] constexpr std::string fuelTypeToString(FuelType t)
{
    switch(t)
    {
        using enum FuelType;
    case Diesel:
        return "diesel";
    case E10:
        return "e10";
    case E5:
        return "e5";
    }
}

std::chrono::system_clock::time_point parseTimestamp(std::string timestamp);

template<FuelType T>
std::optional<PriceCents> parseFuelPrice(const nlohmann::json& station)
{
    static constexpr auto TARGET_TYPE{ fuelTypeToString(T) };
    static constexpr auto TO_CENTS_MOD{ 1000u };

    if(!station.contains("fuels"))
        throw std::runtime_error(""); // TODO Implement custom exception

    for(const auto& fuelObj : station["fuels"])
    {
        auto currentType{ fuelObj["name"].get<std::string>() };
        std::transform(currentType.begin(), currentType.end(), currentType.begin(), [](unsigned char c) {
            return std::tolower(c);
        });

        if(!currentType.contains(TARGET_TYPE))
            continue;

        return std::make_optional(static_cast<unsigned int>(std::round(fuelObj["price"].get<double>() * TO_CENTS_MOD)));
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
