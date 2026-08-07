#include "FileIO.hpp"

#include <filesystem>
#include <format>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace ful::file
{

std::string readFromFile(const std::filesystem::path& filepath)
{
    if(!std::filesystem::exists(filepath))
        throw std::runtime_error(std::format("There is no file at the following location: {}", filepath.string()));

    std::ifstream file{ filepath };

    std::stringstream buffer;
    buffer << file.rdbuf();

    return buffer.str();
}

void writeToFileImpl(const std::filesystem::path& filepath, const char* data, std::streamsize size)
{
    std::ofstream out{ filepath };
    if(!out)
        throw std::ios_base::failure("Failure to open file");

    out.write(data, size);

    if(!out)
        throw std::ios_base::failure("Failure to write file");
}

} // namespace ful::file
