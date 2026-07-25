#ifndef FUL_SRC_LIB_FUEL_I_FUEL_REPOSITORY_HPP
#define FUL_SRC_LIB_FUEL_I_FUEL_REPOSITORY_HPP

#include "fuel/Domain.hpp"

#include <chrono>
#include <vector>

namespace ful::fuel
{

class IFuelRepository
{
public:
    using TimePoint = std::chrono::system_clock::time_point;

    IFuelRepository() = default;
    virtual ~IFuelRepository() = default;

    IFuelRepository(const IFuelRepository&) = default;
    IFuelRepository& operator=(const IFuelRepository&) = default;
    IFuelRepository(IFuelRepository&&) = default;
    IFuelRepository& operator=(IFuelRepository&&) = default;

    virtual void store(const Measurement& m) = 0;
    virtual void storeStation(const Station& s) = 0;
    virtual std::vector<Station> loadStations() = 0;
    virtual std::vector<Measurement> loadMeasurements(TimePoint from, TimePoint to) = 0;
};

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_I_FUEL_REPOSITORY_HPP
