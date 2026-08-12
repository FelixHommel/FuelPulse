#include "utility/sqlite/SQLiteConnection.hpp"
#include "utility/exception/SQLiteConnectionException.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <SQLiteCpp/Database.h>

#include <filesystem>
#include <tuple>

namespace
{

constexpr auto DB_IN_MEMORY_PATH{ ":memory:" };

} // namespace

namespace ful::testing
{

/// \brief When a new \ref SQLiteConnection object is created without providing any parameters, the connection should be
///     in a closed state.
TEST(SQLiteConnectionConstructionTest, CreateConnectionObjectWithoutOpeningConnection)
{
    SQLiteConnection conn{};

    EXPECT_FALSE(conn.isOpen());
    EXPECT_THAT(
        [&conn] { std::ignore = conn.mode(); },
        ::testing::ThrowsMessage<SQLiteConnectionException>(::testing::HasSubstr("not open"))
    );
    EXPECT_THAT(
        [&conn] { std::ignore = conn.database(); },
        ::testing::ThrowsMessage<SQLiteConnectionException>(::testing::HasSubstr("not open"))
    );
}

/// \brief When a new \ref SQLiteConnection object is created and provided with a path to the database, the connection
///     should be established at construction.
TEST(SQLiteConnectionConstructionTest, CreateConnectionObjectWithOpeningConnection)
{
    SQLiteConnection conn{ ::DB_IN_MEMORY_PATH };

    EXPECT_TRUE(conn.isOpen());
    EXPECT_EQ(conn.mode(), SQLiteConnection::OpenMode::ReadWrite);
    EXPECT_NO_THROW(std::ignore = conn.database());
}

/// \brief When a new \ref SQLiteConnection object is created a provided with a path to the database as well as a
///     specific \ref SQLiteConnection::OpenMode, the specified mode should be remembered.
TEST(SQLiteConnectionConstructionTest, CreateConnectionWithSpecifyingOpenMode)
{
    SQLiteConnection conn{ ::DB_IN_MEMORY_PATH, SQLiteConnection::OpenMode::ReadOnly };

    EXPECT_TRUE(conn.isOpen());
    EXPECT_EQ(conn.mode(), SQLiteConnection::OpenMode::ReadOnly);
    EXPECT_NO_THROW(std::ignore = conn.database());
}

/// \brief Test \ref SQLiteConnection::open and \ref SQLiteConnection::close.
///
/// \author Felix Hommel
/// \date 7/24/2026
class SQLiteConnectionTest : public ::testing::Test
{
public:
    SQLiteConnectionTest() = default;
    ~SQLiteConnectionTest() override = default;

    SQLiteConnectionTest(const SQLiteConnectionTest&) = delete;
    SQLiteConnectionTest& operator=(const SQLiteConnectionTest&) = delete;
    SQLiteConnectionTest(SQLiteConnectionTest&&) = delete;
    SQLiteConnectionTest& operator=(SQLiteConnectionTest&&) = delete;

protected:
    SQLiteConnection m_connection;
};

/// \brief Test that \ref SQLiteConnection::open does open a connection to the database.
TEST_F(SQLiteConnectionTest, OpenConnectionToDatabase)
{
    ASSERT_FALSE(m_connection.isOpen());

    m_connection.open(::DB_IN_MEMORY_PATH);

    EXPECT_TRUE(m_connection.isOpen());
}

/// \brief Test that \ref SQLiteConnection::open throws when trying to connect to a database at an invalid location.
TEST_F(SQLiteConnectionTest, TryOpenConnectionAtInvalidLocation)
{
    ASSERT_FALSE(m_connection.isOpen());

    EXPECT_THAT(
        [this] { m_connection.open("non-existing-path/test.db3"); },
        ::testing::ThrowsMessage<SQLiteConnectionException>(::testing::HasSubstr("non-existing-path/test.db3"))
    );

    EXPECT_FALSE(m_connection.isOpen());
}

/// \brief Test that \ref SQLiteConnection::close does close an open database connection.
TEST_F(SQLiteConnectionTest, CloseClosesDatabaseConnection)
{
    m_connection.open(::DB_IN_MEMORY_PATH);

    ASSERT_TRUE(m_connection.isOpen());

    m_connection.close();

    EXPECT_FALSE(m_connection.isOpen());
}

/// \brief Test that database access via a const-ref is valid when the source is a connected database.
TEST_F(SQLiteConnectionTest, ConstRefAccessOnConnectedDatabase)
{
    m_connection.open(::DB_IN_MEMORY_PATH);

    const auto& dbRef{ m_connection };

    EXPECT_NO_THROW(std::ignore = dbRef.database());
}

/// \brief Test that database access via a const-ref throws when the source is not a connected database.
TEST_F(SQLiteConnectionTest, ConstRefAccessOnDisconnectedDatabase)
{
    const auto& dbRef{ m_connection };

    EXPECT_THAT(
        [&dbRef] { std::ignore = dbRef.database(); },
        ::testing::ThrowsMessage<SQLiteConnectionException>(::testing::HasSubstr("not open"))
    );
}

/// \brief Test the \ref SQLiteConnection::OpenMode functionality.
///
/// \author Felix Hommel
/// \date 7/25/2026
class SQLiteConnectionOpenModeTest : public ::testing::Test
{
public:
    SQLiteConnectionOpenModeTest() = default;
    ~SQLiteConnectionOpenModeTest() override = default;

    SQLiteConnectionOpenModeTest(const SQLiteConnectionOpenModeTest&) = delete;
    SQLiteConnectionOpenModeTest& operator=(const SQLiteConnectionOpenModeTest&) = delete;
    SQLiteConnectionOpenModeTest(SQLiteConnectionOpenModeTest&&) = delete;
    SQLiteConnectionOpenModeTest& operator=(SQLiteConnectionOpenModeTest&&) = delete;

protected:
    [[nodiscard]] static int convertOpenMode(SQLiteConnection::OpenMode mode)
    {
        return SQLiteConnection::openModeToSQLiteFlag(mode);
    }
};

/// \brief Test that the ReadOnly open mode is converted correctly.
TEST_F(SQLiteConnectionOpenModeTest, OpenModeReadOnly)
{
    EXPECT_EQ(convertOpenMode(SQLiteConnection::OpenMode::ReadOnly), SQLite::OPEN_READONLY);
}

/// \brief Test that the ReadWrite open mode is converted correctly.
TEST_F(SQLiteConnectionOpenModeTest, OpenModeReadWrite)
{
    EXPECT_EQ(convertOpenMode(SQLiteConnection::OpenMode::ReadWrite), SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE);
}

} // namespace ful::testing
