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
        EXPECT_STREQ("te st", StringUtils::Trim("  te st ").c_str());
        EXPECT_STREQ("", StringUtils::Trim("    ").c_str());
        EXPECT_STREQ("aa", StringUtils::Trim("    aa").c_str());
        EXPECT_STREQ("aa", StringUtils::Trim("aa    ").c_str());
        EXPECT_STREQ("", StringUtils::Trim("").c_str());
        EXPECT_STREQ("a", StringUtils::Trim("a").c_str());
        EXPECT_STREQ("asdf", StringUtils::Trim("asdf").c_str());
    }

    TEST(StringUtilsTest, tokenizeStringToSet)
    {
        const String testString(" test1 test2   test3");
        StringSet tokens;
        StringUtils::Tokenize(testString, tokens);
        EXPECT_EQ(3u, tokens.size());
        EXPECT_TRUE(tokens.contains("test1"));
        EXPECT_TRUE(tokens.contains("test2"));
        EXPECT_TRUE(tokens.contains("test3"));
        EXPECT_FALSE(tokens.contains("test4"));
    }

    TEST(StringUtilsTest, tokenizeString)
    {
        const String testString(" test1 test2   test3");
        std::vector<String> tokens;
        StringUtils::Tokenize(testString, tokens);
        EXPECT_EQ(tokens, std::vector<String>({"test1", "test2", "test3"}));
    }

    TEST(StringUtilsTest, tokenizeEmptyString)
    {
        const String testString("");
        std::vector<String> tokens;
        StringUtils::Tokenize(testString, tokens);
        EXPECT_EQ(0u, tokens.size());
    }

    TEST(StringUtilsTest, WithNonDefaultSplitChar)
    {
        const String testString(",test1,,test2,test3");
        std::vector<String> tokens;
        StringUtils::Tokenize(testString, tokens, ',');
        EXPECT_EQ(tokens, std::vector<String>({"test1", "test2", "test3"}));
    }

    TEST(StringUtilsTest, tokenizeStringToSetWithNonDefaultSplitChar)
    {
        const String testString(",test1,,test2,test3,,");
        StringSet tokens;
        StringUtils::Tokenize(testString, tokens, ',');
        EXPECT_EQ(3u, tokens.size());
        EXPECT_TRUE(tokens.contains("test1"));
        EXPECT_TRUE(tokens.contains("test2"));
        EXPECT_TRUE(tokens.contains("test3"));
        EXPECT_FALSE(tokens.contains("test4"));
    }

    TEST(StringUtilsTest, tokenizeStringWithSingleCharElements)
    {
        const String testString(",1,,2,3");
        StringSet tokens;
        StringUtils::Tokenize(testString, tokens, ',');
        EXPECT_EQ(3u, tokens.size());
        EXPECT_TRUE(tokens.contains("1"));
        EXPECT_TRUE(tokens.contains("2"));
        EXPECT_TRUE(tokens.contains("3"));
        EXPECT_FALSE(tokens.contains("4"));
    }
}
