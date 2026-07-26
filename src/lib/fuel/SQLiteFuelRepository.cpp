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

    SQLite::Statement insert{ m_connection.database(), QUERY_INSERT_MEASUREMENT };

    insert.bind(1, m.stationId);
    insert.bind(2, ::systemClockToUnixTime(m.timestamp));
    for(int idx{ 3 }; const auto& opt : { m.diesel, m.e5, m.e10 })
    {
        opt.has_value() ? insert.bind(idx, *opt) : insert.bind(idx);
        ++idx;
    }

    insert.exec();

    transaction.commit();
}

void SQLiteFuelRepository::storeStation([[maybe_unused]] const Station& s)
{
    FUL_ASSERT(m_connection.isOpen());
    FUL_ASSERT(m_connection.mode() == SQLiteConnection::OpenMode::ReadWrite);
}

std::vector<Station> SQLiteFuelRepository::loadStations()
{
    FUL_ASSERT(m_connection.isOpen());

    return {};
}

std::vector<Measurement> SQLiteFuelRepository::loadMeasurements(TimePoint from, TimePoint to)
{
    FUL_ASSERT(m_connection.isOpen());

    SQLite::Statement query{ m_connection.database(), QUERY_MEASUREMENT_TABLE_FROM_TO };

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

void SQLiteFuelRepository::ensureTableLayout() const
{
    if(!m_connection.database().tableExists(MEASUREMENT_TABLE))
        m_connection.database().exec(CREATE_MEASUREMENT_TABLE_STATEMENT);
}

} // namespace ful::fuel
