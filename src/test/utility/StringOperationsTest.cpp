#include "utility/StringOperations.hpp"

#include <gtest/gtest.h>

#include <string>
#include <string_view>

namespace
{

const std::string RAW_TEST_STR{ "test" };

} // namespace

namespace ful::testing
{

TEST(StringOperationsLTrimTest, EmptyStringStaysEmpty)
{
    std::string s;
    ltrim(s);

    EXPECT_TRUE(s.empty());
}

TEST(StringOperationsLTrimTest, SingleWhitespaceCharacterBecomesEmpty)
{
    std::string s{ " " };
    ltrim(s);

    EXPECT_TRUE(s.empty());
}

TEST(StringOperationsLTrimTest, AllWhiteSpaceBecomesEmpty)
{
    std::string s{ "   \t\n " };
    ltrim(s);

    EXPECT_TRUE(s.empty());
}

TEST(StringOperationsLTrimTest, SingleNonWhitespaceCharacterIsUnchanged)
{
    std::string s{ "a" };
    ltrim(s);

    EXPECT_EQ(s, "a");
}

TEST(StringOperationsLTrimTest, NoWhitespaceIsUnchanged)
{
    std::string s{ ::RAW_TEST_STR };
    ltrim(s);

    EXPECT_EQ(s, ::RAW_TEST_STR);
}

TEST(StringOperationsLTrimTest, LeadingWhitespaceIsRemoved)
{
    std::string s{ "    " + ::RAW_TEST_STR };
    ltrim(s);

    EXPECT_EQ(s, ::RAW_TEST_STR);
}

TEST(StringOperationsLTrimTest, TrailingWhitespaceIsPreserved)
{
    std::string s{ ::RAW_TEST_STR + "    " };
    std::string sCopy{ s };
    ltrim(s);

    EXPECT_EQ(s, sCopy);
}

TEST(StringOperationsLTrimTest, MixedWhitespaceCharactersAreRemoved)
{
    std::string s{ " \t\r\n" + ::RAW_TEST_STR };
    ltrim(s);

    EXPECT_EQ(s, ::RAW_TEST_STR);
}

TEST(StringOperationsRTrimTest, EmptyStringStaysEmpty)
{
    std::string s;
    rtrim(s);

    EXPECT_TRUE(s.empty());
}

TEST(StringOperationsRTrimTest, SingleWhitespaceCharacterBecomesEmpty)
{
    std::string s{ " " };
    rtrim(s);

    EXPECT_TRUE(s.empty());
}

TEST(StringOperationsRTrimTest, AllWhiteSpaceBecomesEmpty)
{
    std::string s{ "   \t\n " };
    rtrim(s);

    EXPECT_TRUE(s.empty());
}

TEST(StringOperationsRTrimTest, SingleNonWhitespaceCharacterIsUnchanged)
{
    std::string s{ "a" };
    rtrim(s);

    EXPECT_EQ(s, "a");
}

TEST(StringOperationsRTrimTest, NoWhitespaceIsUnchanged)
{
    std::string s{ ::RAW_TEST_STR };
    rtrim(s);

    EXPECT_EQ(s, ::RAW_TEST_STR);
}

TEST(StringOperationsRTrimTest, TrailingWhitespaceIsPreserved)
{
    std::string s{ ::RAW_TEST_STR + "    " };
    rtrim(s);

    EXPECT_EQ(s, ::RAW_TEST_STR);
}

TEST(StringOperationsRTrimTest, LeadingWhitespaceIsRemoved)
{
    std::string s{ "    " + ::RAW_TEST_STR };
    std::string sCopy{ s };
    rtrim(s);

    EXPECT_EQ(s, sCopy);
}

TEST(StringOperationsRTrimTest, MixedWhitespaceCharactersAreRemoved)
{
    std::string s{ ::RAW_TEST_STR + " \t\r\n" };
    rtrim(s);

    EXPECT_EQ(s, ::RAW_TEST_STR);
}

TEST(StringOperationsTrimTest, EmptyStringStaysEmpty)
{
    std::string s;
    trim(s);

    EXPECT_TRUE(s.empty());
}

TEST(StringOperationsTrimTest, AllWhiteSpacesBecomeEmpty)
{
    std::string s{ " \t \n " };
    trim(s);

    EXPECT_TRUE(s.empty());
}

TEST(StringOperationsTrimTest, NoWhitespaceIsUnchanged)
{
    std::string s{ ::RAW_TEST_STR };
    trim(s);

    EXPECT_EQ(s, ::RAW_TEST_STR);
}

TEST(StringOperationsTrimTest, BothSidesAreTrimmed)
{
    std::string s{ "  " + ::RAW_TEST_STR + "  " };
    trim(s);

    EXPECT_EQ(s, ::RAW_TEST_STR);
}

TEST(StringOperationsTrimTest, InternalWhitespaceIsPreserved)
{
    const std::string internal{ ::RAW_TEST_STR + "  " + ::RAW_TEST_STR };

    std::string s{ "  " + internal + "  " };
    trim(s);

    EXPECT_EQ(s, internal);
}

TEST(StringOperationsTrimViewTest, EmptyViewProducesEmptyString)
{
    EXPECT_TRUE(trimView(std::string_view{}).empty());
}

TEST(StringOperationsTrimViewTest, BothSidesAreTrimmed)
{
    std::string s{ "  " + ::RAW_TEST_STR + "  " };

    EXPECT_EQ(trimView(s), ::RAW_TEST_STR);
}

TEST(StringOperationsTrimViewTest, InternalWhitespaceIsPreserved)
{
    const std::string internal{ ::RAW_TEST_STR + "  " + ::RAW_TEST_STR };

    std::string s{ "  " + internal + "  " };

    EXPECT_EQ(trimView(s), internal);
}

TEST(StringOperationsTrimViewTest, DoesNotMutateSourceBuffer)
{
    const std::string source{ "  " + ::RAW_TEST_STR + "  " };
    const std::string sourceCopy{ source };
    const std::string_view view{ source };

    const auto trimmed{ trimView(view) };

    EXPECT_EQ(source, sourceCopy);
    EXPECT_EQ(trimmed, ::RAW_TEST_STR);
}

TEST(StringOperationsTrimViewTest, ViewIntoSubstringOfLargerBuffer)
{
    const std::string source{ "prefix   middle   suffix" };
    const std::string_view middle{ source.data() + 6, 12 }; // NOLINT

    EXPECT_EQ(trimView(middle), "middle");
}

} // namespace ful::testing
