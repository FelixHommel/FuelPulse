#include "FileIOException.hpp"

#include "utility/exception/Exception.hpp"

#include <filesystem>
#include <format>
#include <source_location>
#include <string>
#include <string_view>
#include <utility>

namespace ful
{

FileIOException::FileIOException(Exception exception, std::filesystem::path filepath)
    : Exception{ std::move(exception) }, m_filepath{ std::move(filepath) }
{}

FileIOException FileIOException::create(
    std::filesystem::path filepath, std::string_view reason, std::source_location loc
)
{
    auto message{ std::format("File I/O failed for '{}': {}", filepath.string(), reason) };

    return FileIOException{
        Exception{ std::move(message), loc },
        std::move(filepath)
    };
}

} // namespace ful
