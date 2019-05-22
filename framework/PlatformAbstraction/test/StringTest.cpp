//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "Collections/String.h"
#include "Collections/HashMap.h"
#include "ramses-capu/os/Memory.h"

namespace ramses_internal
{

TEST(String, TestCTor)
{
    String str;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("", str.c_str()));
}

TEST(String, TestCStr)
{
    String str("asdf");
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("asdf", str.c_str()));
    EXPECT_EQ(4u, str.getLength());
}

TEST(String, TestCopyConstructor)
{
    String str("asdf");
    String copy(str);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("asdf", str.c_str()));
    EXPECT_STREQ("asdf", str.c_str());
    EXPECT_EQ(str.getLength(), copy.getLength());
}

TEST(String, InitialSizeConstructor)
{
    String str(5, 'a');

    EXPECT_EQ(5u, str.getLength());
    EXPECT_STREQ("aaaaa", str.c_str());
}

TEST(String, ConstructWithEmptyStringIsEmpty)
{
    String str("");
    EXPECT_EQ(0u, str.getLength());
    EXPECT_STREQ("", str.c_str());
}

TEST(String, TestAssignOperator1)
{
    String str("asdf");
    String other("other");

    EXPECT_EQ(4u, str.getLength());
    EXPECT_EQ(5u, other.getLength());
    str = other;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("other", str.c_str()));
    EXPECT_EQ(5u, str.getLength());
    EXPECT_EQ(5u, other.getLength());

    String str2;
    String other2;
    str2 = other2;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("", str2.c_str()));

    // one string on stack, one on heap
    String stringStack1;
    String stringStack2;
    String *stringHeap1 = new String();
    String *stringHeap2 = new String();


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
    String a("abc");
    String b(std::move(a));

    EXPECT_EQ(String("abc"), b);
    EXPECT_EQ(3u, b.getLength());
}

TEST(String, MoveAssignment)
{
    String a("abc");
    String b;
    b = std::move(a);

    EXPECT_EQ(String("abc"), b);
    EXPECT_EQ(3u, b.getLength());
}

TEST(String, TestToLowerCase1)
{
    String str("abcDEF");
    str.toLowerCase();
    EXPECT_EQ(String("abcdef"), str);
}

TEST(String, TestToLowerCase2)
{
    String str("");
    str.toLowerCase();
    EXPECT_EQ(String(""), str);
}

TEST(String, TestToLowerCase3)
{
    String str("ABC");
    str.toLowerCase();
    EXPECT_EQ(String("abc"), str);
}

TEST(String, TestToLowerCase4)
{
    String str("AbC");
    str.toLowerCase();
    EXPECT_EQ(String("abc"), str);
}

TEST(String, TestToLowerCase5)
{
    String str("Abc");
    str.toLowerCase();
    EXPECT_EQ(String("abc"), str);
}

TEST(String, TestToUpperCase1)
{
    String str("abcDEF");
    str.toUpperCase();
    EXPECT_EQ(String("ABCDEF"), str);
}

TEST(String, TestToUpperCase2)
{
    String str("");
    str.toUpperCase();
    EXPECT_EQ(String(""), str);
}

TEST(String, TestToUpperCase3)
{
    String str("ABC");
    str.toUpperCase();
    EXPECT_EQ(String("ABC"), str);
}

TEST(String, TestToUpperCase4)
{
    String str("AbC");
    str.toUpperCase();
    EXPECT_EQ(String("ABC"), str);
}

TEST(String, TestToUpperCase5)
{
    String str("Abc");
    str.toUpperCase();
    EXPECT_EQ(String("ABC"), str);
}

TEST(String, TestAssignOperator2)
{
    String str("asdf");
    str = "other";
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("other", str.c_str()));
    EXPECT_EQ(5u, str.getLength());
}

TEST(String, TestAssignOperator3)
{
    String str("asdf");
    str = '\0';
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("", str.c_str()));
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestAssignOperator4)
{
    String str("asdf");
    String other;
    str = other;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("", str.c_str()));
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestAssignOperator5)
{
    String str("asdf");
    str = 'z';
    EXPECT_STREQ("z", str.c_str());
    EXPECT_EQ(1u, str.getLength());
}

TEST(String, TestAssignOperator6)
{
    String str("asdf");
    str = '\0';
    EXPECT_STREQ("", str.c_str());
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestAppend1)
{
    String str("hello");
    String other("world");
    str.append(other);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("helloworld", str.c_str()));
    EXPECT_EQ(10u, str.getLength());
}

TEST(String, TestAppend2)
{
    String str("hello");
    String other;
    str.append(other);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str.c_str()));
    EXPECT_EQ(5u, str.getLength());
}

TEST(String, TestAppend3)
{
    String str;
    String other("world");
    str.append(other);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("world", str.c_str()));
    EXPECT_EQ(5u, str.getLength());
}

TEST(String, TestAppend4)
{
    String str("hello");
    str.append("world");
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("helloworld", str.c_str()));
    EXPECT_EQ(10u, str.getLength());
}

TEST(String, TestAppend5)
{
    String str;
    String other;
    str.append(other);
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestPlusOperator1)
{
    String str1("hello");
    String str2("world");
    String str3 = str1 + str2;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str1.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("world", str2.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("helloworld", str3.c_str()));
}

TEST(String, TestPlusOperator2)
{
    String str1("hello");
    String str2;
    String str3 = str1 + str2;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str1.c_str()));
    EXPECT_EQ(0u, str2.getLength());
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str3.c_str()));
}

TEST(String, TestPlusOperator3)
{
    String str1;
    String str2("world");
    String str3 = str1 + str2;
    EXPECT_EQ(0u, str1.getLength());
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("world", str2.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("world", str3.c_str()));
}

TEST(String, TestPlusOperator4)
{
    String str1("hello");
    String str2 = str1 + "world";
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str1.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("helloworld", str2.c_str()));
}

TEST(String, TestPlusOperator5)
{
    String str1("world");
    String str2 = "hello" + str1;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("world", str1.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("helloworld", str2.c_str()));
}

TEST(String, TestAddition)
{
    String str1("hello");
    String str2("world");
    String str3 = str1 + str2;
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str1.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("world", str2.c_str()));
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("helloworld", str3.c_str()));
}

TEST(String, TestAddAssignString)
{
    String str ("abc");
    str += "def";

    EXPECT_EQ(6u, str.getLength());
    EXPECT_STREQ("abcdef", str.c_str());
}

TEST(String, TestAddAssignChar)
{
    String str ("abc");
    str += 'd';

    EXPECT_EQ(4u, str.getLength());
    EXPECT_STREQ("abcd", str.c_str());
}

TEST(String, TestSubStringCTor1)
{
    String str("0123456789", 4, 6);
    EXPECT_STREQ("456", str.c_str());
    EXPECT_EQ(3u, str.getLength());
}

TEST(String, TestSubStringCTor2)
{
    String str("0123456789", 4);
    EXPECT_STREQ("456789", str.c_str());
    EXPECT_EQ(6u, str.getLength());
}

TEST(String, TestSubStringCTor3)
{
    String str(static_cast<const char*>(0), 4);
    EXPECT_STREQ("", str.c_str());
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestSubStringCTor4)
{
    String str(0, 4, 9);
    EXPECT_STREQ("", str.c_str());
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestSubStringCTor5)
{
    String str(0, 9, 4);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("", str.c_str()));
    EXPECT_EQ(0u, str.getLength());
}

TEST(String, TestSubStringCTor6)
{
    String str("hello", 0, 100);
    EXPECT_EQ(0, ramses_capu::StringUtils::Strcmp("hello", str.c_str()));
    EXPECT_EQ(5u, str.getLength());
}

TEST(String, TestSubStringCTorWithoutNullTermintor)
{
    const char someData[] = { '1', '2', '3', '4' };
    String str(someData, 0, 2);

    EXPECT_EQ(3u, str.getLength());
    EXPECT_STREQ("123", str.c_str());
}

TEST(String, AccessOperator)
{
    String str("abc");

    EXPECT_EQ('b', str[1]);

    str[1] = 'z';
    EXPECT_STREQ("azc", str.c_str());
}


TEST(String, AutoCast)
{
    String string = "TestString";
    EXPECT_STREQ("TestString", string.c_str());
    EXPECT_EQ(10u, string.getLength());
}

TEST(String, Equals1)
{
    String str1;
    String str2;
    EXPECT_TRUE(str1 == str2);
    EXPECT_TRUE(str2 == str1);
}

TEST(String, Equals2)
{
    String str1;
    String str2("nonnull");
    EXPECT_FALSE(str1 == str2);
    EXPECT_FALSE(str2 == str1);
}

TEST(String, Equals3)
{
    String str1("abc");
    String str2("abc");
    EXPECT_TRUE(str1 == str2);
    EXPECT_FALSE(str1 != str2);
}

TEST(String, Equals4)
{
    String str1("abc");
    EXPECT_TRUE(str1 == "abc");
    EXPECT_FALSE(str1 != "abc");
    EXPECT_TRUE("abc" == str1);
    EXPECT_FALSE("abc" != str1);
}

TEST(String, NotEquals1)
{
    String str1;
    String str2;
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
    String str1("hello");
    String str2("world");
    String str3("world");
    EXPECT_TRUE(str1 != str2);
    EXPECT_FALSE(str2 != str3);
}

TEST(String, NotEquals3)
{
    String str1("hello");
    EXPECT_TRUE(str1 != "world");
    EXPECT_FALSE(str1 != "hello");
    EXPECT_TRUE("world" != str1);
    EXPECT_FALSE("hello" != str1);
}

TEST(String, SmallerGreater1)
{
    String str1("Hello1");
    String str2("Hello2");
    EXPECT_TRUE(str1 < str2);
    EXPECT_TRUE(str2 > str1);
}

TEST(String, SmallerGreater2)
{
    String str1("Hello Long");
    String str2("Hello Longer");
    EXPECT_TRUE(str1 < str2);
    EXPECT_TRUE(str2 > str1);
}

TEST(String, SmallerGreater3)
{
    String str1("Hello Test");
    String str2("Hello Test");
    EXPECT_FALSE(str1 < str2);
    EXPECT_FALSE(str2 > str1);
}

TEST(String, SmallerGreater4)
{
    String str1;
    String str2;
    EXPECT_FALSE(str1 < str2);
    EXPECT_FALSE(str2 > str1);
}

TEST(String, FindChar)
{
    String str1;
    String str2("hello world");
    String str3("");

    EXPECT_EQ(-1, str1.find('a'));
    EXPECT_EQ(4, str2.find('o'));
    EXPECT_EQ(-1, str2.find('O'));
    EXPECT_EQ(-1, str3.find('o'));
}

TEST(String, FindCharOffset)
{
    String str1;
    String str2("hello world");
    String str3("");

    EXPECT_EQ(-1, str1.find('o', 12));
    EXPECT_EQ(-1, str1.find('o', 0));

    EXPECT_EQ(4, str2.find('o',  1));
    EXPECT_EQ(5, str2.find(' ', 0));
    EXPECT_EQ(7, str2.find('o', 5));

    EXPECT_EQ(-1, str2.find('o', 20));

}

TEST(String, FindStringOffset)
{
    String str("hello world I am your old man");

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
    String str1;
    String str2("hello world");
    String str3("");

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
    String str1("hello world");
    str1.truncate(5);
    EXPECT_STREQ("hello", str1.c_str());
    EXPECT_EQ(5u, str1.getLength());

    // test chaining
    String str2("hello world");
    str2.truncate(7).truncate(5);
    EXPECT_STREQ("hello", str2.c_str());
    EXPECT_EQ(5u, str2.getLength());

    // truncate to size of string
    String str3("hello");
    str3.truncate(5);
    EXPECT_STREQ("hello", str3.c_str());
    EXPECT_EQ(5u, str3.getLength());

    // truncate to greater size of string
    String str4("hello");
    str4.truncate(7);
    EXPECT_STREQ("hello", str4.c_str());
    EXPECT_EQ(5u, str4.getLength());

    // truncate with 0 makes empty string
    String str5("hello");
    str5.truncate(0);
    EXPECT_STREQ("", str5.c_str());
    EXPECT_EQ(0u, str5.getLength());
}

TEST(String, FindSubstring)
{
    String str1("hello c++ world.");
    EXPECT_EQ(6, str1.find("c++"));

    // test when the substring is at the start of the string
    String str2("hello c++ world.");
    EXPECT_EQ(0, str2.find("hello"));

    // test when the substring is at the end of the string
    String str3("hello c++ world.");
    EXPECT_EQ(10, str3.find("world."));

    // test substring not found
    String str4("hello c++ world.");
    EXPECT_EQ(-1, str4.find("nosubstring"));

    // test substring is empty
    String str5("hello c++ world.");
    EXPECT_EQ(0, str5.find(""));

    // test string is empty
    String str6("");
    EXPECT_EQ(-1, str6.find("hello"));

    String str7("");
    EXPECT_EQ(0, str7.find(""));
}

TEST(String, GetSubstring)
{
    String str1("hello c++ world.");
    String substr = str1.substr(0, 5);
    EXPECT_EQ(5u, substr.getLength());
    EXPECT_STREQ("hello", substr.c_str());

    // test when start is out of bounds
    String substr2 = str1.substr(25, 5);
    EXPECT_EQ(0u, substr2.getLength());

    // test negative length
    String substr3 = str1.substr(6, -1);
    EXPECT_STREQ("c++ world.", substr3.c_str());

    // test when length is too large
    String substr4 = str1.substr(6, 500);
    EXPECT_STREQ("c++ world.", substr4.c_str());

    // take only the last character of the string
    String substr5 = str1.substr(15, 5);
    EXPECT_STREQ(".", substr5.c_str());

    // length is 0
    String substr6 = str1.substr(2, 0);
    EXPECT_STREQ("", substr6.c_str());

    // startPos and length is 0
    String substr7 = str1.substr(0, 0);
    EXPECT_STREQ("", substr7.c_str());

    // start is exactly end of string
    String substr8 = str1.substr(str1.getLength(), 4);
    EXPECT_STREQ("", substr8.c_str());
}

TEST(String, StartsWith)
{
    String str("hello c++ world.");


    EXPECT_TRUE(str.startsWith("hello"));
    EXPECT_TRUE(str.startsWith("h"));
    EXPECT_TRUE(str.startsWith("hello c++ world."));
    EXPECT_FALSE(str.startsWith("c++"));

}

TEST(String, EndsWith)
{
    String str("hello c++ world.");

    EXPECT_TRUE(str.endsWith("."));
    EXPECT_TRUE(str.endsWith("world."));
    EXPECT_TRUE(str.endsWith("hello c++ world."));
    EXPECT_FALSE(str.endsWith("c++"));

    String path("D:");
    EXPECT_FALSE(path.endsWith("\\"));

    String path2("D:\\dir1\\dir2");
    EXPECT_FALSE(path2.endsWith("\\"));

    String path3("D:\\dir1\\dir2\\");
    EXPECT_TRUE(path3.endsWith("\\"));
}

TEST(String, TestDataGetterOnEmptyString)
{
    // expect SSO
    String s;
    EXPECT_TRUE(s.data() != nullptr);

    const String sConst;
    EXPECT_TRUE(sConst.data() != nullptr);
}

TEST(String, TestDataGetterOnNonEmptyString)
{
    String s("a");
    EXPECT_EQ('a', *s.data());

    const String sConst("a");
    EXPECT_EQ('a', *sConst.data());
}

TEST(String, ResizeAnEmptyString)
{
    String s;
    EXPECT_EQ(0u, s.getLength());

    s.resize(15);
    EXPECT_EQ(15u, s.getLength());
}

TEST(String, ResizeToSmaller)
{
    String s("12345");
    s.resize(3);
    EXPECT_EQ(3u, s.getLength());
}

TEST(String, IsNullterminatedAlsoAfterResizing)
{
    String s("1234");
    s.resize(2);

    char destination[3];

    ramses_capu::Memory::Copy(&destination, s.c_str(), 3);
    ASSERT_EQ(0, destination[2]);
}


TEST(String, IsNullterminatedAlsoAfterResizingToZero)
{
    String s("1234");
    s.resize(0);

    char destination;

    ramses_capu::Memory::Copy(&destination, s.c_str(), 1);
    ASSERT_EQ(0, destination);
}

TEST(String, ResizeReallyAllocatesEnough)
{
    String s("12");
    s.resize(6);
    const char* source = "12345\0";
    ramses_capu::Memory::Copy(s.data(), source, 6);

    EXPECT_STREQ("12345", s.data());
}

TEST(String, ReservePreventReallocAndCapacityChange)
{
    for (ramses_capu::uint_t i = 5; i < 100; ++i)
    {
        String s("A");
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
    String s;
    s.reserve(10);
    EXPECT_GE(s.capacity(), 10u);
}

TEST(String, reserveMoreOnNonOnEmptyStringSetCapacityOnAtLeastThisValue)
{
    String s("foobar");
    const ramses_capu::uint_t size = s.getLength();
    const ramses_capu::uint_t capacity = s.capacity();
    s.reserve(40);
    EXPECT_EQ(size, s.getLength());
    EXPECT_GE(s.capacity(), capacity);
}

TEST(String, reserveLessThanSizeDoesNothing)
{
    String s("foobar");
    const ramses_capu::uint_t size = s.getLength();
    const ramses_capu::uint_t capacity = s.capacity();
    s.reserve(2);
    EXPECT_EQ(size, s.getLength());
    EXPECT_EQ(s.capacity(), capacity);
}

TEST(String, HashMapWithString)
{
    HashMap<String, int32_t> map;

    map.put("testFloat", 3);
    map.put("testFloat4", 4);

    EXPECT_EQ(3, map["testFloat"]);
    EXPECT_EQ(4, map["testFloat4"]);

    // Test clear
    map.clear();
    EXPECT_EQ(static_cast<uint32_t>(0), map.count());
}

TEST(String, HashMapWithStdString)
{
    HashMap<std::string, int32_t> map;

    map.put("testFloat", 3);
    map.put("testFloat4", 4);

    EXPECT_EQ(3, map["testFloat"]);
    EXPECT_EQ(4, map["testFloat4"]);

    // Test clear
    map.clear();
    EXPECT_EQ(static_cast<uint32_t>(0), map.count());
}


}
