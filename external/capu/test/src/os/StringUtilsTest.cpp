/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "ramses-capu/os/StringUtils.h"
#include "ramses-capu/os/Memory.h"

TEST(StringUtils, Strcmp)
{
    char string1[] = "My String";
    char string2[] = "My String";
    char string3[] = "Another String";
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp(string1, string2));
    EXPECT_TRUE(0 <= ramses_capu::StringUtils::Strcmp(string1, string3));
}

TEST(StringUtils, Strncpy)
{
    char string1[20] = "My String";
    char string2[30];
    ramses_capu::StringUtils::Strncpy(string2, sizeof(string2), string1);
    EXPECT_STREQ(string1, string2);
}

TEST(StringUtils, Strncpy2)
{
    char string1[20] = "My String";
    char string2[20];
    ramses_capu::Memory::Set(string2, 42, sizeof(string2));
    ramses_capu::StringUtils::Strncpy(string2, 4, string1);
    EXPECT_STREQ("My ", string2);

    // check that only 4 bytes are written, an no more
    for (uint32_t i = 4; i < sizeof(string2); ++i)
    {
        EXPECT_EQ(42, string2[i]);
    }
}

TEST(StringUtils, Strlen1)
{
    char string1[20] = "My String";
    EXPECT_EQ(9u, ramses_capu::StringUtils::Strlen(string1));
}

TEST(StringUtils, Strlen2)
{
    EXPECT_EQ(static_cast<ramses_capu::uint_t>(0),  ramses_capu::StringUtils::Strlen(0));
}

TEST(StringUtils, Strnlen_n_smaller_than_length)
{
    char string1[5] = "abcd";
    EXPECT_EQ(2u, ramses_capu::StringUtils::Strnlen(string1,2));
}

TEST(StringUtils, Strnlen_n_larger_than_length)
{
    char string1[5] = "abcd";
    EXPECT_EQ(4u, ramses_capu::StringUtils::Strnlen(string1, 123));
}

TEST(StringUtils, Strnlen_emptyString)
{
    char string1[1] = "";
    EXPECT_EQ(0u, ramses_capu::StringUtils::Strnlen(string1, 123));
}

TEST(StringUtils, Sprintf)
{
    char string1[20];
    ramses_capu::StringUtils::Sprintf(string1, 20, "%d", 12345);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("12345", string1));
    ramses_capu::StringUtils::Sprintf(string1, 20, "%d %f", 12345, 12.45);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("12345 12.450000", string1));
}

class VscprintfTest
{
public:
    static int32_t Vscprintf(const char* format, ...)
    {
        int32_t length = 0;
        va_list args;
        va_start(args, format);
        length = ramses_capu::StringUtils::Vscprintf(format, args);
        va_end(args);
        return length;
    }
};

TEST(StringUtils, Vscprintf)
{
    int32_t length = VscprintfTest::Vscprintf("This is a test! %d", 12345);
    EXPECT_EQ(21, length);
}

class VsprintfTest
{
public:
    static int32_t Vsprintf(char* buffer, int32_t buffersize, const char* format, ...)
    {
        int32_t length = 0;
        va_list args;
        va_start(args, format);
        length = ramses_capu::StringUtils::Vsprintf(buffer, buffersize, format, args);
        va_end(args);
        return length;
    }
};

TEST(StringUtils, VsprintfReturnsLengthOfCharactersWritten)
{
    char string1[20];
    EXPECT_EQ(5, VsprintfTest::Vsprintf(string1, 20, "%d", 12345));
    EXPECT_LE(4, VsprintfTest::Vsprintf(string1, 20, "%f", 3.14f));
    EXPECT_LE(7, VsprintfTest::Vsprintf(string1, 20, "%f", 3.14789f));
    EXPECT_LE(5 + 1 + 4, VsprintfTest::Vsprintf(string1, 20, "%d %f", 12345, 3.14f));

    EXPECT_LE(0, VsprintfTest::Vsprintf(string1, 20, "%s", ""));
    EXPECT_LE(0, VsprintfTest::Vsprintf(string1, 20, ""));
}

TEST(StringUtils, SprintfReturnsLengthOfCharactersWritten)
{
    char string1[20];
    EXPECT_EQ(5, ramses_capu::StringUtils::Sprintf(string1, 20, "%d", 12345));
    EXPECT_LE(4, ramses_capu::StringUtils::Sprintf(string1, 20, "%f", 3.14f));
    EXPECT_LE(7, ramses_capu::StringUtils::Sprintf(string1, 20, "%f", 3.14789f));
    EXPECT_LE(5+1+4, ramses_capu::StringUtils::Sprintf(string1, 20, "%d %f", 12345, 3.14f));

    EXPECT_LE(0, ramses_capu::StringUtils::Sprintf(string1, 20, "%s", ""));
    EXPECT_LE(0, ramses_capu::StringUtils::Sprintf(string1, 20, ""));
}

TEST(StringUtils, LastIndexOf)
{
    EXPECT_EQ(5, ramses_capu::StringUtils::LastIndexOf("012345", '5'));
    EXPECT_EQ(8, ramses_capu::StringUtils::LastIndexOf("___512345", '5'));
    EXPECT_EQ(11, ramses_capu::StringUtils::LastIndexOf("______555555asdfasdf", '5'));
    EXPECT_EQ(-1, ramses_capu::StringUtils::LastIndexOf("111", '5'));
    EXPECT_EQ(-1, ramses_capu::StringUtils::LastIndexOf("", '5'));
    EXPECT_EQ(-1, ramses_capu::StringUtils::LastIndexOf(0, '5'));
}

TEST(StringUtils, IndexOf)
{
    EXPECT_EQ(5, ramses_capu::StringUtils::IndexOf("012345", '5'));
    EXPECT_EQ(0, ramses_capu::StringUtils::IndexOf("512345", '5'));
    EXPECT_EQ(0, ramses_capu::StringUtils::IndexOf("555555asdfasdf", '5'));
    EXPECT_EQ(-1, ramses_capu::StringUtils::IndexOf("111", '5'));
    EXPECT_EQ(-1, ramses_capu::StringUtils::IndexOf("", '5'));
    EXPECT_EQ(-1, ramses_capu::StringUtils::IndexOf(0, '5'));
}

TEST(StringUtils, StartsWith1)
{
    EXPECT_TRUE(ramses_capu::StringUtils::StartsWith("prefix", "prefix"));
    EXPECT_TRUE(ramses_capu::StringUtils::StartsWith("prefixwithsuffix", "prefix"));
    EXPECT_FALSE(ramses_capu::StringUtils::StartsWith("prefixwithsuffix", "noprefix"));

    EXPECT_TRUE(ramses_capu::StringUtils::StartsWith("prefix with spaces AND SOME TEXT", "prefix with spaces"));

    EXPECT_TRUE(ramses_capu::StringUtils::StartsWith("testing empty prefix", ""));

    EXPECT_FALSE(ramses_capu::StringUtils::StartsWith("DifferentCase", "differentCase"));
}

TEST(StringUtils, StartsWith2)
{
    EXPECT_FALSE(ramses_capu::StringUtils::StartsWith("prefixwithsuffix", 0));
    EXPECT_FALSE(ramses_capu::StringUtils::StartsWith(0, "prefix"));
}

TEST(StringUtils, IndexOf2)
{
    EXPECT_EQ(6, ramses_capu::StringUtils::IndexOf("Hello World", "World"));
    EXPECT_EQ(-1, ramses_capu::StringUtils::IndexOf("Hello World", "WORLD"));

}
