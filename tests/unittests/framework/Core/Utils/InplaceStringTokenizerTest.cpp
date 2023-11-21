//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/InplaceStringTokenizer.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "gmock/gmock.h"

namespace ramses::internal
{
    using namespace ::testing;

    static std::vector<std::string> CollectTokens(const char* str, size_t maxStringLength, char splitToken)
    {
        std::vector<std::string> res;
        std::string in(str);
        InplaceStringTokenizer::TokenizeToCStrings(in, maxStringLength, splitToken,
            [&res](const char* s) { res.emplace_back(s); });
        EXPECT_EQ(std::string(str), in);
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

    static std::vector<std::string> CollectMultilineTokens(const char* str, size_t maxBlockSize, char splitToken)
    {
        std::vector<std::string> res;
        std::string in(str);
        InplaceStringTokenizer::TokenizeToMultilineCStrings(in, maxBlockSize, splitToken,
                                                            [&res](const char* s) { res.emplace_back(s); });
        EXPECT_EQ(std::string(str), in);
        return res;
    }

    TEST(AInplaceStringTokenizer, MultilineNoSeparator)
    {
        EXPECT_THAT(CollectMultilineTokens("", 4, '\n'),
                    ElementsAre());
        EXPECT_THAT(CollectMultilineTokens("1", 4, '\n'),
                    ElementsAre("1"));
        EXPECT_THAT(CollectMultilineTokens("123", 4, '\n'),
                    ElementsAre("123"));
        EXPECT_THAT(CollectMultilineTokens("1234", 4, '\n'),
                    ElementsAre("1234"));
        EXPECT_THAT(CollectMultilineTokens("12345", 4, '\n'),
                    ElementsAre("1234", "5"));

        EXPECT_THAT(CollectMultilineTokens("12345678", 4, '\n'),
                    ElementsAre("1234", "5678"));
        EXPECT_THAT(CollectMultilineTokens("123456789", 4, '\n'),
                    ElementsAre("1234", "5678", "9"));
    }

    TEST(AInplaceStringTokenizer, MultilineWithSeparator)
    {
        EXPECT_THAT(CollectMultilineTokens("\n", 4, '\n'),
                    ElementsAre("\n"));
        EXPECT_THAT(CollectMultilineTokens("123\n", 4, '\n'),
                    ElementsAre("123\n"));
        EXPECT_THAT(CollectMultilineTokens("1234\n", 4, '\n'),
                    ElementsAre("1234"));
        EXPECT_THAT(CollectMultilineTokens("12345\n", 4, '\n'),
                    ElementsAre("1234", "5\n"));

        EXPECT_THAT(CollectMultilineTokens("123\n45", 4, '\n'),
                    ElementsAre("123", "45"));
        EXPECT_THAT(CollectMultilineTokens("123\n456789", 4, '\n'),
                    ElementsAre("123", "4567", "89"));

        EXPECT_THAT(CollectMultilineTokens("12\n34\n56789", 4, '\n'),
                    ElementsAre("12", "34", "5678", "9"));
        EXPECT_THAT(CollectMultilineTokens("12\n3\n456789", 4, '\n'),
                    ElementsAre("12\n3", "4567", "89"));
        EXPECT_THAT(CollectMultilineTokens("12\n\n\n34\n56789", 4, '\n'),
                    ElementsAre("12\n\n", "34", "5678", "9"));
    }
}
