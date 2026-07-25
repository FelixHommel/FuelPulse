#include "fuel/SQLiteFuelRepository.hpp"
#include "fuel/Domain.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>

namespace
{

inline const auto DB_PATH{ std::filesystem::path(TEST_RESOURCE_DIR) / "SQLiteFuelRepositoryTest.db3" };

} // namespace

namespace ful::testing
{

TEST(TEMP, TEMP)
{
    fuel::SQLiteFuelRepository repo{ ::DB_PATH };

    const auto t{ std::chrono::system_clock::now() };
    fuel::Measurement m{ .stationId = "123", .timestamp = t, .e5 = 123, .e10 = 123, .diesel = 123 };

    repo.store(m);

    auto x = repo.loadMeasurements(t - std::chrono::milliseconds(100), t + std::chrono::milliseconds(100));

    ASSERT_EQ(x.size(), 1);
}

} // namespace ful::testing
