#ifndef FUL_SRC_LIB_UTILITY_SQLITE_SQLITE_CONNECTION_HPP
#define FUL_SRC_LIB_UTILITY_SQLITE_SQLITE_CONNECTION_HPP

#include <SQLiteCpp/Database.h>

#include <cstdint>
#include <filesystem>
#include <memory>

namespace ful
{

namespace testing
{

class SQLiteConnectionOpenModeTest;

} // namespace testing

/// \brief Wrapper around \ref SQLite::Database to simplify connection management.
///
/// \author Felix Hommel
/// \date 7/24/2026
class SQLiteConnection
{
public:
    enum class OpenMode : std::uint8_t
    {
        ReadWrite,
        ReadOnly,
    };

    /// \brief Create a new \ref SQLiteConnection without connecting to a database.
    SQLiteConnection() = default;
    /// \brief Create a new \ref SQLiteConnection and connect it to a database.
    ///
    /// \param databasePath The location of the database on the disc
    /// \param mode (optional) How to open to the database
    SQLiteConnection(const std::filesystem::path& databasePath, OpenMode mode = OpenMode::ReadWrite);
    ~SQLiteConnection() = default;

    SQLiteConnection(const SQLiteConnection&) = delete;
    SQLiteConnection& operator=(const SQLiteConnection&) = delete;
    SQLiteConnection(SQLiteConnection&&) = default;
    SQLiteConnection& operator=(SQLiteConnection&&) = default;

    /// \brief Query whether there is a connection to a database or not.
    ///
    /// \returns \p true if there is an open connection, \p false if not
    [[nodiscard]] bool isOpen() const noexcept;
    /// \brief Query the \ref OpenMode that was used to open the database connection.
    ///
    /// \throws \ref std::logic_error if the \ref SQLiteConnection is closed
    [[nodiscard]] OpenMode mode() const;
    /// \brief Get the handle to the \ref SQLite::Database object
    ///
    /// \throws \ref std::logic_error if the \ref SQLiteConnection is closed
    [[nodiscard]] SQLite::Database& database();
    /// \brief Get the handle to the \ref SQLite::Database object
    ///
    /// \throws \ref std::logic_error if the \ref SQLiteConnection is closed
    [[nodiscard]] SQLite::Database& database() const;

    /// \brief Open a connection to a database.
    ///
    /// \param databasePath Path to the database on the disk
    /// \param mode (optional) How to open to the database
    void open(const std::filesystem::path& databasePath, OpenMode mode = OpenMode::ReadWrite);
    /// \brief Close a connection to a database.
    void close();

private:
    OpenMode m_mode{};
    std::unique_ptr<SQLite::Database> m_database;

    /// \brief Convert a \ref SQLiteConnection::OpenMode to the appropriate SQLite flags.
    ///
    /// \param mode The \ref SQLiteConnection::OpenMode that is converted
    ///
    /// \returns the flags to give to the database.
    [[nodiscard]] static int openModeToSQLiteFlag(OpenMode mode)
    {
        int flags{ 0 };

        switch(mode)
        {
            using enum OpenMode;

        case ReadWrite:
            flags |= SQLite::OPEN_READWRITE;
            flags |= SQLite::OPEN_CREATE;
            break;
        case ReadOnly:
            flags |= SQLite::OPEN_READONLY;
            break;
        default:
            break;
        }

        return flags;
    }

    friend class testing::SQLiteConnectionOpenModeTest;
};

} // namespace ful

#endif // !FUL_SRC_LIB_UTILITY_SQLITE_SQLITE_CONNECTION_HPP
