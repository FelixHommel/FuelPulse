#ifndef FUL_SRC_LIB_FUEL_I_FUEL_REPOSITORY_HPP
#define FUL_SRC_LIB_FUEL_I_FUEL_REPOSITORY_HPP

#include "fuel/Domain.hpp"

#include <chrono>
#include <vector>

namespace ful::fuel
{

/// \brief Interface definition for fuel data storage.
///
/// \author Felix Hommel
/// \date 7/23/2026
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

    /// \brief Store a \ref Measurement.
    ///
    /// \param measurement The \ref Measurement that is stored
    virtual void store(const Measurement& measurement) = 0;
    /// \brief Store a \ref Station.
    ///
    /// \param station The \ref Station that is stored
    virtual void storeStation(const Station& station) = 0;
    /// \brif Load all \ref Station.
    ///
    /// \returns \ref std::vector of \ref Station
    virtual std::vector<Station> loadStations() = 0;
    /// \brief Load all \ref Measurement that are between \p from and \p to.
    ///
    /// \param from Lower time bound
    /// \param to Upper time bound
    ///
    /// \returns \ref std::vector of \ref Measurement that are between \p from and \p to
    virtual std::vector<Measurement> loadMeasurements(TimePoint from, TimePoint to) = 0;
};

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_I_FUEL_REPOSITORY_HPP
