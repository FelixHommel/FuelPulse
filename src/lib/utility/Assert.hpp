#ifndef FUL_SRC_LIB_CORE_UTILITY_HPP
#define FUL_SRC_LIB_CORE_UTILITY_HPP

#include <cstdio>
#include <cstdlib>
#include <source_location>

namespace ful::assertion
{

/// \brief Assertion failure handler.
///
/// Document that an assertion failed and why, then calls std::abort.
///
/// \param pExpr the assertions condition
/// \param pUserMsg user provided message
/// \param loc(optional) std::source_location from where the function was called
[[noreturn]] inline void assertion_fail(
    const char* pExpr, const char* pUserMsg, std::source_location loc = std::source_location::current()
) noexcept
{
    if((pUserMsg != nullptr) && (*pUserMsg != 0))
        std::fprintf(
            stderr,
            "ful assertion failed: %s\n\tLocation: %s:%u (%s)\n\tMessage: %s",
            pExpr,
            loc.file_name(),
            loc.line(),
            loc.function_name(),
            pUserMsg
        );
    else
        std::fprintf(
            stderr,
            "ful assertion failed: %s\n\tLocation: %s:%u (%s)",
            pExpr,
            loc.file_name(),
            loc.line(),
            loc.function_name()
        );

    std::abort();
}

constexpr const char* msgOrNull() noexcept
{
    return nullptr;
}
constexpr const char* msgOrNull(const char* pMsg) noexcept
{
    return pMsg;
}

} // namespace ful::assertion

// NOLINTBEGIN(cppcoreguidelines-macro-usage, cppcoreguidelines-avoid-do-while):
//      Definition of assertion macro this way is fine and not solvable by
//      variadic template function, Usage of do-while for this purpose is fine.
// NOTE: Ensure FUL_ENABLE_ASSERTIONS is defined
#if !defined(FUL_ENABLE_ASSERTIONS)
#    define FUL_ENABLE_ASSERTIONS 0
#endif

#if FUL_ENABLE_ASSERTIONS
/// \brief Assert on \p cond
///
/// Failure of the assertion results in std::abort() being called
///
/// \param cond the condition the assertion needs to pass
#    define FUL_ASSERT(cond, ...)                                                                  \
        do                                                                                         \
        {                                                                                          \
            if(!(cond))                                                                            \
            {                                                                                      \
                ::ful::assertion::assertion_fail(#cond, ::ful::assertion::msgOrNull(__VA_ARGS__)); \
            }                                                                                      \
        }                                                                                          \
        while(0)
#else
#    define FUL_ASSERT(cond, ...) ((void)0)
#endif
// NOLINTEND(cppcoreguidelines-macro-usage, cppcoreguidelines-avoid-do-while)

#endif // !FUL_SRC_LIB_CORE_UTILITY_HPP
