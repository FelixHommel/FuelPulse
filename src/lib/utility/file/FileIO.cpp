#include "FileIO.hpp"

#include <cstddef>
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>

namespace ful::file
{

namespace detail
{

void writeToFileImpl(const std::filesystem::path& filepath, std::span<const std::byte> data)
{
    std::ofstream out{ filepath };
    if(!out)
        throw std::ios_base::failure("Failure to open file");

    out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
}

} // namespace detail

std::string readFromFile(const std::filesystem::path& filepath)
{
    if(!std::filesystem::exists(filepath))
        throw std::runtime_error(std::format("There is no file at the following location: {}", filepath.string()));

    std::ifstream file{ filepath };

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

} // namespace ful::file
