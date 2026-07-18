#ifndef FUL_SRC_TEST_TEST_UTILITY_HPP
#define FUL_SRC_TEST_TEST_UTILITY_HPP

#include <chrono>
#include <functional>
#include <thread>

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
bool waitOn(std::function<bool()> pred, std::chrono::milliseconds timeout = ::DEFAULT_TIMEOUT)
{
    const auto deadline{ std::chrono::steady_clock::now() + timeout };

    while(std::chrono::steady_clock::now() < deadline)
    {
        if(pred())
            return true;

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    return pred();
}


} // namespace ful::testing

#endif // !FUL_SRC_TEST_TEST_UTILITY_HPP
