#ifndef FUL_SRC_LIB_UTILITY_FILE_FILE_IO_HPP
#define FUL_SRC_LIB_UTILITY_FILE_FILE_IO_HPP

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <span>
#include <string>
#include <string_view>

namespace ful::file
{

namespace
{

/// \brief Describe a type that has the capability to output some data to as byte array.
template<typename T>
concept ByteWritable = requires(const T& t) {
    requires std::same_as<typename decltype(std::span{ t.toByteArray() })::element_type, const std::byte>;
};

} // namespace

namespace detail
{

/// \brief Write a byte array directly to the file at \p filepath.
///
/// \param filepath The path to the file where the data is written to
/// \param data A \ref std::span with the byte array data itself
void writeToFileImpl(const std::filesystem::path& filepath, std::span<const std::byte> data);

} // namespace detail

/// \brief Read the content of a file into a string.
///
/// \param filepath The location of the file on the disk
///
/// \returns A \ref std::string containing the content of the file at \p filepath
///
/// \throws A \ref std::runtime_error if the file does not exist
[[nodiscard]] std::string readFromFile(const std::filesystem::path& filepath);

/// \brief Output the data of \p T to a file.
///
/// \tparam T An object complying with \ref Writable
///
/// \param filepath The location where the \p obj is being output to
/// \param obj The object that is output to the file
template<ByteWritable T>
void writeToFile(const std::filesystem::path& filepath, const T& obj)
{
    const auto bytes{ obj.toByteArray() };

    detail::writeToFileImpl(filepath, std::span{ bytes });
}

/// \brief Output a string to a file.
///
/// \param filepath The location of the file on the disk
/// \param str A \ref std::string_view to the string that is supposed to be outputted
inline void writeToFile(const std::filesystem::path& filepath, std::string_view str)
{
    detail::writeToFileImpl(filepath, std::as_bytes(std::span{ str.data(), str.size() }));
}

} // namespace ful::file

#endif // !FUL_SRC_LIB_UTILITY_FILE_FILE_IO_HPP
