#include "FileIO.hpp"

#include "utility/exception/FileIOException.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <ios>
#include <span>
#include <sstream>
#include <string>

namespace ful::file
{

namespace detail
{

void writeToFileImpl(const std::filesystem::path& filepath, std::span<const std::byte> data)
{
    std::ofstream out{ filepath };
    if(!out)
        throw FileIOException::create(filepath, "Failure to open file");

    out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

} // namespace detail

std::string readFromFile(const std::filesystem::path& filepath)
{
    if(!std::filesystem::exists(filepath))
        throw FileIOException::create(filepath, "There is no file at the specified location");

    std::ifstream file{ filepath };

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

} // namespace ful::file
