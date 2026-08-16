#ifndef FUL_SRC_TEST_TEST_UTILITY_CONTROLLABLE_TANKER_KOENIG_GATEWAY_HPP
#define FUL_SRC_TEST_TEST_UTILITY_CONTROLLABLE_TANKER_KOENIG_GATEWAY_HPP

#include <atomic>
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

/// \brief This is a sample gateway implementations used in testing the \ref ICollector interface for the Tanker Koenig
///     API. It is controllable by the test which means that the collector behavior itself can be targeted.
///
/// \author Felix Hommel
/// \date 8/16/2026
class ControllableTankerKoenigGateway
{
public:
    // NOLINTNEXTLINE(readability-named-parameter): Not needed for this dummy function
    std::string fetchPrices(const std::string&, unsigned int) const
    {
        m_fetchStarted.store(true, std::memory_order_release);
        m_fetchStarted.notify_all();

        m_releaseGate.wait(false, std::memory_order_acquire);

        return SAMPLE_VALID_RESPONSE;
    }

    /// \brief Tell \ref ControllableTankerKoenigGateway::fetchPrices() to proceed.
    void release()
    {
        m_releaseGate.store(true, std::memory_order_release);
        m_releaseGate.notify_all();
    }
    /// \brief Wait until \ref ControllableTankerKoenigGateway::fetchPrices() starts the collection.
    void waitUnitilFetchStarted() const { m_fetchStarted.wait(false, std::memory_order_acquire); }

private:
    mutable std::atomic<bool> m_fetchStarted{ false };
    mutable std::atomic<bool> m_releaseGate{ false };
};

} // namespace ful::testing

#endif // !FUL_SRC_TEST_TEST_UTILITY_CONTROLLABLE_TANKER_KOENIG_GATEWAY_HPP
