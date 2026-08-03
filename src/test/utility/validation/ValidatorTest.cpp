#include "utility/validation/Validator.hpp"

#include <gtest/gtest.h>

#include <nlohmann/json.hpp>

#include <iterator>
#include <string>

namespace ful::testing
{

namespace
{

using namespace nlohmann::literals;

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
          "minimum": 1
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
          "type": "string"
      },
      "report_dir": {
          "description": "Path to the directory where the reports will be output to",
          "type": "string"
      }
  }
}
)" };

const auto NON_CONFORM_JSON{ R"(
{
    "max_stations": -1
}
)"_json };

} // namespace

TEST(ValidatorFromJsonStringTest, ValidatorFromValidJsonStringProducesValidator)
{
    // NOLINTBEGIN(bugprone-unchecked-optional-access): Not unchecked access
    Validator validator{ std::string(VALID_SCHEMA) };

    const auto r{ validator.validate(NON_CONFORM_JSON) };

    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(std::distance((*r).begin(), (*r).end()), 1);
    // NOLINTEND(bugprone-unchecked-optional-access)
}

} // namespace ful::testing
