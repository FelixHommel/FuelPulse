#ifndef FUL_SRC_TEST_TEST_UTILITY_CONTROLLABLE_TANKER_KOENIG_GATEWAY_HPP
#define FUL_SRC_TEST_TEST_UTILITY_CONTROLLABLE_TANKER_KOENIG_GATEWAY_HPP

#include <gtest/gtest.h>

#include <atomic>
#include <memory>
#include <stdexcept>
#include <string>

namespace ful::testing
{

namespace
{

/// \brief Example response provided on https://creativecommons.tankerkoenig.de/swagger.
///
/// \see https://creativecommons.tankerkoenig.de/swagger/#/search/get_stations_postalcode
constexpr auto SAMPLE_VALID_RESPONSE{
    R"(
{
  "apiVersion": "4.0.6",
  "timestamp": "2020-01-04T22:14:06+01:00",
  "stations": [
    {
      "country": "de",
      "id": "83d5ac80-4f23-4106-b054-7c7704bfcb95",
      "name": "Aral Tankstelle",
      "brand": "ARAL",
      "street": "Cannstatter Straße 3",
      "postalCode": "70190",
      "place": "Stuttgart",
      "coords": {
        "lat": 48.78922,
        "lng": 9.192324
      },
      "isOpen": true,
      "closesAt": "2020-01-05T06:00:00+01:00",
      "opensAt": "2020-01-05T06:00:00+01:00",
      "openingTimes": [
        {
          "days": [
            "mon"
          ],
          "times": [
            {
              "open": "06:00",
              "close": "20:00"
            }
          ]
        }
      ],
      "dist": 2.5,
      "fuels": [
        {
          "category": "gasoline",
          "name": "Super E5",
          "price": 1.399,
          "lastChange": {
            "timestamp": "2020-01-04T22:05:06+01",
            "amount": -0.07
          }
        }
      ]
    }
  ]
}
)"

};

} // namespace

class TestGateway
{
public:
    TestGateway() = default;
    virtual ~TestGateway() = default;

    TestGateway(const TestGateway&) = default;
    TestGateway& operator=(const TestGateway&) = default;
    TestGateway(TestGateway&&) = delete;
    TestGateway& operator=(TestGateway&&) = delete;

    [[nodiscard]] virtual std::string fetchPrices(const std::string&, unsigned int) const = 0;

    [[nodiscard]] int callCount() const { return m_callCount->load(std::memory_order_relaxed); }

protected:
    std::shared_ptr<std::atomic<int>> m_callCount{ std::make_shared<std::atomic<int>>(0) };
};

/// \brief Gateway that returns \ref SAMPLE_VALID_RESPONSE immediately, counting invocations.
///
/// \author Felix Hommel
/// \date 8/18/2026
class ImmediateGateway final : public TestGateway
{
public:
    // NOLINTNEXTLINE(readability-named-parameter): Not needed for this test implementation
    [[nodiscard]] std::string fetchPrices(const std::string&, unsigned int) const override
    {
        m_callCount->fetch_add(1, std::memory_order_relaxed);
        return SAMPLE_VALID_RESPONSE;
    }
};

/// \brief Gateway that blocks inside \ref BlockingCountingGateway::fetchPrices() until explicitly released, tracking
///     invocation count and fetch-started state.
///
/// \author Felix Hommel
/// \date 8/18/2026
class BlockingCountingGateway final : public TestGateway
{
public:
    // NOLINTNEXTLINE(readability-named-parameter): Not needed for this test implementation
    [[nodiscard]] std::string fetchPrices(const std::string&, unsigned int) const override
    {
        m_callCount->fetch_add(1, std::memory_order_relaxed);
        m_state->fetchStarted.store(true, std::memory_order_release);
        m_state->fetchStarted.notify_all();

        m_state->releaseGate.wait(false, std::memory_order_acquire);

        m_state->fetchStarted.store(false, std::memory_order_release);
        m_state->releaseGate.store(false, std::memory_order_release);
        return SAMPLE_VALID_RESPONSE;
    }

    /// \brief Let a currently-blocked \ref BlockingCountingGateway::fetchPrices() call return.
    void release()
    {
        m_state->releaseGate.store(true, std::memory_order_release);
        m_state->releaseGate.notify_all();
    }

    /// \brief Block until a \ref BlockingCountingGateway::fetchPrices() call has started (and not released yet).
    void waitUntilFetchStarted() const { m_state->fetchStarted.wait(false, std::memory_order_acquire); }

private:
    struct State
    {
        std::atomic<bool> fetchStarted{ false };
        std::atomic<bool> releaseGate{ false };
    };

    std::shared_ptr<State> m_state{ std::make_shared<State>() };
};

/// \brief Gateway that always throws a \ref std::runtime_error and counts its invocations.
///
/// \author Felix Hommel
/// \date 8/18/2026
class ThrowingStdExceptionGateway final : public TestGateway
{
public:
    // NOLINTNEXTLINE(readability-named-parameter): Not needed for this test implementation
    [[nodiscard]] std::string fetchPrices(const std::string&, unsigned int) const override
    {
        m_callCount->fetch_add(1, std::memory_order_relaxed);
        throw std::runtime_error("network failure");
    }
};

/// \brief Gateway that always throws a non-std::exception.
///
/// \author Felix Hommel
/// \date 8/18/2026
class ThrowingNonStdExceptionGateway final : public TestGateway
{
public:
    // NOLINTNEXTLINE(readability-named-parameter): Not needed for this test implementation
    [[nodiscard]] std::string fetchPrices(const std::string&, unsigned int) const override
    {
        m_callCount->fetch_add(1, std::memory_order_relaxed);
        throw 1;
    }
};

/// \brief Gateway that returns syntactically invalid JSON.
///
/// \author Felix Hommel
/// \date 8/18/2026
class MalformedJsonGateway final : public TestGateway
{
public:
    // NOLINTNEXTLINE(readability-named-parameter): Not needed for this test implementation
    [[nodiscard]] std::string fetchPrices(const std::string&, unsigned int) const override
    {
        m_callCount->fetch_add(1, std::memory_order_relaxed);
        return "{ malformed: json }";
    }
};

} // namespace ful::testing

#endif // !FUL_SRC_TEST_TEST_UTILITY_CONTROLLABLE_TANKER_KOENIG_GATEWAY_HPP
