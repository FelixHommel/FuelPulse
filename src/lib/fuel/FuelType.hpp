#ifndef FUL_SRC_LIB_FUEL_FUEL_TYPE_HPP
#define FUL_SRC_LIB_FUEL_FUEL_TYPE_HPP

#include <cstdint>
#include <string_view>
#include <utility>

namespace ful::fuel
{

/// \brief Enum to describe the different types of fuels that are supported.
///
/// \author Felix Hommel
/// \date 8/16/2026
enum class FuelType : std::uint8_t
{
    Diesel,
    E10,
    E5
};

/// \brief Convert a \ref FuelType to \ref std::string.
///
/// \param t The \ref FuelType that is being converted
///
/// \returns The \ref std::string representation of \p t
[[nodiscard]] constexpr std::string_view fuelTypeToString(FuelType t)
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

    std::unreachable();
}

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_FUEL_TYPE_HPP
