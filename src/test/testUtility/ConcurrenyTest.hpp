#ifndef FUL_SRC_TEST_TEST_UTILITY_CONCURRENCY_TEST_HPP
#define FUL_SRC_TEST_TEST_UTILITY_CONCURRENCY_TEST_HPP

#include <chrono>
#include <functional>

namespace
{

constexpr auto DEFAULT_TIMEOUT{ std::chrono::milliseconds(2000) };

} // namespace

namespace ful::testing
{

/// \brief Poll \p pred until it returns true or \p timeout elapses.
///
/// \param pred The predicate function
/// \param timeout (optional) Timeout after which to abort in case the predicate still has not evaluated to true
bool waitOn(std::function<bool()> pred, std::chrono::milliseconds timeout = ::DEFAULT_TIMEOUT);

} // namespace ful::testing

#endif // !FUL_SRC_TEST_TEST_UTILITY_CONCURRENCY_TEST_HPP
