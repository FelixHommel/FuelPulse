#include "ConcurrenyTest.hpp"

#include <chrono>
#include <functional>
#include <thread>

namespace ful::testing
{

bool waitOn(std::function<bool()> pred, std::chrono::milliseconds timeout)
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
