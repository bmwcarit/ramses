//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Collections/StringOutputStream.h"
#include "Math3d/Matrix44f.h"
#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "gmock/gmock-matchers.h"


namespace ramses_internal
{
    using namespace testing;
    using ::testing::AnyOf;

    TEST(AStringOutputStream, WriteFloatWithDefaultPrecision)
    {
        StringOutputStream outputStream;
        outputStream << 47.11f;
        EXPECT_STREQ("47.110001", outputStream.c_str());
        EXPECT_EQ(9U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteFloatMaximumNegativeWithDefaultPrecision)
    {
        StringOutputStream outputStream;
        outputStream << -std::numeric_limits<float>::max();
        EXPECT_EQ(47U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteFloatZeroWithMaximumPrecision)
    {
        StringOutputStream outputStream;
        outputStream.setDecimalDigits(45);
        outputStream << 0.f;
        EXPECT_EQ(2U + 45U, outputStream.size()); // '0.' + precision
    }

    TEST(AStringOutputStream, WriteFloatZeroWithCappedToMaximumPrecision)
    {
        StringOutputStream outputStream;
        outputStream.setDecimalDigits(46);
        outputStream << 0.f;
        EXPECT_EQ(2U + 45U, outputStream.size()); // '0.' + precision
    }

    TEST(AStringOutputStream, WriteFloatSmallestNegativeWithMaximumPrecision)
    {
        StringOutputStream outputStream;
        outputStream.setDecimalDigits(45);
        outputStream << (-1.f / (std::numeric_limits<float>::max() - 1));
        EXPECT_EQ(3U + 45U, outputStream.size()); // '-' + '0.' + precision
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
        EXPECT_STREQ("true", outputStream.c_str());
        EXPECT_EQ(4U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteBoolFalse)
    {
        StringOutputStream outputStream;
        outputStream << false;
        EXPECT_STREQ("false", outputStream.c_str());
        EXPECT_EQ(5U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteCharArray)
    {
        StringOutputStream outputStream;
        outputStream << "Hello World";
        EXPECT_STREQ("Hello World", outputStream.c_str());
        EXPECT_EQ(11U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteChar)
    {
        StringOutputStream outputStream;
        char c = 'T';
        outputStream << c;
        EXPECT_STREQ("T", outputStream.c_str());
        EXPECT_EQ(1U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteUInt16)
    {
        StringOutputStream outputStream;
        outputStream << static_cast<uint16_t>(4711);
        EXPECT_STREQ("4711", outputStream.c_str());
        EXPECT_EQ(4U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteFixed)
    {
        StringOutputStream outputStream;
        outputStream << 47.11f;
        EXPECT_STREQ("47.110001", outputStream.c_str());
        EXPECT_EQ(9u, outputStream.size());
        outputStream.clear();
        outputStream.setFloatingPointType(StringOutputStream::EFloatingPointType_Fixed);
        outputStream << 47.11f;
        EXPECT_STREQ("47.1100", outputStream.c_str());
        EXPECT_EQ(7u, outputStream.size());
    }

    TEST(AStringOutputStream, Clear)
    {
        StringOutputStream outputStream;
        outputStream << "Some data";
        outputStream.clear();
        EXPECT_STREQ("", outputStream.c_str());
        EXPECT_EQ(0u, outputStream.size());

        outputStream << "Some data";
        outputStream.clear();
        EXPECT_STREQ("", outputStream.c_str());
        EXPECT_EQ(0u, outputStream.size());
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
        EXPECT_STREQ("4711 47.110001 Hello World true", outputStream.c_str());
        EXPECT_EQ(31U, outputStream.size());
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

    TEST(AStringOutputStream, WriteUInt32HexNoLeadingZero)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexNoLeadingZeros);
        uint32_t value = 4446222;
        outputStream << value;
        EXPECT_STREQ("43D80E", outputStream.c_str());
        EXPECT_EQ(6U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteUInt32HexLeadingZero)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexLeadingZeros);
        uint32_t value = 4446222;
        outputStream << value;
        EXPECT_STREQ("0043D80E", outputStream.c_str());
        EXPECT_EQ(8U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteInt32HexNoLeadingZero)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexNoLeadingZeros);
        int32_t value = 4446222;
        outputStream << value;
        EXPECT_STREQ("43D80E", outputStream.c_str());
        EXPECT_EQ(6U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteInt32HexLeadingZero)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexLeadingZeros);
        int32_t value = 4446222;
        outputStream << value;
        EXPECT_STREQ("0043D80E", outputStream.c_str());
        EXPECT_EQ(8U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteInt32HexNegativeValue)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexLeadingZeros);
        int32_t value = -4446222;
        outputStream << value;
        EXPECT_STREQ("FFBC27F2", outputStream.c_str());//uint32_max - value
        EXPECT_EQ(8U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteUInt64HexNoLeadingZero)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexNoLeadingZeros);
        uint64_t value = 353544462511222;
        outputStream << value;
        EXPECT_STREQ("1418BFC19AC76", outputStream.c_str());
        EXPECT_EQ(13U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteUInt64HexLeadingZero)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexLeadingZeros);
        uint64_t value = 353544462511222;
        outputStream << value;
        EXPECT_STREQ("0001418BFC19AC76", outputStream.c_str());
        EXPECT_EQ(16U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteInt64HexNoLeadingZero)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexNoLeadingZeros);
        int64_t value = 353544462511222;
        outputStream << value;
        EXPECT_STREQ("1418BFC19AC76", outputStream.c_str());
        EXPECT_EQ(13U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteInt64HexLeadingZero)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexLeadingZeros);
        int64_t value = 353544462511222;
        outputStream << value;
        EXPECT_STREQ("0001418BFC19AC76", outputStream.c_str());
        EXPECT_EQ(16U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteInt64HexNegativeValue)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexLeadingZeros);
        int64_t value = -353544462511222;
        outputStream << value;
        EXPECT_STREQ("FFFEBE7403E6538A", outputStream.c_str());//uint64_max - value
        EXPECT_EQ(16U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteUInt16HexLeadingZero)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexLeadingZeros);
        uint16_t value = 1337;
        outputStream << value;
        EXPECT_STREQ("0539", outputStream.c_str());
        EXPECT_EQ(4U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteUInt16HexNoLeadingZero)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexNoLeadingZeros);
        uint16_t value = 1337;
        outputStream << value;
        EXPECT_STREQ("539", outputStream.c_str());
        EXPECT_EQ(3U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteInt16HexLeadingZero)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexLeadingZeros);
        uint16_t value = 1337;
        outputStream << value;
        EXPECT_STREQ("0539", outputStream.c_str());
        EXPECT_EQ(4U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteInt16HexNoLeadingZero)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexNoLeadingZeros);
        uint16_t value = 1337;
        outputStream << value;
        EXPECT_STREQ("539", outputStream.c_str());
        EXPECT_EQ(3U, outputStream.size());
    }

    TEST(AStringOutputStream, WriteInt16HexNegativeValue)
    {
        StringOutputStream outputStream;
        outputStream.setHexadecimalOutputFormat(StringOutputStream::EHexadecimalType_HexLeadingZeros);
        int16_t value = -1337;
        outputStream << value;
        EXPECT_STREQ("FAC7", outputStream.c_str());//uint16_max - value
        EXPECT_EQ(4U, outputStream.size());
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

        EXPECT_THAT(stream.c_str(), AnyOf(StrEq("0"), StrEq("0x0"), StrEq("00000000"), StrEq("0000000000000000"), StrEq("(nil)")));
    }

    TEST(AStringOutputStream, PrintsNonNullVoidPointerAsHex)
    {
        StringOutputStream stream;
        void* forgedPointer = reinterpret_cast<void*>(1024);
        stream << forgedPointer;

        EXPECT_THAT(stream.c_str(), AnyOf(StrEq("0x400"), StrEq("00000400"), StrEq("0000000000000400")));
    }

    TEST(AStringOutputStream, Matrix44f)
    {
        StringOutputStream stream;

        Matrix44f m(1.0f, 5.0f, 9.0f, 13.0f, 2.0f, 6.0f, 10.0f, 14.0f, 3.0f, 7.0f, 11.0f, 15.0f, 4.0f, 8.0f, 12.0f,
                    16.0f);

        stream << m;

        EXPECT_STREQ("1.000000 2.000000 3.000000 4.000000 5.000000 6.000000 7.000000 8.000000 9.000000 10.000000 11.000000 12.000000 13.000000 14.000000 15.000000 16.000000", stream.c_str());
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

    TEST(AStringOutputStream, ToStringWorksForStreamableTypes)
    {
        EXPECT_EQ("-123", StringOutputStream::ToString(-123));
        EXPECT_EQ("true", StringOutputStream::ToString(true));
        EXPECT_EQ("foobar", StringOutputStream::ToString(std::string("foobar")));
    }

    TEST(AStringOutputStream, CanGetUnderlayingString)
    {
        StringOutputStream stream;
        stream << "foo";
        EXPECT_EQ("foo", stream.data());
        stream << "bar";
        EXPECT_EQ("foobar", stream.data());
    }

    TEST(AStringOutputStream, CanConstructWithString)
    {
        StringOutputStream stream(String("foo"));
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
