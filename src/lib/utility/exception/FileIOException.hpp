#ifndef FUL_SRC_LIB_UTILITY_EXCEPTION_FILE_IO_EXCEPTION_HPP
#define FUL_SRC_LIB_UTILITY_EXCEPTION_FILE_IO_EXCEPTION_HPP

#include "utility/exception/Exception.hpp"

#include <filesystem>
#include <source_location>
#include <string_view>

namespace ful
{

/// \brief Exception used for error cases involving file operations.
///
/// \author Felix Hommel
/// \date 8/8/2026
class FileIOException final : public Exception
{
public:
    ~FileIOException() override = default;

    FileIOException(const FileIOException&) = default;
    FileIOException& operator=(const FileIOException&) = default;
    FileIOException(FileIOException&&) = default;
    FileIOException& operator=(FileIOException&&) = default;

    /// \brief Create a new \ref FileIOException.
    ///
    /// \param filepath The \ref std::filesystem::path to the location of the file that caused the exception
    /// \param reason Provided reason for the exception
    /// \param loc (optional) The \ref std::source_location where the exception was caused
    ///
    /// \returns \ref The new \ref FileIOException
    [[nodiscard]] static FileIOException create(
        std::filesystem::path filepath,
        std::string_view reason,
        std::source_location loc = std::source_location::current()
    );

    /// \brief Get the path of the file that caused the exception.
    [[nodiscard]] const std::filesystem::path& path() const noexcept { return m_filepath; }

private:
    std::filesystem::path m_filepath;

    FileIOException(Exception exception, std::filesystem::path filepath);
};

} // namespace ful

#endif // !FUL_SRC_LIB_UTILITY_EXCEPTION_FILE_IO_EXCEPTION_HPP
