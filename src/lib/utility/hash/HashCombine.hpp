#ifndef FUL_SRC_LIB_UTILITY_HASH_HASH_COMBINE_HPP
#define FUL_SRC_LIB_UTILITY_HASH_HASH_COMBINE_HPP

#include <cstddef>
#include <functional>

namespace ful::hash
{

// NOLINTBEGIN(readability-magic-numbers): For more info on the magic numbers see:
//  https://stackoverflow.com/questions/4948780/magic-number-in-boosthash-combine

/// \brief Boost hash_combine style hash function.
///
/// \tparam T The type that is hashed
///
/// \param seed The seed of the hash value
/// \param value The object that is hashed
template<typename T>
constexpr void hashCombine(std::size_t& seed, const T& value)
{
    seed ^= std::hash<T>{}(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// NOLINTEND(readability-magic-numbers)

} // namespace ful::hash

#endif // !FUL_SRC_LIB_UTILITY_HASH_HASH_COMBINE_HPP
