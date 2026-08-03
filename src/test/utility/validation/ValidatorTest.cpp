#include "utility/validation/Validator.hpp"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <nlohmann/json_fwd.hpp>

#include <string>

namespace ful::testing
{

namespace
{

constexpr auto VALID_SCHEMA{ R"(
{
  "$schema": "https://json-schema.org/draft/2020-12/schema",
  "title": "FuelPulseConfig",
  "type": "object",
  "properties": {
      "max_stations": {
          "description": "The maximum amount of stations to analyze",
          "type": "integer",
          "minimum": 1
      },
      "postal_code": {
          "description": "The postal code around which to analyze the gas stations",
          "type": "integer",
          "minimum": 1067,
          "maximum": 99998
      },
      "search_radius": {
          "description": "The radius around which to search for gas stations",
          "type": "number",
          "minimum": 1.0
      },
      "collection_interval": {
          "description": "Intervall in which to query the gas station prices",
          "type": "integer",
          "minimum": 1
      },
      "database_path": {
          "description": "Path to the database where the measurements and stations are stored",
          "type": "string",
          "pattern": ".db3$"
      },
      "report_dir": {
          "description": "Path to the directory where the reports will be output to",
          "type": "string"
      }
  }
}
)" };

const auto SCHEMA_CONFORM_JSON{ R"(
{
    "max_stations": 1,
    "postal_code": 1067,
    "search_radius": 1.0,
    "collection_interval": 1,
    "databasePath": "db.db3",
    "report_dir": "report/"
}
)" };

} // namespace

/// \brief Test that passing a schema conform document to the validator reports no errors.
TEST(ValidatorFromJsonStringTest, ValidDocumentReportsNoValidationErrors)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Not unchecked access
    const Validator validator{ std::string(VALID_SCHEMA) };

    const auto errors{ validator.validate(nlohmann::json::parse(std::string(SCHEMA_CONFORM_JSON))) };

    EXPECT_FALSE(errors.has_value());
    // NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Test that passing a document with an invalid 'max_stations' field reports back the error.
TEST(ValidatorFromJsonStringTest, InvalidMaxStationsIsCaught)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Not unchecked access
    const Validator validator{ std::string(VALID_SCHEMA) };

    auto json = nlohmann::json::parse(std::string(SCHEMA_CONFORM_JSON));
    json["postal_code"] = 01066; // NOLINT(readability-magic-numbers): 01067 is the smallest postal code in Germany

    const auto result{ validator.validate(json) };
    ASSERT_TRUE(result.has_value());

    const auto& errors{ *result };
    ASSERT_EQ(errors.size(), 1);
    EXPECT_THAT(errors[0].path.to_string(), ::testing::HasSubstr("postal_code"));
    EXPECT_THAT(errors[0].message, ::testing::HasSubstr("below minimum"));
    // NOLINTEND(bugprone-unchecked-optional-access)
}

/// \brief Test that multiple schema validations are reported appropriately.
TEST(ValidatorFromJsonStringTest, MultipleViolationsAreCaught)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Not unchecked access
    const Validator validator{ std::string(VALID_SCHEMA) };

    auto json = nlohmann::json::parse(std::string(SCHEMA_CONFORM_JSON));
    json["postal_code"] = 99999; // NOLINT(readability-magic-numbers): 99998 is the largest postal code in Germany
    json["database_path"] = "invalid.db";

    const auto result{ validator.validate(json) };
    ASSERT_TRUE(result.has_value());

    const auto& errors{ *result };
    ASSERT_EQ(errors.size(), 2);
    EXPECT_THAT(errors[0].path.to_string(), ::testing::HasSubstr("database_path"));
    EXPECT_THAT(errors[0].message, ::testing::HasSubstr("does not match regex pattern"));
    EXPECT_THAT(errors[1].path.to_string(), ::testing::HasSubstr("postal_code"));
    EXPECT_THAT(errors[1].message, ::testing::HasSubstr("exceeds maximum"));
    // NOLINTEND(bugprone-unchecked-optional-access)
}

} // namespace ful::testing
