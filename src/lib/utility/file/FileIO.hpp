#ifndef FUL_SRC_LIB_UTILITY_FILE_FILE_IO_HPP
#define FUL_SRC_LIB_UTILITY_FILE_FILE_IO_HPP

#include <filesystem>
#include <string>

namespace ful::file
{

/// \brief Read the content of a file into a string.
///
/// \param filepath The location of the file on the disk
///
/// \returns A \ref std::string containing the content of the file at \p filepath
///
/// \throws A \ref std::runtime_error if the file does not exist
[[nodiscard]] std::string readFromFile(const std::filesystem::path& filepath);

} // namespace ful::file

#endif // !FUL_SRC_LIB_UTILITY_FILE_FILE_IO_HPP
