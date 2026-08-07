#ifndef FUL_SRC_LIB_UTILITY_FILE_FILE_IO_HPP
#define FUL_SRC_LIB_UTILITY_FILE_FILE_IO_HPP

#include <concepts>
#include <cstddef>
#include <filesystem>
#include <ios>
#include <string>

namespace ful::file
{

namespace
{

/// \brief Describe a type that has the capability to output some data to as byte array.
template<typename T>
concept Writable = requires(const T& t) {
    { t.toByteArray().data() } -> std::same_as<const std::byte*>;
    { t.toByteArray().size() } -> std::convertible_to<std::streamsize>;
} || requires(const T& t) {
    { t.toByteArray().data() } -> std::same_as<const char*>;
    { t.toByteArray().size() } -> std::convertible_to<std::streamsize>;
};

} // namespace

/// \brief Read the content of a file into a string.
///
/// \param filepath The location of the file on the disk
///
/// \returns A \ref std::string containing the content of the file at \p filepath
///
/// \throws A \ref std::runtime_error if the file does not exist
[[nodiscard]] std::string readFromFile(const std::filesystem::path& filepath);

void writeToFileImpl(const std::filesystem::path& filepath, const char* data, std::streamsize size);

/// \brief Output a string to a file.
///
/// \param filepath The location where the \p obj is being output to
/// \param obj The \ref std::string that is being outputted
// void writeToFile(const std::filesystem::path& filepath, const std::string& obj)
// {
//     writeToFileImpl(filepath, obj.c_str(), static_cast<std::streamsize>(obj.size()));
// }

/// \brief Output the data of \p T to a file.
///
/// \tparam T An object complying with \ref Writable
///
/// \param filepath The location where the \p obj is being output to
/// \param obj The object that is output to the file
template<Writable T>
void writeToFile(const std::filesystem::path& filepath, const T& obj)
{
    const auto bytes{ obj.toByteArray() };

    writeToFileImpl(filepath, reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

} // namespace ful::file

#endif // !FUL_SRC_LIB_UTILITY_FILE_FILE_IO_HPP
