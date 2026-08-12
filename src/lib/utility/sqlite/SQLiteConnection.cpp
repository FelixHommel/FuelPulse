#include "SQLiteConnection.hpp"

#include "utility/exception/SQLiteConnectionException.hpp"

#include <SQLiteCpp/Database.h>
#include <SQLiteCpp/Exception.h>

#include <filesystem>
#include <memory>

namespace ful
{

// clang-format off: For some reason clang-format was unhappy about the member initializer list
SQLiteConnection::SQLiteConnection(const std::filesystem::path& databasePath, OpenMode mode)
    : m_database{
        std::make_unique<SQLite::Database>(databasePath.string(), SQLiteConnection::openModeToSQLiteFlag(mode))
    }
    , m_mode{ mode }
{}
// clang-format on

bool SQLiteConnection::isOpen() const noexcept
{
    return static_cast<bool>(m_database);
}

SQLiteConnection::OpenMode SQLiteConnection::mode() const
{
    if(!m_database)
        throw SQLiteConnectionException::create("Database connection is not open");

    return m_mode;
}

SQLite::Database& SQLiteConnection::database()
{
    if(!m_database)
        throw SQLiteConnectionException::create("Database connection is not open");

    return *m_database;
}

SQLite::Database& SQLiteConnection::database() const
{
    if(!m_database)
        throw SQLiteConnectionException::create("Database connection is not open");

    return *m_database;
}

void SQLiteConnection::open(const std::filesystem::path& databasePath, OpenMode mode)
{
    try
    {
        m_database
            = std::make_unique<SQLite::Database>(databasePath.string(), SQLiteConnection::openModeToSQLiteFlag(mode));
        m_mode = mode;
    }
    catch(const SQLite::Exception& e)
    {
        throw SQLiteConnectionException::create(e.what(), databasePath);
    }
}

void SQLiteConnection::close()
{
    m_database.reset();
}

} // namespace ful
