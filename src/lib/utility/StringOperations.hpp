#ifndef FUL_SRC_LIB_UTILITY_STRING_OPERATIONS_HPP
#define FUL_SRC_LIB_UTILITY_STRING_OPERATIONS_HPP

#include <algorithm>
#include <cctype>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>

namespace ful
{

/// \brief In-place trim on the left side of the string.
///
/// \param s The string to be trimmed
constexpr void ltrim(std::string& s)
{
    s.erase(std::begin(s), std::ranges::find_if(s, [](unsigned char c) { return !std::isspace(c); }));
}

/// \brief In-place trim on the right side of the string.
///
/// \param s The string to be trimmed
constexpr void rtrim(std::string& s)
{
    s.erase(
        std::ranges::find_if(std::views::reverse(s), [](unsigned char c) { return !std::isspace(c); }).base(),
        std::end(s)
    );
}

/// \brief In-place trim on both sides of the string.
///
/// \param s The string to be trimmed
constexpr void trim(std::string& s)
{
    ltrim(s);
    rtrim(s);
}

/// \brief Create a trimmed version of a provided string.
///
/// \param sv \ref std::string_view to the original string
///
/// \returns \ref std::string with a trimmed version of \p sv
constexpr std::string trimView(std::string_view sv)
{
    std::string s(sv);

    ltrim(s);
    rtrim(s);

    return s;
}

} // namespace ful

#endif // !FUL_SRC_LIB_UTILITY_STRING_OPERATIONS_HPP
