#include "fuel/TankerKoenigAPIParser.hpp"

#include <gtest/gtest.h>

#include <chrono>
#include <exception>
#include <format>
#include <string>
#include <vector>

namespace ful::testing
{

namespace
{

/// \brief Abstract sample of a valid Tanker Koenig API response with one station that serves all three supported fuels.
///
/// The sample was cut so that it only includes the most relevant parts that the collector actually extracts.
///
/// \see https://creativecommons.tankerkoenig.de/swagger
constexpr auto VALID_RESPONSE_SINGLE_STATION_ALL_FUELS{
    R"(
{
    "timestamp": "2020-01-04T22:14:06+01:00",
    "stations": [
        {
            "id": "test-id-0",
            "fuels": [
                {
                    "name": "Super E5",
                    "price": 2.249
                },
                {
                    "name": "Super E10",
                    "price": 2.189
                },
                {
                    "name": "Diesel",
                    "price": 2.309
                }
            ]
        }
    ]
}
    )"
};

/// \brief Abstract sample of a valid Tanker Koenig API response with multiple stations.
///
/// The sample was cut so that it only includes the most relevant parts that the collector actually extracts.
///
/// \see https://creativecommons.tankerkoenig.de/swagger
constexpr auto VALID_RESPONSE_MULTIPLE_STATION_ALL_FUELS{
    R"(
{
    "timestamp": "2020-01-04T22:14:06+01:00",
    "stations": [
        {
            "id": "test-id-0",
            "fuels": [
                {
                    "name": "Super E5",
                    "price": 2.249
                },
                {
                    "name": "Super E10",
                    "price": 2.189
                },
                {
                    "name": "Diesel",
                    "price": 2.309
                }
            ]
        },
        {
            "id": "test-id-1",
            "fuels": [
                {
                    "name": "Super E5",
                    "price": 2.249
                },
                {
                    "name": "Super E10",
                    "price": 2.189
                },
                {
                    "name": "Diesel",
                    "price": 2.309
                }
            ]
        }
    ]
}
    )"
};

/// \brief Abstract sample of a valid Tanker Koenig API response with one station that serves some supported fuels.
///
/// The sample was cut so that it only includes the most relevant parts that the collector actually extracts.
///
/// \see https://creativecommons.tankerkoenig.de/swagger
constexpr auto VALID_RESPONSE_SINGLE_STATION_SOME_FUELS{
    R"(
{
    "timestamp": "2020-01-04T22:14:06+01:00",
    "stations": [
        {
            "id": "test-id-0",
            "fuels": [
                {
                    "name": "Super E10",
                    "price": 2.189
                },
                {
                    "name": "Diesel",
                    "price": 2.309
                }
            ]
        }
    ]
}
    )"
};

/// \brief Abstract sample of a valid Tanker Koenig API response with lower case fuel names.
constexpr auto VALID_RESPONSE_LOWER_CASE_NAMING{
    R"(
{
    "timestamp": "2020-01-04T22:14:06+01:00",
    "stations": [
        {
            "id": "test-id-0",
            "fuels": [
                {
                    "name": "Super e5",
                    "price": 2.249
                },
                {
                    "name": "Super e10",
                    "price": 2.189
                },
                {
                    "name": "diesel",
                    "price": 2.309
                }
            ]
        }
    ]
}
    )"
};

/// \brief Abstract sample of an invalid response that has no \p stations key.
constexpr auto INVALID_MISSING_STATIONS_KEY{
    R"(
{
    "timestamp": "2020-01-04T22:14:06+01:00"
}
    )"
};

/// \brief Abstract sample of an invalid response that does not contain a \p fuels key.
constexpr auto INVALID_MISSING_FUELS_KEY{
    R"(
{
    "timestamp": "2020-01-04T22:14:06+01:00",
    "stations": [
        {
            "id": "test-id-1"
        }
    ]
}
    )"
};

/// \brief Abstract sample of an invalid response with a malformed timestamp.
constexpr auto INVALID_MALFORMED_TIMESTAMP{
    R"(
{
    "timestamp": "",
    "stations": [
        {
            "id": "test-id-0",
            "fuels": [
                {
                    "name": "Diesel",
                    "price": 2.309
                }
            ]
        }
    ]
}
    )"
};

} // namespace

/// \brief Test that the parser can extract all supported fuel types from a single station.
TEST(TankerKoenigAPIParserTest, ParsesSingleStationAllPricesPresent)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Technically not unchecked

    const auto results{ fuel::parseStationPrices(VALID_RESPONSE_SINGLE_STATION_ALL_FUELS) };

    ASSERT_EQ(results.size(), 1);

    ASSERT_TRUE(results[0].diesel.has_value());
    EXPECT_EQ(*(results[0].diesel), 2309);
    ASSERT_TRUE(results[0].e10.has_value());
    EXPECT_EQ(*(results[0].e10), 2189);
    ASSERT_TRUE(results[0].e5.has_value());
    EXPECT_EQ(*(results[0].e5), 2249);

    EXPECT_EQ(results[0].stationId, "test-id-0");

    // NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Test that the parser simply returns an empty \ref std::vector if the response for some reason doesn't contain
///     a \p stations key.
TEST(TankerKoenigAPIParserTest, MissingStationsKeyReturnsEmptyVector)
{
    const auto results{ fuel::parseStationPrices(INVALID_MISSING_STATIONS_KEY) };

    ASSERT_TRUE(results.empty());
}

/// \brief Test that the parser returns a \ref std::nullopt if the station in the response does not have a \p fuels key
///     for all types of fuel.
TEST(TankerKoenigAPIParserTest, MissingFuelsKeyreturnsNullopt)
{
    const auto results{ fuel::parseStationPrices(INVALID_MISSING_FUELS_KEY) };

    EXPECT_FALSE(results[0].diesel.has_value());
    EXPECT_FALSE(results[0].e10.has_value());
    EXPECT_FALSE(results[0].e5.has_value());
}

/// \brief Test that the parser can handle multiple stations correctly
TEST(TankerKoenigAPIParserTest, ParsesMultipleStationAllPricesPresent)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Technically not unchecked

    const auto results{ fuel::parseStationPrices(VALID_RESPONSE_MULTIPLE_STATION_ALL_FUELS) };

    ASSERT_EQ(results.size(), 2);

    for(unsigned int i{ 0 }; i < results.size(); ++i)
    {
        ASSERT_TRUE(results[i].diesel.has_value());
        EXPECT_EQ(*(results[i].diesel), 2309);
        ASSERT_TRUE(results[i].e10.has_value());
        EXPECT_EQ(*(results[i].e10), 2189);
        ASSERT_TRUE(results[i].e5.has_value());
        EXPECT_EQ(*(results[i].e5), 2249);

        EXPECT_EQ(results[i].stationId, std::format("test-id-{}", i));
    }

    // NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Test that the parser can extract supported fuel types and supply \ref std::nullopt if a fuel type is not available.
TEST(TankerKoenigAPIParserTest, ParsesSingleStationSomePricesPresent)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Technically not unchecked

    const auto results{ fuel::parseStationPrices(VALID_RESPONSE_SINGLE_STATION_SOME_FUELS) };

    ASSERT_EQ(results.size(), 1);

    ASSERT_TRUE(results[0].diesel.has_value());
    EXPECT_EQ(*(results[0].diesel), 2309);
    ASSERT_TRUE(results[0].e10.has_value());
    EXPECT_EQ(*(results[0].e10), 2189);
    EXPECT_FALSE(results[0].e5.has_value());

    EXPECT_EQ(results[0].stationId, "test-id-0");

    // NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Test that supplying malformed JSON throws an exception.
TEST(TankerKoenigAPIParserTest, InvalidJsonThrows)
{
    EXPECT_THROW(fuel::parseStationPrices("{abc: 1}"), std::exception);
}

/// \brief Test that the parser can translate a valid ISO-8601 timestamp to \ref std::chrono::system_clock::time_point.
TEST(TankerKoenigAPIParserTest, ValidIsoTimestampParsesCorrectly)
{
    // FIXME: Come up with alternative to std::chrono::parse() for the test since MacOS can't deal with that

    // std::istringstream stream{ "2020-01-04T22:14:06+01:00" };
    //
    // std::chrono::sys_time<std::chrono::seconds> timestamp;
    // stream >> std::chrono::parse("%FT%T%Ez", timestamp);
    //
    // const auto results{ fuel::parseStationPrices(VALID_RESPONSE_SINGLE_STATION_ALL_FUELS) };
    //
    // EXPECT_EQ(results[0].timestamp, timestamp);
}

/// \brief Test that the parser uses the placeholder timestamp if the provided timestamp is not in ISO-8601 format.
TEST(TankerKoenigAPIParserTest, MalformedTiemstampProvidesPlaceholderTimetsamp)
{
    const auto results{ fuel::parseStationPrices(INVALID_MALFORMED_TIMESTAMP) };

    EXPECT_EQ(results[0].timestamp, std::chrono::system_clock::time_point::min());
}

/// \brief Test that the case of the fuel name does not matter to the parser.
TEST(TankerKoenigAPIParserTest, CaseInsensitiveFuelNameMatches)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Technically not unchecked

    const auto results{ fuel::parseStationPrices(VALID_RESPONSE_LOWER_CASE_NAMING) };

    ASSERT_TRUE(results[0].diesel.has_value());
    EXPECT_EQ(*(results[0].diesel), 2309);
    ASSERT_TRUE(results[0].e10.has_value());
    EXPECT_EQ(*(results[0].e10), 2189);
    ASSERT_TRUE(results[0].e5.has_value());
    EXPECT_EQ(*(results[0].e5), 2249);

    // NOLINTEND(bugprone-unchecked-optional-access)
}

} // namespace ful::testing
