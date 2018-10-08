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

#include "ConstStringTest.h"


namespace ramses_capu
{
    ConstStringTest::ConstStringTest()
    {
    }

    ConstStringTest::~ConstStringTest()
    {
    }

    void ConstStringTest::SetUp()
    {
    }

    void ConstStringTest::TearDown()
    {
    }

    TEST_F(ConstStringTest, ConstCharConstructor)
    {
        ConstString string("Hello World");
        EXPECT_STREQ("Hello World", string.c_str());
        EXPECT_EQ(11U, string.length());
    }

    TEST_F(ConstStringTest, ConstCharSubConstructor)
    {
        ConstString string("Hello World", 6U);
        EXPECT_STREQ("World", string.c_str());
        EXPECT_EQ(5U, string.length());
    }

    TEST_F(ConstStringTest, CopyConstructor)
    {
        ConstString string1 = "Hello World";
        ConstString string2(string1);

        EXPECT_STREQ("Hello World", string2.c_str());
        EXPECT_EQ(11U, string2.length());
    }

    TEST_F(ConstStringTest, AutoConvert)
    {
        ConstString string = "Hello World";
        EXPECT_STREQ("Hello World", string);
    }

    TEST_F(ConstStringTest, EqualOperator)
    {
        ConstString string1 = "Hello World";
        ConstString string2 = "Hello World";
        EXPECT_TRUE(string1 == string2);
    }

    TEST_F(ConstStringTest, NotEqualOperator)
    {
        ConstString string1 = "Hello World";
        ConstString string2 = "World Hello";
        EXPECT_TRUE(string1 != string2);
    }


    TEST_F(ConstStringTest, SmallerGreater1)
    {
        ConstString str1("Hello1");
        ConstString str2("Hello2");
        EXPECT_TRUE(str1 < str2);
        EXPECT_TRUE(str2 > str1);
    }

    TEST_F(ConstStringTest, SmallerGreater2)
    {
        ConstString str1("Hello Long");
        ConstString str2("Hello Longer");
        EXPECT_TRUE(str1 < str2);
        EXPECT_TRUE(str2 > str1);
    }

    TEST_F(ConstStringTest, SmallerGreater3)
    {
        ConstString str1("Hello Test");
        ConstString str2("Hello Test");
        EXPECT_FALSE(str1 < str2);
        EXPECT_FALSE(str2 > str1);
    }

    TEST_F(ConstStringTest, AssignmentOperator)
    {
        ConstString string1 = "Hello";
        ConstString string2 = "World";

        string2 = string1;
        EXPECT_STREQ("Hello", string2);
    }

    TEST_F(ConstStringTest, FindChar)
    {
        ConstString string = "Hello World";
        EXPECT_EQ(1, string.find('e'));
    }

    TEST_F(ConstStringTest, FindSubStr)
    {
        ConstString string = "Hello World";
        EXPECT_EQ(6, string.find("World"));
    }

    TEST_F(ConstStringTest, RFindChar)
    {
        ConstString string = "Hello World";
        EXPECT_EQ(7, string.rfind('o'));
    }

}
