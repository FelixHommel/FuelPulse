#include "SQLiteFuelRepository.hpp"

#include "fuel/Domain.hpp"
#include "utility/Assert.hpp"
#include "utility/SQLiteConnection.hpp"

#include "SQLiteCpp/Statement.h"
#include "SQLiteCpp/Transaction.h"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

namespace
{

constexpr auto MEASUREMENT_TABLE_NAME{ "measurements" };
constexpr auto STATION_TABLE_NAME{ "stations" };

constexpr auto CREATE_MEASUREMENT_TABLE{
    "CREATE TABLE measurements (id INTEGER PRIMARY KEY AUTOINCREMENT, stationId TEXT NOT NULL, timestamp INTEGER NOT NULL, priceE5 INTEGER, priceE10 INTEGER, priceDiesel INTEGER)"
};
constexpr auto CREATE_STATIONS_TABLE{
    "CREATE TABLE stations (stationId TEXT PRIMARY KEY, name TEXT, brand TEXT, longitude REAL, latitude REAL)"
};

constexpr auto INSERT_MEASUREMENT{ "INSERT INTO measurements values(?, ?, ?, ?, ?, ?)" };
constexpr auto INSERT_STATION{ "INSERT INTO stations values(?, ?, ?, ?, ?)" };

constexpr auto QUERY_MEASUREMENTS_FROM_TO{
    "SELECT stationId, timestamp, priceE5, priceE10, priceDiesel FROM measurements AS m WHERE m.timestamp BETWEEN ? AND ?"
};
constexpr auto QUERY_STATIONS{ "SELECT * FROM stations" };

/// \brief Convert a \ref std::chrono::system_clock::time_point to long (in ms).
constexpr std::int64_t systemClockToUnixTime(const ful::fuel::SQLiteFuelRepository::TimePoint& tp)
{
    return static_cast<std::int64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count()
    );
}

/// \brief Convert a long (in ms) to \ref std::chrono::system_clock::time_point.
constexpr ful::fuel::SQLiteFuelRepository::TimePoint unixTimeToSystemClock(std::int64_t tp)
{
    return ful::fuel::SQLiteFuelRepository::TimePoint(std::chrono::milliseconds(tp));
}

} // namespace

namespace ful::fuel
{

SQLiteFuelRepository::SQLiteFuelRepository(const std::filesystem::path& dbLocation, SQLiteConnection::OpenMode mode)
    : m_connection(dbLocation, mode)
{
    ensureTableLayout();
}

void SQLiteFuelRepository::store(const Measurement& m)
{
    FUL_ASSERT(m_connection.isOpen());
    FUL_ASSERT(m_connection.mode() == SQLiteConnection::OpenMode::ReadWrite);

    SQLite::Transaction transaction{ m_connection.database() };

    SQLite::Statement insert{ m_connection.database(), ::INSERT_MEASUREMENT };

    // NOLINTBEGIN(readability-magic-numbers): Just indices into the SQLite query
    insert.bind(1);
    insert.bind(2, m.stationId);
    insert.bind(3, ::systemClockToUnixTime(m.timestamp));
    for(int idx{ 4 }; const auto& opt : { m.e5, m.e10, m.diesel })
        opt.has_value() ? insert.bind(idx++, *opt) : insert.bind(idx++);
    // NOLINTEND(readability-magic-numbers)

    insert.exec();

    transaction.commit();
}

void SQLiteFuelRepository::storeStation(const Station& s)
{
    FUL_ASSERT(m_connection.isOpen());
    FUL_ASSERT(m_connection.mode() == SQLiteConnection::OpenMode::ReadWrite);

    SQLite::Transaction transaction{ m_connection.database() };

    SQLite::Statement insert{ m_connection.database(), ::INSERT_STATION };

    // NOLINTBEGIN(readability-magic-numbers): Just indices into the SQLite query
    insert.bind(1, s.id);
    insert.bind(2, s.name);
    insert.bind(3, s.brand);
    insert.bind(4, s.longitude);
    insert.bind(5, s.latitude);
    // NOLINTEND(readability-magic-numbers)

    insert.exec();

    transaction.commit();
}

std::vector<Measurement> SQLiteFuelRepository::loadMeasurements(TimePoint from, TimePoint to)
{
    FUL_ASSERT(m_connection.isOpen());

    SQLite::Statement query{ m_connection.database(), ::QUERY_MEASUREMENTS_FROM_TO };

    // NOLINTBEGIN(readability-magic-numbers): Just indices into the SQLite query
    query.bind(1, ::systemClockToUnixTime(from));
    query.bind(2, ::systemClockToUnixTime(to));

    std::vector<Measurement> result;
    while(query.executeStep())
    {
        result.emplace_back(
            query.getColumn(0).getString(),
            ::unixTimeToSystemClock(query.getColumn(1).getInt64()),
            query.getColumn(2).isNull() ? std::nullopt : std::make_optional(query.getColumn(2).getUInt()),
            query.getColumn(3).isNull() ? std::nullopt : std::make_optional(query.getColumn(3).getUInt()),
            query.getColumn(4).isNull() ? std::nullopt : std::make_optional(query.getColumn(4).getUInt())
        );
    }
    // NOLINTEND(readability-magic-numbers)

    return result;
}

std::vector<Station> SQLiteFuelRepository::loadStations()
{
    FUL_ASSERT(m_connection.isOpen());

    SQLite::Statement query{ m_connection.database(), ::QUERY_STATIONS };

    std::vector<Station> result;
    while(query.executeStep())
    {
        // NOLINTBEGIN(readability-magic-numbers): Just indices into the SQLite query
        result.emplace_back(
            query.getColumn(0).getString(),
            query.getColumn(1).getString(),
            query.getColumn(2).getString(),
            query.getColumn(3).getDouble(),
            query.getColumn(4).getDouble()
        );
        // NOLINTEND(readability-magic-numbers)
    }

    return result;
}

/// \brief Make sure that the connected database has the required tables.
///
/// If the database does not have the required tables, the tables will be created.
void SQLiteFuelRepository::ensureTableLayout() const
{
    FUL_ASSERT(m_connection.isOpen());

    if(!m_connection.database().tableExists(::MEASUREMENT_TABLE_NAME))
    {
        FUL_ASSERT(m_connection.mode() == SQLiteConnection::OpenMode::ReadWrite);
        m_connection.database().exec(::CREATE_MEASUREMENT_TABLE);
    }

    if(!m_connection.database().tableExists(::STATION_TABLE_NAME))
    {
        FUL_ASSERT(m_connection.mode() == SQLiteConnection::OpenMode::ReadWrite);
        m_connection.database().exec(::CREATE_STATIONS_TABLE);
    }
}

} // namespace ful::fuel
