#ifndef GAS_SRC_TEST_TEST_UTILITY_RANDOM_NUMBER_GENERATOR_HPP
#define GAS_SRC_TEST_TEST_UTILITY_RANDOM_NUMBER_GENERATOR_HPP

#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace
{

/// \brief Static provider for an empty set of \p T
///
/// \tparam T Can be any numerical type
///
/// \author Felix Hommel
/// \date 2/4/2026
template<typename T>
const std::unordered_set<T>& emptySetProvider()
{
    static const std::unordered_set<T> empty{};

    return empty;
}

template<typename T> concept NumericalValue = std::is_integral_v<T> || std::is_floating_point_v<T>;

constexpr std::uint32_t DEFAULT_RNG_SEED{ 0xC0FFEEu };

} // namespace

namespace gas::testing
{

/// \brief Generate a random number in [min, max], can generate for integer and
/// floating point types.
///
/// When the exclusion list is used, be careful to not exclude all possible
/// values. This *can* cause an infinite loop.
///
/// \tparam T Can be any numerical type
/// \param min (optional) The minimum value the random number has to have
/// \param max (optional) The maximum value the random number has to have
/// \param exclusions (optional) Exclude specific numbers. Careful when using
/// with floating point values
/// \param seed (optional) Seed for the random number generator to allow for
/// deterministic reproduction
///
/// \author Felix Hommel
/// \date 2/4/2025
template<::NumericalValue T>
T generateRandomValue(
    T min = std::numeric_limits<T>::min(),
    T max = std::numeric_limits<T>::max(),
    const std::unordered_set<T>& exclusions = ::emptySetProvider<T>(),
    std::uint32_t seed = ::DEFAULT_RNG_SEED
)
{
    thread_local std::mt19937 mt(static_cast<std::seed_seq::result_type>(seed));

    if constexpr(std::is_integral_v<T>)
    {
        std::uniform_int_distribution<T> dist{ min, max };

        while(true)
        {
            if(T val{ dist(mt) }; !exclusions.contains(val))
                return val;
        }
    }
    else if constexpr(std::is_floating_point_v<T>)
    {
        std::uniform_real_distribution<T> dist{ min, max };

        while(true)
        {
            if(T val{ dist(mt) }; !exclusions.contains(val))
                return val;
        }
    }
}

/// \brief Create a vector filled with random values
///
/// When the exclusion list is used, be careful to not exclude all possible
/// values. This *can* cause an infinite loop.
///
/// \tparam T Can be any numerical type.
/// \param n How many elements to create
/// \param min (optional) The minimum value the random number has to have
/// \param max (optional) The maximum value the random number has to have
/// \param exclusions (optional) Exclude specific numbers. Careful when using
/// with floating point values
/// \param seed (optional) Seed for the random number generator to allow for
/// deterministic reproduction
///
/// \author Felix Hommel
/// \date 2/4/2025
template<::NumericalValue T>
std::vector<T> generateRandomVector(
    std::size_t n,
    T min = std::numeric_limits<T>::min(),
    T max = std::numeric_limits<T>::max(),
    const std::unordered_set<T>& exclusions = ::emptySetProvider<T>(),
    std::uint32_t seed = DEFAULT_RNG_SEED
)
{
    std::vector<T> vec(n);

    for(std::size_t i{ 0 }; i < n; ++i)
        vec.push_back(generateRandomValue<T>(min, max, exclusions, seed));

    return vec;
}

} // namespace gas::testing

#endif // !GAS_SRC_TEST_TEST_UTILITY_RANDOM_NUMBER_GENERATOR_HPP
