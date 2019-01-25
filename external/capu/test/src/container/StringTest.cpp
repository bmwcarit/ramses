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
#include "ramses-capu/container/String.h"
#include "ramses-capu/Error.h"
#include "ramses-capu/Config.h"
#include "ramses-capu/os/Memory.h"

TEST(String, TestCTor)
{
    ramses_capu::String str;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("", str.c_str()));
}

TEST(String, TestCStr)
{
    ramses_capu::String str("asdf");
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("asdf", str.c_str()));
    EXPECT_EQ(4u, str.getLength());
}

TEST(String, TestCopyConstructor)
{
    ramses_capu::String str("asdf");
    ramses_capu::String copy(str);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("asdf", str.c_str()));
    EXPECT_STREQ("asdf", str.c_str());
    EXPECT_EQ(str.getLength(), copy.getLength());
}

TEST(String, InitialSizeConstructor)
{
    ramses_capu::String str(5, 'a');

    EXPECT_EQ(5u, str.getLength());
    EXPECT_STREQ("aaaaa", str.c_str());
}

TEST(String, ConstructWithEmptyStringDoesNotAllocateMemory)
{
    ramses_capu::String str("");
    EXPECT_EQ(0u, str.getLength());
    EXPECT_STREQ("", str.c_str());
    EXPECT_TRUE(str.data() == nullptr);
}

TEST(String, TestAssignOperator1)
{
    ramses_capu::String str("asdf");
    ramses_capu::String other("other");

    EXPECT_EQ(4u, str.getLength());
    EXPECT_EQ(5u, other.getLength());
    str = other;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("other", str.c_str()));
    EXPECT_EQ(5u, str.getLength());
    EXPECT_EQ(5u, other.getLength());

    ramses_capu::String str2;
    ramses_capu::String other2;
    str2 = other2;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("", str2.c_str()));

    // one string on stack, one on heap
    ramses_capu::String stringStack1;
    ramses_capu::String stringStack2;
    ramses_capu::String *stringHeap1 = new ramses_capu::String();
    ramses_capu::String *stringHeap2 = new ramses_capu::String();


    stringStack1 = *stringHeap1;
    EXPECT_EQ(0u, stringHeap1->getLength());
    EXPECT_EQ(0u, stringStack1.getLength());
    delete stringHeap1;

    *stringHeap2 = stringStack2;
    EXPECT_EQ(0u, stringHeap2->getLength());
    EXPECT_EQ(0u, stringStack2.getLength());
    delete stringHeap2;
}

TEST(String, MoveConstructor)
{
    ramses_capu::String a("abc");
    ramses_capu::String b(std::move(a));

    EXPECT_EQ(ramses_capu::String("abc"), b);
    EXPECT_EQ(3u, b.getLength());
    EXPECT_EQ(ramses_capu::String(), a);
    EXPECT_EQ(0u, a.getLength());
}

TEST(String, MoveAssignment)
{
    ramses_capu::String a("abc");
    ramses_capu::String b;
    b = std::move(a);

    EXPECT_EQ(ramses_capu::String("abc"), b);
    EXPECT_EQ(3u, b.getLength());
    EXPECT_EQ(ramses_capu::String(), a);
    EXPECT_EQ(0u, a.getLength());
}

TEST(String, TestToLowerCase1)
{
    ramses_capu::String str("abcDEF");
    str.toLowerCase();
    EXPECT_EQ(ramses_capu::String("abcdef"), str);
}

TEST(String, TestToLowerCase2)
{
    ramses_capu::String str("");
    str.toLowerCase();
    EXPECT_EQ(ramses_capu::String(""), str);
}

TEST(String, TestToLowerCase3)
{
    ramses_capu::String str("ABC");
    str.toLowerCase();
    EXPECT_EQ(ramses_capu::String("abc"), str);
}

TEST(String, TestToLowerCase4)
{
    ramses_capu::String str("AbC");
    str.toLowerCase();
    EXPECT_EQ(ramses_capu::String("abc"), str);
}

TEST(String, TestToLowerCase5)
{
    ramses_capu::String str("Abc");
    str.toLowerCase();
    EXPECT_EQ(ramses_capu::String("abc"), str);
}

TEST(String, TestToUpperCase1)
{
    ramses_capu::String str("abcDEF");
    str.toUpperCase();
    EXPECT_EQ(ramses_capu::String("ABCDEF"), str);
}

TEST(String, TestToUpperCase2)
{
    ramses_capu::String str("");
    str.toUpperCase();
    EXPECT_EQ(ramses_capu::String(""), str);
}

TEST(String, TestToUpperCase3)
{
    ramses_capu::String str("ABC");
    str.toUpperCase();
    EXPECT_EQ(ramses_capu::String("ABC"), str);
}

TEST(String, TestToUpperCase4)
{
    ramses_capu::String str("AbC");
    str.toUpperCase();
    EXPECT_EQ(ramses_capu::String("ABC"), str);
}

TEST(String, TestToUpperCase5)
{
    ramses_capu::String str("Abc");
    str.toUpperCase();
    EXPECT_EQ(ramses_capu::String("ABC"), str);
}

TEST(String, TestAssignOperator2)
{
    ramses_capu::String str("asdf");
    str = "other";
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("other", str.c_str()));
    EXPECT_EQ(5u, str.getLength());
}

TEST(String, TestAssignOperator3)
{
    ramses_capu::String str("asdf");
    str = '\0';
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("", str.c_str()));
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestAssignOperator4)
{
    ramses_capu::String str("asdf");
    ramses_capu::String other;
    str = other;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("", str.c_str()));
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestAssignOperator5)
{
    ramses_capu::String str("asdf");
    str = 'z';
    EXPECT_STREQ("z", str.c_str());
    EXPECT_EQ(1u, str.getLength());
}

TEST(String, TestAssignOperator6)
{
    ramses_capu::String str("asdf");
    str = '\0';
    EXPECT_STREQ("", str.c_str());
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestAppend1)
{
    ramses_capu::String str("hello");
    ramses_capu::String other("world");
    str.append(other);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("helloworld", str.c_str()));
    EXPECT_EQ(10u, str.getLength());
}

TEST(String, TestAppend2)
{
    ramses_capu::String str("hello");
    ramses_capu::String other;
    str.append(other);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str.c_str()));
    EXPECT_EQ(5u, str.getLength());
}

TEST(String, TestAppend3)
{
    ramses_capu::String str;
    ramses_capu::String other("world");
    str.append(other);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("world", str.c_str()));
    EXPECT_EQ(5u, str.getLength());
}

TEST(String, TestAppend4)
{
    ramses_capu::String str("hello");
    str.append("world");
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("helloworld", str.c_str()));
    EXPECT_EQ(10u, str.getLength());
}

TEST(String, TestAppend5)
{
    ramses_capu::String str;
    ramses_capu::String other;
    str.append(other);
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestPlusOperator1)
{
    ramses_capu::String str1("hello");
    ramses_capu::String str2("world");
    ramses_capu::String str3 = str1 + str2;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str1.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("world", str2.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("helloworld", str3.c_str()));
}

TEST(String, TestPlusOperator2)
{
    ramses_capu::String str1("hello");
    ramses_capu::String str2;
    ramses_capu::String str3 = str1 + str2;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str1.c_str()));
    EXPECT_EQ(0u, str2.getLength());
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str3.c_str()));
}

TEST(String, TestPlusOperator3)
{
    ramses_capu::String str1;
    ramses_capu::String str2("world");
    ramses_capu::String str3 = str1 + str2;
    EXPECT_EQ(0u, str1.getLength());
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("world", str2.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("world", str3.c_str()));
}

TEST(String, TestPlusOperator4)
{
    ramses_capu::String str1("hello");
    ramses_capu::String str2 = str1 + "world";
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str1.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("helloworld", str2.c_str()));
}

TEST(String, TestPlusOperator5)
{
    ramses_capu::String str1("world");
    ramses_capu::String str2 = "hello" + str1;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("world", str1.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("helloworld", str2.c_str()));
}

TEST(String, TestAddition)
{
    ramses_capu::String str1("hello");
    ramses_capu::String str2("world");
    ramses_capu::String str3 = str1 + str2;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str1.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("world", str2.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("helloworld", str3.c_str()));
}

TEST(String, TestAddAssignString)
{
    ramses_capu::String str ("abc");
    str += "def";

    EXPECT_EQ(6u, str.getLength());
    EXPECT_STREQ("abcdef", str.c_str());
}

TEST(String, TestAddAssignChar)
{
    ramses_capu::String str ("abc");
    str += 'd';

    EXPECT_EQ(4u, str.getLength());
    EXPECT_STREQ("abcd", str.c_str());
}

TEST(String, TestSubStringCTor1)
{
    ramses_capu::String str("0123456789", 4, 6);
    EXPECT_STREQ("456", str.c_str());
    EXPECT_EQ(3u, str.getLength());
}

TEST(String, TestSubStringCTor2)
{
    ramses_capu::String str("0123456789", 4);
    EXPECT_STREQ("456789", str.c_str());
    EXPECT_EQ(6u, str.getLength());
}

TEST(String, TestSubStringCTor3)
{
    ramses_capu::String str(static_cast<const char*>(0), 4);
    EXPECT_STREQ("", str.c_str());
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestSubStringCTor4)
{
    ramses_capu::String str(0, 4, 9);
    EXPECT_STREQ("", str.c_str());
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestSubStringCTor5)
{
    ramses_capu::String str(0, 9, 4);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("", str.c_str()));
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestSubStringCTor6)
{
    ramses_capu::String str("hello", 0, 100);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str.c_str()));
    EXPECT_EQ(5u, str.getLength());
}

TEST(String, TestSubStringCTorWithoutNullTermintor)
{
    const char someData[] = { '1', '2', '3', '4' };
    ramses_capu::String str(someData, 0, 2);

    EXPECT_EQ(3u, str.getLength());
    EXPECT_STREQ("123", str.c_str());
}

TEST(String, AccessOperator)
{
    ramses_capu::String str("abc");

    EXPECT_EQ('b', str[1]);

    str[1] = 'z';
    EXPECT_STREQ("azc", str.c_str());
}


TEST(String, AutoCast)
{
    ramses_capu::String string = "TestString";
    EXPECT_STREQ("TestString", string.c_str());
    EXPECT_EQ(10u, string.getLength());
}

TEST(String, Equals1)
{
    ramses_capu::String str1;
    ramses_capu::String str2;
    EXPECT_TRUE(str1 == str2);
    EXPECT_TRUE(str2 == str1);
}

TEST(String, Equals2)
{
    ramses_capu::String str1;
    ramses_capu::String str2("nonnull");
    EXPECT_FALSE(str1 == str2);
    EXPECT_FALSE(str2 == str1);
}

TEST(String, Equals3)
{
    ramses_capu::String str1("abc");
    ramses_capu::String str2("abc");
    EXPECT_TRUE(str1 == str2);
    EXPECT_FALSE(str1 != str2);
}

TEST(String, Equals4)
{
    ramses_capu::String str1("abc");
    EXPECT_TRUE(str1 == "abc");
    EXPECT_FALSE(str1 != "abc");
    EXPECT_TRUE("abc" == str1);
    EXPECT_FALSE("abc" != str1);
}

TEST(String, NotEquals1)
{
    ramses_capu::String str1;
    ramses_capu::String str2;
    EXPECT_FALSE(str1 != str2);
    EXPECT_FALSE(str2 != str1);

    str2 = "";
    EXPECT_FALSE(str1 != str2);
    EXPECT_FALSE(str2 != str1);

    str2 = "test";
    EXPECT_TRUE(str1 != str2);
    EXPECT_TRUE(str2 != str1);
}

TEST(String, NotEquals2)
{
    ramses_capu::String str1("hello");
    ramses_capu::String str2("world");
    ramses_capu::String str3("world");
    EXPECT_TRUE(str1 != str2);
    EXPECT_FALSE(str2 != str3);
}

TEST(String, NotEquals3)
{
    ramses_capu::String str1("hello");
    EXPECT_TRUE(str1 != "world");
    EXPECT_FALSE(str1 != "hello");
    EXPECT_TRUE("world" != str1);
    EXPECT_FALSE("hello" != str1);
}

TEST(String, SmallerGreater1)
{
    ramses_capu::String str1("Hello1");
    ramses_capu::String str2("Hello2");
    EXPECT_TRUE(str1 < str2);
    EXPECT_TRUE(str2 > str1);
}

TEST(String, SmallerGreater2)
{
    ramses_capu::String str1("Hello Long");
    ramses_capu::String str2("Hello Longer");
    EXPECT_TRUE(str1 < str2);
    EXPECT_TRUE(str2 > str1);
}

TEST(String, SmallerGreater3)
{
    ramses_capu::String str1("Hello Test");
    ramses_capu::String str2("Hello Test");
    EXPECT_FALSE(str1 < str2);
    EXPECT_FALSE(str2 > str1);
}

TEST(String, SmallerGreater4)
{
    ramses_capu::String str1;
    ramses_capu::String str2;
    EXPECT_FALSE(str1 < str2);
    EXPECT_FALSE(str2 > str1);
}

TEST(String, FindChar)
{
    ramses_capu::String str1;
    ramses_capu::String str2("hello world");
    ramses_capu::String str3("");

    EXPECT_EQ(-1, str1.find('a'));
    EXPECT_EQ(0, str1.find(0)); // indexof(0) = strlen
    EXPECT_EQ(4, str2.find('o'));
    EXPECT_EQ(11, str2.find(0)); // indexof(0) = strlen
    EXPECT_EQ(-1, str2.find('O'));
    EXPECT_EQ(-1, str3.find('o'));
    EXPECT_EQ(0, str3.find(0)); // indexof(0) = strlen
}

TEST(String, FindCharOffset)
{
    ramses_capu::String str1;
    ramses_capu::String str2("hello world");
    ramses_capu::String str3("");

    EXPECT_EQ(-1, str1.find('o', 12));
    EXPECT_EQ(-1, str1.find('o', 0));

    EXPECT_EQ(4, str2.find('o',  1));
    EXPECT_EQ(5, str2.find(' ', 0));
    EXPECT_EQ(7, str2.find('o', 5));

    EXPECT_EQ(-1, str2.find('o', 20));

}

TEST(String, FindStringOffset)
{
    ramses_capu::String str("hello world I am your old man");

    EXPECT_EQ(5, str.find(" ", 0));
    EXPECT_EQ(11, str.find(" ", 6));
    EXPECT_EQ(13, str.find(" ", 12));
    EXPECT_EQ(16, str.find(" ", 14));
    EXPECT_EQ(21, str.find(" ", 17));
    EXPECT_EQ(25, str.find(" ", 22));

    EXPECT_EQ(9, str.find("ld",  0));
    EXPECT_EQ(23, str.find("ld", 10));

    EXPECT_EQ(-1, str.find("ld", 25));

    EXPECT_EQ(-1, str.find("ld", 29));
    EXPECT_EQ(-1, str.find("ld", 30));

}

TEST(String, RFindChar)
{
    ramses_capu::String str1;
    ramses_capu::String str2("hello world");
    ramses_capu::String str3("");

    EXPECT_EQ(-1, str1.rfind('a'));
    EXPECT_EQ(0, str1.rfind(0)); // indexof(0) = strlen
    EXPECT_EQ(7, str2.rfind('o'));
    EXPECT_EQ(11, str2.rfind(0)); // indexof(0) = strlen
    EXPECT_EQ(-1, str2.rfind('O'));
    EXPECT_EQ(-1, str3.rfind('o'));
    EXPECT_EQ(0, str3.rfind(0)); // indexof(0) = strlen
}

TEST(String, Truncate)
{
    ramses_capu::String str1("hello world");
    str1.truncate(5);
    EXPECT_STREQ("hello", str1.c_str());
    EXPECT_EQ(5u, str1.getLength());

    // test chaining
    ramses_capu::String str2("hello world");
    str2.truncate(7).truncate(5);
    EXPECT_STREQ("hello", str2.c_str());
    EXPECT_EQ(5u, str2.getLength());

    // truncate to size of string
    ramses_capu::String str3("hello");
    str3.truncate(5);
    EXPECT_STREQ("hello", str3.c_str());
    EXPECT_EQ(5u, str3.getLength());

    // truncate to greater size of string
    ramses_capu::String str4("hello");
    str4.truncate(7);
    EXPECT_STREQ("hello", str4.c_str());
    EXPECT_EQ(5u, str4.getLength());

    // truncate with 0 makes empty string
    ramses_capu::String str5("hello");
    str5.truncate(0);
    EXPECT_STREQ("", str5.c_str());
    EXPECT_EQ(0u, str5.getLength());
}

TEST(String, FindSubstring)
{
    ramses_capu::String str1("hello c++ world.");
    EXPECT_EQ(6, str1.find("c++"));

    // test when the substring is at the start of the string
    ramses_capu::String str2("hello c++ world.");
    EXPECT_EQ(0, str2.find("hello"));

    // test when the substring is at the end of the string
    ramses_capu::String str3("hello c++ world.");
    EXPECT_EQ(10, str3.find("world."));

    // test substring not found
    ramses_capu::String str4("hello c++ world.");
    EXPECT_EQ(-1, str4.find("nosubstring"));

    // test substring is empty
    ramses_capu::String str5("hello c++ world.");
    EXPECT_EQ(0, str5.find(""));

    // test string is empty
    ramses_capu::String str6("");
    EXPECT_EQ(-1, str6.find("hello"));

    ramses_capu::String str7("");
    EXPECT_EQ(0, str7.find(""));
}

TEST(String, GetSubstring)
{
    ramses_capu::String str1("hello c++ world.");
    ramses_capu::String substr = str1.substr(0, 5);
    EXPECT_EQ(5u, substr.getLength());
    EXPECT_STREQ("hello", substr.c_str());

    // test when start is out of bounds
    ramses_capu::String substr2 = str1.substr(25, 5);
    EXPECT_EQ(0u, substr2.getLength());

    // test negative length
    ramses_capu::String substr3 = str1.substr(6, -1);
    EXPECT_STREQ("c++ world.", substr3.c_str());

    // test when length is too large
    ramses_capu::String substr4 = str1.substr(6, 500);
    EXPECT_STREQ("c++ world.", substr4.c_str());

    // take only the last character of the string
    ramses_capu::String substr5 = str1.substr(15, 5);
    EXPECT_STREQ(".", substr5.c_str());

    // length is 0
    ramses_capu::String substr6 = str1.substr(2, 0);
    EXPECT_STREQ("", substr6.c_str());

    // startPos and length is 0
    ramses_capu::String substr7 = str1.substr(0, 0);
    EXPECT_STREQ("", substr7.c_str());

    // start is exactly end of string
    ramses_capu::String substr8 = str1.substr(str1.getLength(), 4);
    EXPECT_STREQ("", substr8.c_str());
}

TEST(String, StartsWith)
{
    ramses_capu::String str("hello c++ world.");


    EXPECT_TRUE(str.startsWith("hello"));
    EXPECT_TRUE(str.startsWith("h"));
    EXPECT_TRUE(str.startsWith("hello c++ world."));
    EXPECT_FALSE(str.startsWith("c++"));

}

TEST(String, EndsWith)
{
    ramses_capu::String str("hello c++ world.");

    EXPECT_TRUE(str.endsWith("."));
    EXPECT_TRUE(str.endsWith("world."));
    EXPECT_TRUE(str.endsWith("hello c++ world."));
    EXPECT_FALSE(str.endsWith("c++"));

    ramses_capu::String path("D:");
    EXPECT_FALSE(path.endsWith("\\"));

    ramses_capu::String path2("D:\\dir1\\dir2");
    EXPECT_FALSE(path2.endsWith("\\"));

    ramses_capu::String path3("D:\\dir1\\dir2\\");
    EXPECT_TRUE(path3.endsWith("\\"));
}

TEST(String, ReplaceNoOccurence)
{
    ramses_capu::String str("hello c++ world.");
    ramses_capu::String result = str.replace("java", "c++");

    EXPECT_STREQ("hello c++ world.", result.c_str());
}

TEST(String, ReplaceSingleOccurence)
{
    ramses_capu::String str("hello c++ world.");
    ramses_capu::String result = str.replace("c++", "java");

    EXPECT_STREQ("hello java world.", result.c_str());
}

TEST(String, ReplaceMultipleOccurence)
{
    ramses_capu::String str("hello c++ world. hello java world.");
    ramses_capu::String result = str.replace("hello", "bye");

    EXPECT_STREQ("bye c++ world. bye java world.", result.c_str());
}

TEST(String, ReplaceOccurenceWithOffset)
{
    ramses_capu::String str("hello c++ world. hello java world.");
    ramses_capu::String result = str.replace("hello", "bye", 15);

    EXPECT_STREQ("hello c++ world. bye java world.", result.c_str());
}

TEST(String, TestDataGetterOnEmptyString)
{
    ramses_capu::String s;
    EXPECT_TRUE(s.data() == NULL);

    const ramses_capu::String sConst;
    EXPECT_TRUE(sConst.data() == NULL);
}

TEST(String, TestDataGetterOnNonEmptyString)
{
    ramses_capu::String s("a");
    EXPECT_EQ('a', *s.data());

    const ramses_capu::String sConst("a");
    EXPECT_EQ('a', *sConst.data());
}

TEST(String, ResizeAnEmptyString)
{
    ramses_capu::String s;
    EXPECT_EQ(0u, s.getLength());

    s.resize(15);
    EXPECT_EQ(15u, s.getLength());
}

TEST(String, ResizeToSmaller)
{
    ramses_capu::String s("12345");
    s.resize(3);
    EXPECT_EQ(3u, s.getLength());
}

TEST(String, IsNullterminatedAlsoAfterResizing)
{
    ramses_capu::String s("1234");
    s.resize(2);

    char destination[3];

    ramses_capu::Memory::Copy(&destination, s.c_str(), 3);
    ASSERT_EQ(0, destination[2]);
}


TEST(String, IsNullterminatedAlsoAfterResizingToZero)
{
    ramses_capu::String s("1234");
    s.resize(0);

    char destination;

    ramses_capu::Memory::Copy(&destination, s.c_str(), 1);
    ASSERT_EQ(0, destination);
}

TEST(String, ResizeReallyAllocatesEnough)
{
    ramses_capu::String s("12");
    s.resize(6);
    const char* source = "12345\0";
    ramses_capu::Memory::Copy(s.data(), source, 6);

    EXPECT_STREQ("12345", s.data());
}

TEST(String, ReservePreventReallocAndCapacityChange)
{
    for (ramses_capu::uint_t i = 5; i < 100; ++i)
    {
        ramses_capu::String s("A");
        s.reserve(i);
        const ramses_capu::uint_t capacityBefore = s.capacity();
        const std::uintptr_t ptrBefore = reinterpret_cast<std::uintptr_t>(s.c_str());
        s.resize(i);
        const ramses_capu::uint_t capacityAfter = s.capacity();
        const std::uintptr_t ptrAfter = reinterpret_cast<std::uintptr_t>(s.c_str());
        EXPECT_EQ(capacityBefore, capacityAfter);
        EXPECT_EQ(ptrBefore, ptrAfter);
    }
}

TEST(String, reserveOnEmptyStringSetCapacityOnAtLeastThisValue)
{
    ramses_capu::String s;
    EXPECT_EQ(0u, s.capacity());
    s.reserve(10);
    EXPECT_GE(s.capacity(), 10u);
}

TEST(String, reserveMoreOnNonOnEmptyStringSetCapacityOnAtLeastThisValue)
{
    ramses_capu::String s("foobar");
    const ramses_capu::uint_t size = s.getLength();
    const ramses_capu::uint_t capacity = s.capacity();
    s.reserve(40);
    EXPECT_EQ(size, s.getLength());
    EXPECT_GE(s.capacity(), capacity);
}

TEST(String, reserveLessThanSizeDoesNothing)
{
    ramses_capu::String s("foobar");
    const ramses_capu::uint_t size = s.getLength();
    const ramses_capu::uint_t capacity = s.capacity();
    s.reserve(2);
    EXPECT_EQ(size, s.getLength());
    EXPECT_EQ(s.capacity(), capacity);
}
