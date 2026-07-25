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
    std::vector<Station> loadStations() override;
    std::vector<Measurement> loadMeasurements(TimePoint from, TimePoint to) override;

private:
    static constexpr auto MEASUREMENT_TABLE{ "measurements" };
    static constexpr auto CREATE_MEASUREMENT_TABLE_STATEMENT{
        "CREATE TABLE measurements (stationId TEXT PRIMARY KEY, timestamp INTEGER, priceDiesel INTEGER, priceE5 INTEGER, priceE10 INTEGER)"
    };
    static constexpr auto QUERY_MEASUREMENT_TABLE_FROM_TO{
        "SELECT * FROM measurements AS m WHERE m.timestamp BETWEEN ? AND ?"
    };
    static constexpr auto QUERY_INSERT_MEASUREMENT{ "INSERT INTO measurements values(?, ?, ?, ?, ?)" };

    SQLiteConnection m_connection;

    void ensureTableLayout() const;
};

} // namespace ful::fuel

#endif // !FUL_SRC_LIB_FUEL_SQLITE_FUEL_REPOSITORY_HPP
