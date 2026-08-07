#include <gtest/gtest.h>

#include "testUtility/TemporaryFile.hpp"
#include "utility/file/FileIO.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

namespace ful::testing
{

namespace
{

struct ByteWritable
{
    std::vector<std::byte> data;

    [[nodiscard]] std::span<const std::byte> toByteArray() const { return data; }
};

struct CharWritable
{
    std::string data;

    [[nodiscard]] std::string_view toByteArray() const { return data; }
};

} // namespace

/// \brief Attempting to read from a file that does not exist should throw an exception.
TEST(FileIOReadTest, ThrowsWhenFileDoesNotExist)
{
    EXPECT_THROW(std::ignore = file::readFromFile("definetly-does-not-exist.txt"), std::runtime_error);
}

/// \brief Test that reading an existing file does return the exact same content that is written in the file.
TEST(FileIOReadTest, ReturnsContentOfExistingFile)
{
    TemporaryFile tempFile{ std::filesystem::path("definetly-does-not-exist.txt") };
    constexpr auto TEST_FILE_CONTENT{ "line one\nline two\n" };

    {
        std::ofstream out{ tempFile.path() };

        out << TEST_FILE_CONTENT;
    }

    EXPECT_EQ(file::readFromFile(tempFile.path()), TEST_FILE_CONTENT);
}

} // namespace ful::testing
