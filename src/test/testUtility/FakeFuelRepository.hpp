#ifndef FUL_SRC_TEST_TEST_UTILITY_FAKE_FUEL_REPOSITORY_HPP
#define FUL_SRC_TEST_TEST_UTILITY_FAKE_FUEL_REPOSITORY_HPP

#include "fuel/Domain.hpp"
#include "fuel/IFuelRepository.hpp"

#include <algorithm>
#include <iterator>
#include <vector>

namespace ful::testing
{

/// \brief In-memory \ref ful::fuel::IFuelRepository test double.
///
/// \author Felix Hommel
/// \date 8/27/2026
class FakeFuelRepository : public fuel::IFuelRepository
{
public:
    FakeFuelRepository() = default;
    ~FakeFuelRepository() override = default;

    FakeFuelRepository(const FakeFuelRepository&) = default;
    FakeFuelRepository& operator=(const FakeFuelRepository&) = default;
    FakeFuelRepository(FakeFuelRepository&&) = default;
    FakeFuelRepository& operator=(FakeFuelRepository&&) = default;

    void store(const fuel::Measurement& measurement) override { m_measurements.push_back(measurement); }
    void storeStation(const fuel::Station& station) override { m_stations.push_back(station); }

    [[nodiscard]] std::vector<fuel::Station> loadStations() const override { return m_stations; }

    [[nodiscard]] std::vector<fuel::Measurement> loadMeasurements(TimePoint from, TimePoint to) const override
    {
        std::vector<fuel::Measurement> result;

        std::ranges::copy_if(m_measurements, std::back_inserter(result), [&from, &to](const fuel::Measurement& m) {
            return m.timestamp >= from && m.timestamp <= to;
        });

        return result;
    }

private:
    std::vector<fuel::Measurement> m_measurements;
    std::vector<fuel::Station> m_stations;
};

} // namespace ful::testing

#endif // !FUL_SRC_TEST_TEST_UTILITY_FAKE_FUEL_REPOSITORY_HPP
