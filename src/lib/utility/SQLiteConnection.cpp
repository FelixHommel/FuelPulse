#include "SQLiteConnection.hpp"

#include <SQLiteCpp/Database.h>

#include <filesystem>
#include <memory>
#include <stdexcept>

namespace ful
{

SQLiteConnection::SQLiteConnection(const std::filesystem::path& databasePath, OpenMode mode)
    : m_database{
        std::make_unique<SQLite::Database>(databasePath.string(), SQLiteConnection::openModeToSQLiteFlag(mode))
    }
    , m_mode{ mode }
{}

bool SQLiteConnection::isOpen() const noexcept
{
    return static_cast<bool>(m_database);
}

SQLiteConnection::OpenMode SQLiteConnection::mode() const
{
    if(!m_database)
        throw std::logic_error("Database connection is not open");

    return m_mode;
}

SQLite::Database& SQLiteConnection::database()
{
    if(!m_database)
        throw std::logic_error("Database connection is not open");

    return *m_database;
}

SQLite::Database& SQLiteConnection::database() const
{
    if(!m_database)
        throw std::logic_error("Database connection is not open");

    return *m_database;
}

void SQLiteConnection::open(const std::filesystem::path& databasePath, OpenMode mode)
{
    m_database
        = std::make_unique<SQLite::Database>(databasePath.string(), SQLiteConnection::openModeToSQLiteFlag(mode));
    m_mode = mode;
}

void SQLiteConnection::close()
{
    m_database.reset();
}

} // namespace ful
