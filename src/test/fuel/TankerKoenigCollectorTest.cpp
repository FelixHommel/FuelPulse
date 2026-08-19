#include "fuel/TankerKoenigCollector.hpp"
#include "core/EventBus.hpp"
#include "fuel/Domain.hpp"
#include "fuel/SQLiteFuelRepository.hpp"
#include "testUtility/ConcurrencyTest.hpp"
#include "testUtility/DummyTankerKoenigGateways.hpp"
#include <utility/threading/Threading.hpp>

#include <gtest/gtest.h>

#include <chrono>
#include <thread>
#include <vector>

namespace ful::testing
{

namespace
{

using TimePoint = std::chrono::system_clock::time_point;

constexpr auto TEST_API_KEY{ "test-api-key" };
constexpr auto TEST_POSTAL_CODE{ 70190u };
constexpr auto MEMORY_DB{ ":memory:" };

constexpr auto SHORT_SETTLE_DELAY{ std::chrono::milliseconds(50) };
constexpr auto RELEASE_DELAY{ std::chrono::milliseconds(50) };

constexpr auto WIDE_LOWER_BOUND{ TimePoint{} - std::chrono::hours(24 * 365 * 100) };
constexpr auto WIDE_UPPER_BOUND{ TimePoint{} + std::chrono::hours(24 * 365 * 100) };

} // namespace

/// \brief Base fixture providing a fresh in-memory repository and event bus for each test.
///
/// \author Felix Hommel
/// \date 8/18/2026
class TankerKoenigCollectorTest : public ::testing::Test
{
public:
    TankerKoenigCollectorTest() = default;
    ~TankerKoenigCollectorTest() override = default;

    TankerKoenigCollectorTest(const TankerKoenigCollectorTest&) = delete;
    TankerKoenigCollectorTest& operator=(const TankerKoenigCollectorTest&) = delete;
    TankerKoenigCollectorTest(TankerKoenigCollectorTest&&) = delete;
    TankerKoenigCollectorTest& operator=(TankerKoenigCollectorTest&&) = delete;

protected:
    fuel::SQLiteFuelRepository m_repo{ MEMORY_DB };
    EventBus m_bus;

    [[nodiscard]] std::vector<fuel::Measurement> getRepoContent()
    {
        return m_repo.loadMeasurements(WIDE_LOWER_BOUND, WIDE_UPPER_BOUND);
    }
};

/// \brief Test that a successful fetch is parsed and persisted to the repository.
TEST_F(TankerKoenigCollectorTest, CollectStoresParsedMeasurementsInRepository)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Technically not unchecked

    fuel::TankerKoenigCollector<ImmediateGateway> collector{ TEST_API_KEY, TEST_POSTAL_CODE, m_repo, m_bus };

    collector.collect();

    ASSERT_TRUE(waitOn([this] { return !getRepoContent().empty(); }));

    const auto measurements{ getRepoContent() };

    ASSERT_EQ(measurements.size(), 1);
    EXPECT_EQ(measurements.front().stationId, "83d5ac80-4f23-4106-b054-7c7704bfcb95");

    ASSERT_TRUE(measurements.front().e5.has_value());
    EXPECT_EQ(*measurements.front().e5, 1399u);

    // NOLINTEND(bugprone-unchecked-optional-access)
}

using TankerKoenigCollectorConcurrencyTest = TankerKoenigCollectorTest;

/// \brief Test that a second \ref TankerKoenigCollector::collect() call while a fetch is already in flight is skipped
///     rather than starting a second fetch.
TEST_F(TankerKoenigCollectorConcurrencyTest, SecondCollectWhileFirstStillFetchingIsSkipped)
{
    BlockingCountingGateway gateway;
    fuel::TankerKoenigCollector<BlockingCountingGateway> collector{
        TEST_API_KEY, TEST_POSTAL_CODE, m_repo, m_bus, gateway
    };

    collector.collect();
    gateway.waitUntilFetchStarted();

    collector.collect();
    gateway.release();

    ASSERT_TRUE(waitOn([&gateway] { return gateway.callCount() >= 1; }));

    std::this_thread::sleep_for(SHORT_SETTLE_DELAY);
    EXPECT_EQ(gateway.callCount(), 1);
}

/// \brief Test that a \ref TankerKoenigCollector::collect() call after a previous fetch has fully completed does
///     trigger a second fetch.
TEST_F(TankerKoenigCollectorConcurrencyTest, CollectAfterPreviousFetchCompletesStartsNewFetch)
{
    BlockingCountingGateway gateway;
    fuel::TankerKoenigCollector<BlockingCountingGateway> collector{
        TEST_API_KEY, TEST_POSTAL_CODE, m_repo, m_bus, gateway
    };

    collector.collect();
    gateway.waitUntilFetchStarted();
    gateway.release();
    ASSERT_TRUE(waitOn([&gateway] { return gateway.callCount() == 1; }));

    // NOTE: This needs a sleep so that previous collect has enough time to release the semaphore, otherwise test fails
    //  but still flakey in this implementation.
    std::this_thread::sleep_for(SHORT_SETTLE_DELAY);

    collector.collect();
    gateway.waitUntilFetchStarted();
    gateway.release();

    EXPECT_TRUE(waitOn([&gateway] { return gateway.callCount() == 2; }));
}

using TankerKoenigCollectorExceptionSafetyTest = TankerKoenigCollectorTest;

/// \brief Test that a gateway throwing a \ref std::exception does not leave the internal semaphore held, so a
///     subsequent \ref TankerKoenigCollector::collect() call is still accepted rather than silently skipped.
TEST_F(TankerKoenigCollectorExceptionSafetyTest, GatewayThrowingStdExceptionReleasesGateForNextCollect)
{
    ThrowingStdExceptionGateway gateway;
    fuel::TankerKoenigCollector<ThrowingStdExceptionGateway> collector{
        TEST_API_KEY, TEST_POSTAL_CODE, m_repo, m_bus, gateway
    };

    EXPECT_NO_THROW(collector.collect());
    ASSERT_TRUE(waitOn([&gateway] { return gateway.callCount() == 1; }));

    std::this_thread::sleep_for(SHORT_SETTLE_DELAY);

    EXPECT_NO_THROW(collector.collect());
    ASSERT_TRUE(waitOn([&gateway] { return gateway.callCount() == 2; }));

    EXPECT_TRUE(getRepoContent().empty());
}

/// \brief Test that a gateway that returns invalid JSON is handled by the existing \ref std::exception path and
///     nothing gets stored.
TEST_F(TankerKoenigCollectorExceptionSafetyTest, MalformedJsonFromGatewayIsHandeledGracefully)
{
    fuel::TankerKoenigCollector<MalformedJsonGateway> collector{ TEST_API_KEY, TEST_POSTAL_CODE, m_repo, m_bus };

    EXPECT_NO_FATAL_FAILURE(collector.collect());

    std::this_thread::sleep_for(SHORT_SETTLE_DELAY);
    EXPECT_TRUE(getRepoContent().empty());
}

TEST_F(TankerKoenigCollectorExceptionSafetyTest, GatewayThrowingNonStdExceptionReleasesGateForNextCollect)
{
    ThrowingNonStdExceptionGateway gateway;
    fuel::TankerKoenigCollector<ThrowingNonStdExceptionGateway> collector{
        TEST_API_KEY, TEST_POSTAL_CODE, m_repo, m_bus, gateway
    };

    EXPECT_NO_THROW(collector.collect());
    ASSERT_TRUE(waitOn([&gateway] { return gateway.callCount() == 1; }));

    EXPECT_NO_THROW(collector.collect());
    ASSERT_TRUE(waitOn([&gateway] { return gateway.callCount() == 2; }));

    EXPECT_TRUE(getRepoContent().empty());
}

using TankerKoenigCollectorLifetimeTest = TankerKoenigCollectorTest;

TEST_F(TankerKoenigCollectorLifetimeTest, DestructorBlocksUntilInFlightCollectionIsFinished)
{
    constexpr auto MAX_ACCEPTABLE_DELAY{ std::chrono::milliseconds(2000) };

    BlockingCountingGateway gateway;
    threading::thread_t releaser([&gateway] {
        gateway.waitUntilFetchStarted();
        std::this_thread::sleep_for(RELEASE_DELAY);
        gateway.release();
    });

    const auto start{ std::chrono::steady_clock::now() };
    {
        fuel::TankerKoenigCollector<BlockingCountingGateway> collector{
            TEST_API_KEY, TEST_POSTAL_CODE, m_repo, m_bus, gateway
        };

        collector.collect();
        gateway.waitUntilFetchStarted();
    }
    const auto elapsed{
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start)
    };

    releaser.join();

    EXPECT_GE(static_cast<double>(elapsed.count()), static_cast<double>(RELEASE_DELAY.count()));
    EXPECT_LT(static_cast<double>(elapsed.count()), static_cast<double>(MAX_ACCEPTABLE_DELAY.count()));

    EXPECT_FALSE(getRepoContent().empty());
}

} // namespace ful::testing
