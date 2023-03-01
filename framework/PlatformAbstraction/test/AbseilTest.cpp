//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "absl/types/variant.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "absl/strings/string_view.h"
#include "absl/algorithm/container.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "Collections/String.h"

namespace ramses_internal
{
    TEST(AAbseil, canUseVariant)
    {
        struct foo
        {
            int i;
        };
        absl::variant<int, foo, std::string> v;
        v = 123;
        EXPECT_EQ(123, absl::get<int>(v));
        v = foo{543};
        EXPECT_EQ(543, absl::get<foo>(v).i);
        v = "baz";
        EXPECT_EQ("baz", absl::get<std::string>(v));
    }

    TEST(AAbseil, canUseOptional)
    {
        absl::optional<int> o;
        EXPECT_FALSE(o.has_value());
        o = 444;
        EXPECT_TRUE(o.has_value());
        EXPECT_EQ(444, *o);
    }

    TEST(AAbseil, canUseSpan)
    {
        int data[3] = {4, 3, 2};
        absl::Span<int> s(data);
        EXPECT_EQ(data, s.data());
        EXPECT_EQ(3u, s.size());
    }

    TEST(AAbseil, canUseStringView)
    {
        const char* str = "asd";
        absl::string_view sv(str);
        EXPECT_EQ("asd", sv);
        EXPECT_EQ(str, sv.data());
        EXPECT_EQ(3u, sv.size());
    }

    TEST(AAbseil, canUseContainerAlgorithms)
    {
        std::vector<int> v = {1, 2, 3, 4};
        EXPECT_EQ(v.begin() + 1, absl::c_find(v, 2));
        EXPECT_EQ(v.end(), absl::c_find(v, 0));
    }

    TEST(AAbseil, stringToLower)
    {
        EXPECT_EQ("ascdef!123", absl::AsciiStrToLower("ascDEF!123"));
        EXPECT_EQ("ascdef!123", absl::AsciiStrToLower(String("ascDEF!123")));
        EXPECT_EQ("ascdef!123", absl::AsciiStrToLower(std::string("ascDEF!123")));
    }

    TEST(AAbseil, stringToUpper)
    {
        EXPECT_EQ("ASCDEF!123", absl::AsciiStrToUpper("ascDEF!123"));
        EXPECT_EQ("ASCDEF!123", absl::AsciiStrToUpper(String("ascDEF!123")));
        EXPECT_EQ("ASCDEF!123", absl::AsciiStrToUpper(std::string("ascDEF!123")));
    }

    TEST(AAbseil, stringStartsWith)
    {
        EXPECT_TRUE(absl::StartsWith("", ""));
        EXPECT_TRUE(absl::StartsWith("abcde", ""));
        EXPECT_TRUE(absl::StartsWith("abc", "abc"));
        EXPECT_TRUE(absl::StartsWith("abcde", "abc"));
        EXPECT_FALSE(absl::StartsWith("", "abc"));
        EXPECT_FALSE(absl::StartsWith("def", "abc"));
    }

    TEST(AAbseil, stringEndsWith)
    {
        EXPECT_TRUE(absl::EndsWith("", ""));
        EXPECT_TRUE(absl::EndsWith("abcde", ""));
        EXPECT_TRUE(absl::EndsWith("abc", "abc"));
        EXPECT_TRUE(absl::EndsWith("abcde", "de"));
        EXPECT_FALSE(absl::EndsWith("", "abc"));
        EXPECT_FALSE(absl::EndsWith("abc", "def"));
    }
}
