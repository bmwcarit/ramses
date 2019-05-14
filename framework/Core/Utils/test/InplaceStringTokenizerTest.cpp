//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/InplaceStringTokenizer.h"
#include "Collections/Vector.h"
#include "gtest/gtest.h"

namespace ramses_internal
{
    static std::vector<String> CollectTokens(const char* str, UInt maxStringLength, Char splitToken)
    {
        std::vector<String> res;
        String in(str);
        InplaceStringTokenizer::TokenizeToCStrings(in, maxStringLength, splitToken,
            [&res](const char* s) { res.push_back(s); });
        EXPECT_EQ(String(str), in);
        return res;
    }

    TEST(AInplaceStringTokenizer, CanHandleEmptyString)
    {
        EXPECT_EQ(0u, CollectTokens("", 100, '\0').size());
    }

    TEST(AInplaceStringTokenizer, ReturnsOneStringWhenFits)
    {
        auto v = CollectTokens("asdef", 100, '\0');
        ASSERT_EQ(1u, v.size());
        EXPECT_EQ("asdef", v[0]);
    }

    TEST(AInplaceStringTokenizer, SplitsBasedOnLengthOnly)
    {
        auto v = CollectTokens("1234567", 3, '\0');
        ASSERT_EQ(3u, v.size());
        EXPECT_EQ("123", v[0]);
        EXPECT_EQ("456", v[1]);
        EXPECT_EQ("7", v[2]);
    }

    TEST(AInplaceStringTokenizer, SplitsBasedOnLengthWhenLastExactlyFits)
    {
        auto v = CollectTokens("123456", 3, '\0');
        ASSERT_EQ(2u, v.size());
        EXPECT_EQ("123", v[0]);
        EXPECT_EQ("456", v[1]);
    }

    TEST(AInplaceStringTokenizer, SplitsBasedOnSeparator)
    {
        auto v = CollectTokens("123\n456\n7", 1000, '\n');
        ASSERT_EQ(3u, v.size());
        EXPECT_EQ("123", v[0]);
        EXPECT_EQ("456", v[1]);
        EXPECT_EQ("7", v[2]);
    }

    TEST(AInplaceStringTokenizer, SplitsBasedOnSeparatorWhenLastExactlyFits)
    {
        auto v = CollectTokens("123\n456", 1000, '\n');
        ASSERT_EQ(2u, v.size());
        EXPECT_EQ("123", v[0]);
        EXPECT_EQ("456", v[1]);
    }

    TEST(AInplaceStringTokenizer, SkipsSeparatorInLastPosition)
    {
        auto v = CollectTokens("123\n456\n", 1000, '\n');
        ASSERT_EQ(2u, v.size());
        EXPECT_EQ("123", v[0]);
        EXPECT_EQ("456", v[1]);
    }

    TEST(AInplaceStringTokenizer, SplitsOnMixedLengthAndSeparator)
    {
        auto v = CollectTokens("1234\n56\n7890", 3, '\n');
        ASSERT_EQ(5u, v.size());
        EXPECT_EQ("123", v[0]);
        EXPECT_EQ("4", v[1]);
        EXPECT_EQ("56", v[2]);
        EXPECT_EQ("789", v[3]);
        EXPECT_EQ("0", v[4]);
    }
}
