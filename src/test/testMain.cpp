#include "gtest/gtest.h"

#include <filesystem>

namespace
{

void ensureTestResourcesDirecetoryExists()
{
    if(!std::filesystem::exists(TEST_RESOURCE_DIR))
        std::filesystem::create_directory(TEST_RESOURCE_DIR);
}

} // namespace

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    ::ensureTestResourcesDirecetoryExists();

    return RUN_ALL_TESTS();
}
