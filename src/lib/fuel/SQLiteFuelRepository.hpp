#ifndef FUL_SRC_LIB_FUEL_SQLITE_FUEL_REPOSITORY_HPP
#define FUL_SRC_LIB_FUEL_SQLITE_FUEL_REPOSITORY_HPP

#include "fuel/IFuelRepository.hpp"
#include "utility/SQLiteConnection.hpp"

#include <SQLiteCpp/Database.h>

#include <filesystem>
#include <vector>

namespace ful::fuel
{

/// \brief SQLite storage repository for fuel pricing data.
///
/// \author Felix Hommel
/// \date 7/24/2026
class SQLiteFuelRepository : public IFuelRepository
{
public:
    /// \brief Create a new \ref SQLiteFuelRepository
    ///
    /// \param dbLocation Path to the database file
    /// \param mode (optional) How the connection is established
    SQLiteFuelRepository(
        const std::filesystem::path& dbLocation, SQLiteConnection::OpenMode mode = SQLiteConnection::OpenMode::ReadWrite
    );
    ~SQLiteFuelRepository() override = default;

    SQLiteFuelRepository(const SQLiteFuelRepository&) = delete;
    SQLiteFuelRepository& operator=(const SQLiteFuelRepository&) = delete;
    SQLiteFuelRepository(SQLiteFuelRepository&&) = default;
    SQLiteFuelRepository& operator=(SQLiteFuelRepository&&) = default;

    void store(const Measurement& m) override;
    void storeStation(const Station& s) override;
    std::vector<Measurement> loadMeasurements(TimePoint from, TimePoint to) override;
    std::vector<Station> loadStations() override;

private:
    SQLiteConnection m_connection;

    void ensureTableLayout() const;
};

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_SQLITE_FUEL_REPOSITORY_HPP
