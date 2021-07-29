//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Collections/StringOutputStream.h"
#include "Collections/String.h"
#include "framework_common_gmock_header.h"
#include "UnsafeTestMemoryHelpers.h"
#include "gmock/gmock.h"


namespace ramses_internal
{
    TEST(AStringOutputStream, WriteFloat)
    {
        StringOutputStream outputStream;
        outputStream << 47.11f;
        EXPECT_EQ("47.11", outputStream.data());
        EXPECT_EQ(5U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteFloatMaximumNegative)
    {
        StringOutputStream outputStream;
        outputStream << -std::numeric_limits<float>::max();
        EXPECT_EQ("-3.4028235e+38", outputStream.data());
    }

    TEST(AStringOutputStream, WriteFloatZero)
    {
        StringOutputStream outputStream;
        outputStream << 0.f;
        EXPECT_EQ("0", outputStream.data());
    }

    TEST(AStringOutputStream, WriteFloatSmallestNegative)
    {
        StringOutputStream outputStream;
        outputStream << (-1.f / (std::numeric_limits<float>::max() - 1));
        EXPECT_EQ("-2.938736e-39", outputStream.data());
    }

    TEST(AStringOutputStream, WriteInt32)
    {
        StringOutputStream outputStream;
        outputStream << -1059192060;
        EXPECT_STREQ("-1059192060", outputStream.c_str());
        EXPECT_EQ(11U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteUInt32)
    {
        StringOutputStream outputStream;
        outputStream << 4711u;
        EXPECT_STREQ("4711", outputStream.c_str());
        EXPECT_EQ(4U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteInt64)
    {
        StringOutputStream outputStream;
        int64_t value = 0x6464646432323232LL;
        outputStream << value;
        EXPECT_STREQ("7234017282965516850", outputStream.c_str());
        EXPECT_EQ(19U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteUInt64)
    {
        StringOutputStream outputStream;
        uint64_t value = 0x6464646432323232uLL;
        outputStream << value;
        EXPECT_STREQ("7234017282965516850", outputStream.c_str());
        EXPECT_EQ(19U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteString)
    {
        StringOutputStream outputStream;
        outputStream << String("Hello World");
        EXPECT_STREQ("Hello World", outputStream.c_str());
        EXPECT_EQ(11U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteStdString)
    {
        StringOutputStream outputStream;
        outputStream << std::string("Hello World");
        EXPECT_STREQ("Hello World", outputStream.c_str());
        EXPECT_EQ(11U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteBoolTrue)
    {
        StringOutputStream outputStream;
        outputStream << true;
        EXPECT_EQ("true", outputStream.data());
    }

    TEST(AStringOutputStream, WriteBoolFalse)
    {
        StringOutputStream outputStream;
        outputStream << false;
        EXPECT_EQ("false", outputStream.data());
    }

    TEST(AStringOutputStream, WriteCharArray)
    {
        StringOutputStream outputStream;
        outputStream << "Hello World";
        EXPECT_EQ("Hello World", outputStream.data());
    }

    TEST(AStringOutputStream, WriteNullCharArray)
    {
        StringOutputStream outputStream;
        const char* str = nullptr;
        outputStream << str;
        EXPECT_EQ("", outputStream.data());
    }

    TEST(AStringOutputStream, WriteChar)
    {
        StringOutputStream outputStream;
        char c = 'T';
        outputStream << c;
        EXPECT_EQ("T", outputStream.data());
    }

    TEST(AStringOutputStream, WriteUInt16)
    {
        StringOutputStream outputStream;
        outputStream << static_cast<uint16_t>(4711);
        EXPECT_EQ("4711", outputStream.data());
    }

    TEST(AStringOutputStream, AutoFlush)
    {
        StringOutputStream outputStream;
        outputStream << "Some data";
        EXPECT_STREQ("Some data", outputStream.c_str());
        EXPECT_EQ(9u, outputStream.size());
    }

    TEST(AStringOutputStream, Flush)
    {
        StringOutputStream outputStream;
        outputStream << 4711;
        EXPECT_EQ(4U, outputStream.size());
    }

    TEST(AStringOutputStream, MultipleData)
    {
        StringOutputStream outputStream;
        outputStream << 4711 << " " << 47.11f << " " << "Hello World" << " " << true;
        EXPECT_EQ("4711 47.11 Hello World true", outputStream.data());
    }

    TEST(AStringOutputStream, Resize)
    {
        StringOutputStream outputStream;
        outputStream << "Exactly 16 chars";
        EXPECT_STREQ("Exactly 16 chars", outputStream.c_str());
        EXPECT_EQ(16U, outputStream.size());
    }

    TEST(AStringOutputStream, C_StrIsNullTerminatedForConstStreams)
    {
        StringOutputStream outputStream;
        outputStream << "a";
        const StringOutputStream& constStream = outputStream;
        EXPECT_STREQ("a", constStream.c_str());
    }

    TEST(AStringOutputStream, hasLengthZeroIntially)
    {
        StringOutputStream stream;
        EXPECT_EQ(0u, stream.size());
    }

    TEST(AStringOutputStream, returnsCorrectLength)
    {
        StringOutputStream stream;
        stream << "12345";
        EXPECT_EQ(5u, stream.size());
    }

    TEST(AStringOutputStream, PrintsNullVoidPointerAsHex)
    {
        StringOutputStream stream;
        void* nullPointer = nullptr;
        stream << nullPointer;
        EXPECT_EQ("0x0", stream.data());
    }

    TEST(AStringOutputStream, PrintsNonNullVoidPointerAsHex)
    {
        StringOutputStream stream;
        void* forgedPointer = UnsafeTestMemoryHelpers::ForgeArbitraryPointer(1024);
        stream << forgedPointer;
        EXPECT_EQ("0x400", stream.data());
    }

    TEST(AStringOutputStream, releaseReturnsStringAndClearsStream)
    {
        StringOutputStream stream;

        stream << "foo";
        String s1(stream.release());
        EXPECT_EQ(String("foo"), s1);
        EXPECT_STREQ("", stream.c_str());
        EXPECT_EQ(0u, stream.size());

        stream << "bar";
        EXPECT_STREQ("bar", stream.c_str());
        EXPECT_EQ(3u, stream.size());
    }

    TEST(AStringOutputStream, CanGetUnderlayingString)
    {
        StringOutputStream stream;
        stream << "foo";
        EXPECT_EQ("foo", stream.data());
        stream << "bar";
        EXPECT_EQ("foobar", stream.data());
    }

    TEST(AStringOutputStream, CanConstructWithStdString)
    {
        StringOutputStream stream(std::string("foo"));
        stream << "bar";
        EXPECT_EQ("foobar", stream.data());
    }
}
