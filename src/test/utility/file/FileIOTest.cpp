#include <gtest/gtest.h>

#include "testUtility/TemporaryFile.hpp"
#include "utility/file/FileIO.hpp"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <ios>
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

[[nodiscard]] std::filesystem::path makeFileName(const std::string& fileSuffix)
{
    return { std::string(::testing::UnitTest::GetInstance()->current_test_info()->test_suite_name()) + fileSuffix };
}

} // namespace

/// \brief Attempting to read from a file that does not exist should throw an exception.
TEST(FileIOReadTest, ThrowsWhenFileDoesNotExist)
{
    EXPECT_THROW(std::ignore = file::readFromFile("definetly-does-not-exist.txt"), std::runtime_error);
}

/// \brief Test that reading an existing file does return the exact same content that is written in the file.
TEST(FileIOReadTest, ReturnsContentOfExistingFile)
{
    const TemporaryFile tempFile{ makeFileName("_content.txt") };
    constexpr auto TEST_FILE_CONTENT{ "line one\nline two\n" };

    {
        std::ofstream out{ tempFile.path() };

        out << TEST_FILE_CONTENT;
    }

    EXPECT_EQ(file::readFromFile(tempFile.path()), TEST_FILE_CONTENT);
}

/// \brief Test that reading an empty file returns an empty string.
TEST(FileIOReadTest, ReturnsEmptyStringForEmptyFile)
{
    const TemporaryFile tempFile{ makeFileName("_empty.txt") };

    EXPECT_TRUE(file::readFromFile(tempFile.path()).empty());
}

/// \brief Test that writing char-sourced \ref Writable data and reading it back gives the exact same value.
TEST(FileIOWriteTest, RoundTripsCharBasedWritable)
{
    const TemporaryFile tempFile{ makeFileName("_char.txt") };
    constexpr auto TEST_FILE_CONTENT{ "hello test" };

    file::writeToFile(tempFile.path(), CharWritable{ .data = TEST_FILE_CONTENT });

    EXPECT_EQ(file::readFromFile(tempFile.path()), TEST_FILE_CONTENT);
}

/// \brief Test that writing byte-sourced \ref Writable data and reading it back gives the exact same value.
TEST(FileIOWriteTest, RoundTripsByteBasedWritable)
{
    const TemporaryFile tempFile{ makeFileName("_bytes.txt") };
    const std::vector<std::byte> TEST_FILE_CONTENT{ std::byte{ 'h' }, std::byte{ 'i' } };

    file::writeToFile(tempFile.path(), ByteWritable{ .data = TEST_FILE_CONTENT });

    EXPECT_EQ(file::readFromFile(tempFile.path()), "hi");
}

/// \brief Test that writing to a file that already has content fully overwrites the preexisting content.
TEST(FileIOWriteTest, OverwritesExistingFileContent)
{
    const TemporaryFile tempFile{ makeFileName("_overwrite.txt") };
    constexpr auto ORIGINAL_CONTENT{ "original content" };
    constexpr auto CHANGED_CONTENT{ "changed content" };

    file::writeToFile(tempFile.path(), CharWritable{ .data = ORIGINAL_CONTENT });
    file::writeToFile(tempFile.path(), CharWritable{ .data = CHANGED_CONTENT });

    EXPECT_EQ(file::readFromFile(tempFile.path()), CHANGED_CONTENT);
}

/// \brief Test that writing zero bytes to a file leaves the file completely empty.
TEST(FileIOWriteTest, WiritingEmptyDataCreatesEmptyFile)
{
    const TemporaryFile tempFile{ makeFileName("_emptyData.txt") };

    file::writeToFile(tempFile.path(), CharWritable{ .data = "" });

    EXPECT_TRUE(file::readFromFile(tempFile.path()).empty());
}

/// \brief Test that attempting to write to a file whose parent directory does not exist throws instead of failing silently.
TEST(FileIOWriteTest, ThrowsWhenParentDirectoryDoesNotExist)
{
    const auto PATH{ TEST_RESOURCE_DIR / std::filesystem::path("no-such-directory") / "file.txt" };

    EXPECT_THROW(file::writeToFile(PATH, CharWritable{ .data = "data" }), std::ios_base::failure);
}

} // namespace ful::testing
