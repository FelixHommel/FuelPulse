#include "fuel/SQLiteFuelRepository.hpp"
#include "fuel/Domain.hpp"
#include "fuel/IFuelRepository.hpp"
#include "testUtility/RandomNumberGenerator.hpp"
#include "utility/sqlite/SQLiteConnection.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <exception>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

namespace
{

using ful::fuel::Measurement;
using ful::fuel::PriceCents;
using ful::fuel::Station;
using TimePoint = ful::fuel::IFuelRepository::TimePoint;

constexpr auto IN_MEMORY_DB{ ":memory:" };

TimePoint truncateToMillis(TimePoint tp)
{
    return TimePoint{ std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()) };
}

Station makeStation(std::string id = "station-1")
{
    // NOLINTNEXTLINE(readability-magic-numbers): Long, Lat for Stuttgart, Germany
    return { .id = std::move(id), .name = "Aral", .brand = "Aral", .longitude = 9.1777, .latitude = 48.7758 };
}

Measurement makeMeasurement(
    std::string stationId,
    TimePoint timestamp,
    std::optional<PriceCents> e5 = std::make_optional(ful::testing::generateRandomValue<PriceCents>()),
    std::optional<PriceCents> e10 = std::make_optional(ful::testing::generateRandomValue<PriceCents>()),
    std::optional<PriceCents> diesel = std::make_optional(ful::testing::generateRandomValue<PriceCents>())
)
{
    return { .stationId = std::move(stationId), .timestamp = timestamp, .e5 = e5, .e10 = e10, .diesel = diesel };
}

Measurement makeNoPriceMeasurement(std::string stationId, TimePoint timestamp)
{
    return ::makeMeasurement(std::move(stationId), timestamp, std::nullopt, std::nullopt, std::nullopt);
}

} // namespace

namespace ful::testing
{

/// \brief Test that two \ref SQLiteFuelRepository can access the same database after each other without there being
///     any issues.
TEST(SQLiteFuelRepositorySchemaReuseTest, SecondRepositoryOverSameFileReusesExistingSchema)
{
    const auto dbPath{ TEST_RESOURCE_DIR / std::filesystem::path("fuelPulseSchemaReuseTest.db3") };

    std::error_code ec;
    std::filesystem::remove(dbPath, ec);

    {
        fuel::SQLiteFuelRepository first{ dbPath, SQLiteConnection::OpenMode::ReadWrite };
    }

    EXPECT_NO_THROW(fuel::SQLiteFuelRepository second{ dbPath });

    std::filesystem::remove(dbPath, ec);
}

/// \brief Test the features of \ref SQLiteFuelRepository.
///
/// \author Felix Hommel
/// \date 7/26/2026
class SQLiteFuelRepositoryTest : public ::testing::Test
{
public:
    SQLiteFuelRepositoryTest() = default;
    ~SQLiteFuelRepositoryTest() override = default;

    SQLiteFuelRepositoryTest(const SQLiteFuelRepositoryTest&) = delete;
    SQLiteFuelRepositoryTest& operator=(const SQLiteFuelRepositoryTest&) = delete;
    SQLiteFuelRepositoryTest(SQLiteFuelRepositoryTest&&) = delete;
    SQLiteFuelRepositoryTest& operator=(SQLiteFuelRepositoryTest&&) = delete;

    void SetUp() override { m_repository = std::make_unique<fuel::SQLiteFuelRepository>(::IN_MEMORY_DB); }

protected:
    std::unique_ptr<fuel::SQLiteFuelRepository> m_repository;
};

using SQLiteFuelRepositoryConstructionTest = SQLiteFuelRepositoryTest;

/// \brief Test that a freshly constructed \ref SQLiteFuelRepository with a new database has no entries.
TEST_F(SQLiteFuelRepositoryConstructionTest, ConstructingOverFreshDatabaseCreatesEmptySchema)
{
    EXPECT_TRUE(m_repository->loadStations().empty());
    EXPECT_TRUE(m_repository->loadMeasurements(TimePoint::min(), TimePoint::max()).empty());
}

using SQLiteFuelRepositoryStationTest = SQLiteFuelRepositoryTest;

/// \brief Test that a single \ref Station can be stored and retrieved afterward.
TEST_F(SQLiteFuelRepositoryStationTest, StoreSingleStationCanBeLoadedBack)
{
    const auto station{ ::makeStation() };

    m_repository->storeStation(station);
    const auto loaded{ m_repository->loadStations() };

    ASSERT_EQ(loaded.front(), station);
}

/// \brief Test that multiple \ref Station objects can be stored and retrieved afterward.
TEST_F(SQLiteFuelRepositoryStationTest, StoreMultipleStationsAllLoadedBack)
{
    const std::vector<Station> stations{
        { ::makeStation("station-1"), ::makeStation("station-2"), ::makeStation("station-3") }
    };

    for(const auto& s : stations)
        m_repository->storeStation(s);

    for(auto original{ stations.begin() }; const auto& s : m_repository->loadStations())
        EXPECT_EQ(s, *(original++));
}

using SQLiteFuelRepositoryMeasurementTest = SQLiteFuelRepositoryTest;

/// \brief Test that a single stored \ref Measurement can be retrieved back without modifications.
TEST_F(SQLiteFuelRepositoryMeasurementTest, StoreSingleMeasurementCanBeLoadedBack)
{
    const auto timestamp{ ::truncateToMillis(std::chrono::system_clock::now()) };
    const auto measurement{ ::makeMeasurement("station-1", timestamp) };

    m_repository->store(measurement);

    const auto loaded{
        m_repository->loadMeasurements(timestamp - std::chrono::seconds(1), timestamp + std::chrono::seconds(1))
    };

    ASSERT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded.front(), measurement);
}

/// \brief Test that a \ref Measurement that has no price data is stored and retrieved with only empty
///     \ref std::optional for prices.
TEST_F(SQLiteFuelRepositoryMeasurementTest, StoreMeasurementWithMissingPricesPreservesNullopt)
{
    const auto timestamp{ ::truncateToMillis(std::chrono::system_clock::now()) };
    const auto measurement{ ::makeNoPriceMeasurement("station-1", timestamp) };

    m_repository->store(measurement);

    const auto loaded{
        m_repository->loadMeasurements(timestamp - std::chrono::seconds(1), timestamp + std::chrono::seconds(1))
    };

    ASSERT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded.front(), measurement);
}

/// \brief Test that a \ref Measurement that only has partial price information is stored correctly and retrieved back
///     with the same price information.
TEST_F(SQLiteFuelRepositoryMeasurementTest, StoreMeasurementWithPartialPricesPreservesEachField)
{
    const auto timestamp{ ::truncateToMillis(std::chrono::system_clock::now()) };
    const auto measurement{ ::makeMeasurement(
        "station-1", timestamp, std::make_optional(generateRandomValue<PriceCents>()), std::nullopt, std::nullopt
    ) };

    m_repository->store(measurement);

    const auto loaded{
        m_repository->loadMeasurements(timestamp - std::chrono::seconds(1), timestamp + std::chrono::seconds(1))
    };

    ASSERT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded.front(), measurement);
}

/// \brief Test that only \ref Measurement that are within the specified time frame are retrieved back from the Database.
TEST_F(SQLiteFuelRepositoryMeasurementTest, LoadMeasurementsExcludesEntriesOutsideRange)
{
    const auto timestamp{ ::truncateToMillis(std::chrono::system_clock::now()) };

    m_repository->store(::makeMeasurement("station-1", timestamp - std::chrono::hours(1)));
    m_repository->store(::makeMeasurement("station-1", timestamp));
    m_repository->store(::makeMeasurement("station-1", timestamp + std::chrono::hours(1)));

    const auto loaded{
        m_repository->loadMeasurements(timestamp - std::chrono::seconds(1), timestamp + std::chrono::seconds(1))
    };

    ASSERT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded.front().timestamp, timestamp);
}

/// \brief Test that the loading operation is including the time bounds of the measurements.
TEST_F(SQLiteFuelRepositoryMeasurementTest, LoadMeasurementsRangeBoundsAreInclusive)
{
    const auto from{ ::truncateToMillis(std::chrono::system_clock::now()) };
    const auto to{ from + std::chrono::hours(1) };

    m_repository->store(::makeMeasurement("station-1", from));
    m_repository->store(::makeMeasurement("station-1", to));

    const auto loaded{ m_repository->loadMeasurements(from, to) };

    ASSERT_EQ(loaded.size(), 2);
}

/// \brief Test that measurements from several different stations are all returned when they fall within the requested
///     range.
TEST_F(SQLiteFuelRepositoryMeasurementTest, LoadMeasurementsReturnsEntriesFromAllStations)
{
    const auto timestamp{ ::truncateToMillis(std::chrono::system_clock::now()) };

    m_repository->store(::makeMeasurement("station-1", timestamp));
    m_repository->store(::makeMeasurement("station-2", timestamp));

    const auto loaded{
        m_repository->loadMeasurements(timestamp - std::chrono::seconds(1), timestamp + std::chrono::seconds(1))
    };

    ASSERT_EQ(loaded.size(), 2);
}

/// \brief Test that a query range that has no matches returns an empty vector.
TEST_F(SQLiteFuelRepositoryMeasurementTest, LoadMeasurementsWithNoMatchingRangeReturnsEmpty)
{
    const auto timestamp{ ::truncateToMillis(std::chrono::system_clock::now()) };

    m_repository->store(::makeMeasurement("station-1", timestamp));

    EXPECT_TRUE(
        m_repository->loadMeasurements(timestamp + std::chrono::seconds(1), timestamp + std::chrono::seconds(2)).empty()
    );
}

/// \brief Test fixture using a file on disk to test the \ref SQLiteConnection::OpenMode::ReadOnly because an in-memory
///     database can't be used in read-only mode.
///
/// \author Felix Hommel
/// \date 7/27/2026
class SQLiteFuelRepositoryReadOnlyTest : public ::testing::Test
{
public:
    SQLiteFuelRepositoryReadOnlyTest() = default;
    ~SQLiteFuelRepositoryReadOnlyTest() override
    {
        std::error_code ec;
        std::filesystem::remove(m_dbPath, ec);
    }

    SQLiteFuelRepositoryReadOnlyTest(const SQLiteFuelRepositoryReadOnlyTest&) = delete;
    SQLiteFuelRepositoryReadOnlyTest& operator=(const SQLiteFuelRepositoryReadOnlyTest&) = delete;
    SQLiteFuelRepositoryReadOnlyTest(SQLiteFuelRepositoryReadOnlyTest&&) = delete;
    SQLiteFuelRepositoryReadOnlyTest& operator=(SQLiteFuelRepositoryReadOnlyTest&&) = delete;

    void SetUp() override
    {
        m_dbPath = std::filesystem::path(TEST_RESOURCE_DIR)
                 / (std::string(::testing::UnitTest::GetInstance()->current_test_info()->name()) + ".db3");

        std::error_code ec;
        std::filesystem::remove(m_dbPath, ec);

        fuel::SQLiteFuelRepository writer{ m_dbPath, SQLiteConnection::OpenMode::ReadWrite };
        writer.storeStation(::makeStation());
    }

protected:
    std::filesystem::path m_dbPath;
};

/// \brief Test that a read-only \ref SQLiteFuelRepository can read values from the database.
TEST_F(SQLiteFuelRepositoryReadOnlyTest, ReadOnlyRepositoryCanLoadExistingStations)
{
    fuel::SQLiteFuelRepository readOnly{ m_dbPath, SQLiteConnection::OpenMode::ReadOnly };

    const auto loaded{ readOnly.loadStations() };

    ASSERT_EQ(loaded.size(), 1);
    EXPECT_EQ(loaded.front().id, ::makeStation().id);
}

/// \brief Test that a read-only \ref SQLiteFuelRepository fails assertions when trying to write something.
TEST_F(SQLiteFuelRepositoryReadOnlyTest, ReadOnlyRepositoryCantStoreAnything)
{
    fuel::SQLiteFuelRepository readOnly{ m_dbPath, SQLiteConnection::OpenMode::ReadOnly };

    EXPECT_THROW(
        readOnly.store(::makeMeasurement("station-1", ::truncateToMillis(std::chrono::system_clock::now()))),
        std::exception
    );
    EXPECT_THROW(readOnly.storeStation(::makeStation()), std::exception);
}

} // namespace ful::testing
