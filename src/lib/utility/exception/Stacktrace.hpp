#ifndef FUL_SRC_LIB_UTILITY_EXCEPTION_STACKTRACE_HPP
#define FUL_SRC_LIB_UTILITY_EXCEPTION_STACKTRACE_HPP

#if FUL_USE_STACKTRACE
#    include <stacktrace>
#endif

#include <string>

namespace ful::detail
{

/// \brief A wrapper class for \ref std::stacktrace that only exposes the real stacktrace when it is available on the
///     current platform.
///
/// \author Felix Hommel
/// \date 8/8/2026
class Stacktrace
{
public:
#if FUL_USE_STACKTRACE
    Stacktrace() : m_trace{ std::stacktrace::current(2) } {}
#else
    Stacktrace() = default;
#endif

    // NOLINTBEGIN(readability-convert-member-functions-to-static): Not always possible to be static

    /// \brief Return a string formatted version of the stacktrace if possible.
    [[nodiscard]] std::string format() const
    {
#if FUL_USE_STACKTRACE
        return std::to_string(m_trace);
#else
        return {};
#endif
    }

    // NOLINTEND(readability-convert-member-functions-to-static)

private:
#if FUL_USE_STACKTRACE
    std::stacktrace m_trace;
#endif
};

} // namespace ful::detail

#endif // !FUL_SRC_LIB_UTILITY_EXCEPTION_STACKTRACE_HPP
