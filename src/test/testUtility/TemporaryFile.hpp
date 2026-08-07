#ifndef FUL_SRC_TEST_TEST_UTILITY_TEMPORARY_FILE_HPP
#define FUL_SRC_TEST_TEST_UTILITY_TEMPORARY_FILE_HPP

#include <filesystem>
#include <fstream>

namespace ful::testing
{

/// \brief Simple RAII-wrapper that ensures the existence of a file for the duration of the objects existence.
///
/// \author Felix Hommel
/// \date 08/07/2026
class TemporaryFile
{
public:
    TemporaryFile(const std::filesystem::path& localFilePath) : m_filepath{ TEST_RESOURCE_DIR / localFilePath }
    {
        if(!std::filesystem::exists(m_filepath))
        {
            std::ofstream out{ m_filepath };
        }
    }

    ~TemporaryFile()
    {
        if(std::filesystem::exists(m_filepath))
            std::filesystem::remove(m_filepath);
    }

    TemporaryFile(const TemporaryFile&) = default;
    TemporaryFile& operator=(const TemporaryFile&) = default;
    TemporaryFile(TemporaryFile&&) = delete;
    TemporaryFile& operator=(TemporaryFile&&) = delete;

    [[nodiscard]] std::filesystem::path path() const noexcept { return m_filepath; }

private:
    std::filesystem::path m_filepath;
};

} // namespace ful::testing

#endif // !FUL_SRC_TEST_TEST_UTILITY_TEMPORARY_FILE_HPP
