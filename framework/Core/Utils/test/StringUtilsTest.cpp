//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Utils/StringUtils.h"
#include "SceneAPI/ResourceContentHash.h"

namespace ramses_internal
{
    TEST(StringUtilsTest, trimString)
    {
        const char* testString = "  te st ";
        String trimmedString = StringUtils::Trim(testString);
        EXPECT_STREQ("te st", trimmedString.c_str());

        const char* stringWithOnlySpaces = "    ";
        String trimmedStringWithOnlySpaces = StringUtils::Trim(stringWithOnlySpaces);
        EXPECT_STREQ("", trimmedStringWithOnlySpaces.c_str());

        const char* stringWithOnlyLeftSpaces = "    aa";
        String trimmedStringWithOnlyLeftSpaces = StringUtils::Trim(stringWithOnlyLeftSpaces);
        EXPECT_STREQ("aa", trimmedStringWithOnlyLeftSpaces.c_str());

        const char* stringWithOnlyRightSpaces = "aa    ";
        String trimmedStringWithOnlyRightSpaces = StringUtils::Trim(stringWithOnlyRightSpaces);
        EXPECT_STREQ("aa", trimmedStringWithOnlyRightSpaces.c_str());

        const char* emptyString = "";
        String trimmedEmptyString = StringUtils::Trim(emptyString);
        EXPECT_STREQ("", trimmedEmptyString.c_str());
    }

    TEST(StringUtilsTest, tokenizeString)
    {
        const String testString(" test1 test2   test3");
        StringSet tokens;
        StringUtils::Tokenize(testString, tokens);
        EXPECT_EQ(3u, tokens.count());
        EXPECT_TRUE(tokens.hasElement("test1"));
        EXPECT_TRUE(tokens.hasElement("test2"));
        EXPECT_TRUE(tokens.hasElement("test3"));
        EXPECT_FALSE(tokens.hasElement("test4"));
    }

    TEST(StringUtilsTest, tokenizeStringWithNonDefaultSplitChar)
    {
        const String testString(",test1,,test2,test3");
        StringSet tokens;
        StringUtils::Tokenize(testString, tokens, ',');
        EXPECT_EQ(3u, tokens.count());
        EXPECT_TRUE(tokens.hasElement("test1"));
        EXPECT_TRUE(tokens.hasElement("test2"));
        EXPECT_TRUE(tokens.hasElement("test3"));
        EXPECT_FALSE(tokens.hasElement("test4"));
    }

    TEST(StringUtilsTest, tokenizeStringWithSingleCharElements)
    {
        const String testString(",1,,2,3");
        StringSet tokens;
        StringUtils::Tokenize(testString, tokens, ',');
        EXPECT_EQ(3u, tokens.count());
        EXPECT_TRUE(tokens.hasElement("1"));
        EXPECT_TRUE(tokens.hasElement("2"));
        EXPECT_TRUE(tokens.hasElement("3"));
        EXPECT_FALSE(tokens.hasElement("4"));
    }

    TEST(StringUtilsTest, getHexFromResourceContentHash)
    {
        ResourceContentHash zero_hash(0, 0);
        ResourceContentHash small_hash(6342u, 0);
        ResourceContentHash big_hash(0x0123456789abcdef, 0xfedcba9876543210);

        EXPECT_STREQ("0x00000000000000000000000000000000", StringUtils::HexFromResourceContentHash(zero_hash).c_str());
        EXPECT_STREQ("0x000000000000000000000000000018c6", StringUtils::HexFromResourceContentHash(small_hash).c_str());
        EXPECT_STREQ("0xfedcba98765432100123456789abcdef", StringUtils::HexFromResourceContentHash(big_hash).c_str());
    }

    TEST(StringUtilsTest, getHexFromUInt64)
    {
        uint64_t zero_value  =                   0u;
        uint64_t small_value =                6342u;
        uint64_t big_value   = 6782422452544544535u;

        EXPECT_STREQ("0x0000000000000000", StringUtils::HexFromNumber(zero_value).c_str());
        EXPECT_STREQ("0x00000000000018c6", StringUtils::HexFromNumber(small_value).c_str());
        EXPECT_STREQ("0x5e200149288d1f17", StringUtils::HexFromNumber(big_value).c_str());
    }

    TEST(StringUtilsTest, getHexFromUInt32)
    {
        uint32_t zero_value  =         0u;
        uint32_t small_value =      6342u;
        uint32_t big_value   = 752544535u;

        EXPECT_STREQ("0x00000000", StringUtils::HexFromNumber(zero_value).c_str());
        EXPECT_STREQ("0x000018c6", StringUtils::HexFromNumber(small_value).c_str());
        EXPECT_STREQ("0x2cdaeb17", StringUtils::HexFromNumber(big_value).c_str());
    }

    TEST(StringUtilsTest, getHexFromUInt8)
    {
        uint8_t zero_value  =   0u;
        uint8_t small_value =   4u;
        uint8_t big_value   = 235u;

        EXPECT_STREQ("0x00", StringUtils::HexFromNumber(zero_value).c_str());
        EXPECT_STREQ("0x04", StringUtils::HexFromNumber(small_value).c_str());
        EXPECT_STREQ("0xeb", StringUtils::HexFromNumber(big_value).c_str());
    }

    TEST(StringUtilsTest, IToAFromUInt64)
    {
        uint64_t zero_value  =                   0u;
        uint64_t small_value =                6342u;
        uint64_t big_value   = 6782422452544544535u;

        EXPECT_STREQ(                  "0", StringUtils::IToA(zero_value).c_str());
        EXPECT_STREQ(               "6342", StringUtils::IToA(small_value).c_str());
        EXPECT_STREQ("6782422452544544535", StringUtils::IToA(big_value).c_str());
    }

    TEST(StringUtilsTest, IToAFromUInt32)
    {
        uint32_t zero_value  =         0u;
        uint32_t small_value =      6342u;
        uint32_t big_value   = 752544535u;

        EXPECT_STREQ(        "0", StringUtils::IToA(zero_value).c_str());
        EXPECT_STREQ(     "6342", StringUtils::IToA(small_value).c_str());
        EXPECT_STREQ("752544535", StringUtils::IToA(big_value).c_str());
    }

    TEST(StringUtilsTest, IToAFromUInt8)
    {
        uint8_t zero_value  =   0u;
        uint8_t small_value =   4u;
        uint8_t big_value   = 235u;

        EXPECT_STREQ(  "0", StringUtils::IToA(zero_value).c_str());
        EXPECT_STREQ(  "4", StringUtils::IToA(small_value).c_str());
        EXPECT_STREQ("235", StringUtils::IToA(big_value).c_str());
    }

    TEST(StringUtilsTest, IToAFromInt64)
    {
        int64_t zero_value     =                    0;
        int64_t small_value    =                 6342;
        int64_t big_value      =  6782422452544544535;
        int64_t negative_value = -3742426824234155721;

        EXPECT_STREQ(                   "0", StringUtils::IToA(zero_value).c_str());
        EXPECT_STREQ(                "6342", StringUtils::IToA(small_value).c_str());
        EXPECT_STREQ( "6782422452544544535", StringUtils::IToA(big_value).c_str());
        EXPECT_STREQ("-3742426824234155721", StringUtils::IToA(negative_value).c_str());
    }

    TEST(StringUtilsTest, IToAFromInt32)
    {
        int32_t zero_value     =          0;
        int32_t small_value    =       6342;
        int32_t big_value      =  752544535;
        int32_t negative_value = -491281023;

        EXPECT_STREQ(         "0", StringUtils::IToA(zero_value).c_str());
        EXPECT_STREQ(      "6342", StringUtils::IToA(small_value).c_str());
        EXPECT_STREQ( "752544535", StringUtils::IToA(big_value).c_str());
        EXPECT_STREQ("-491281023", StringUtils::IToA(negative_value).c_str());
    }

    TEST(StringUtilsTest, IToAFromInt8)
    {
        int8_t zero_value     =    0;
        int8_t small_value    =    4;
        int8_t big_value      =  125;
        int8_t negative_value = -123;

        EXPECT_STREQ(   "0", StringUtils::IToA(zero_value).c_str());
        EXPECT_STREQ(   "4", StringUtils::IToA(small_value).c_str());
        EXPECT_STREQ( "125", StringUtils::IToA(big_value).c_str());
        EXPECT_STREQ("-123", StringUtils::IToA(negative_value).c_str());
    }

}
