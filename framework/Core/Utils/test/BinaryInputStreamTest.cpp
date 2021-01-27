//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/BinaryInputStream.h"
#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Math3d/Matrix44f.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Collections/String.h"

namespace ramses_internal
{
    TEST(BinaryInputStreamTest, ReadInt32Value)
    {
        Byte buffer[4];
        int32_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(int32_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5, value);
    }

    TEST(BinaryInputStreamTest, ReadUInt32Value)
    {
        Byte buffer[4];
        uint32_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(uint32_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5u, value);
    }

    TEST(BinaryInputStreamTest, ReadInt64Value)
    {
        Byte buffer[8];
        int64_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(int64_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5, value);
    }

    TEST(BinaryInputStreamTest, ReadUInt64Value)
    {
        Byte buffer[8];

        uint64_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(uint64_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5u, value);
    }

    TEST(BinaryInputStreamTest, ReadUInt16Value)
    {
        Byte buffer[2];
        uint16_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(uint16_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5, value);
    }

    TEST(BinaryInputStreamTest, ReadStringValue)
    {
        Byte buffer[16];
        String value = "Hello World";
        uint32_t strlen = static_cast<uint32_t>(value.size());

        PlatformMemory::Copy(buffer, reinterpret_cast<char*>(&strlen), sizeof(uint32_t));
        PlatformMemory::Copy(buffer + sizeof(uint32_t), value.c_str(), value.size() + 1);

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_STREQ("Hello World", value.c_str());
    }

    TEST(BinaryInputStreamTest, ReadEmptyStringValue)
    {
        Byte buffer[16];
        String value = "";
        uint32_t strlen = static_cast<uint32_t>(value.size());

        PlatformMemory::Copy(buffer, reinterpret_cast<char*>(&strlen), sizeof(uint32_t));
        PlatformMemory::Copy(buffer + sizeof(uint32_t), value.c_str(), value.size() + 1);

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_STREQ("", value.c_str());
    }

    TEST(BinaryInputStreamTest, ReadBoolValue)
    {
        Byte buffer[4];
        bool value = true;
        PlatformMemory::Copy(buffer, &value, sizeof(bool));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(true, value);
    }

    TEST(BinaryInputStreamTest, ReadFloatValue)
    {
        Byte buffer[4];
        float value = 0.002f;
        PlatformMemory::Copy(buffer, &value, sizeof(float));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(0.002f, value);
    }

    TEST(BinaryInputStreamTest, ReadMultipleData)
    {
        Byte buffer[1024];
        int32_t intVal = 5;
        float floatVal = 4.3f;
        String stringVal = "Hello World";
        uint32_t strlen = static_cast<uint32_t>(stringVal.size());
        bool boolVal = true;

        UInt offset = 0;

        PlatformMemory::Copy(buffer, &intVal, sizeof(int32_t));
        offset += sizeof(int32_t);
        PlatformMemory::Copy(buffer + offset, &floatVal, sizeof(float));
        offset += sizeof(float);
        PlatformMemory::Copy(buffer + offset, &strlen, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        PlatformMemory::Copy(buffer + offset , stringVal.c_str(), stringVal.size() + 1);
        offset += stringVal.size();
        PlatformMemory::Copy(buffer + offset, &boolVal, sizeof(bool));

        BinaryInputStream inStream(buffer);

        inStream >> intVal >> floatVal >> stringVal >> boolVal;

        EXPECT_EQ(5, intVal);
        EXPECT_EQ(4.3f, floatVal);
        EXPECT_STREQ("Hello World", stringVal.c_str());
        EXPECT_EQ(true, boolVal);
    }

    TEST(BinaryInputStreamTest, ReadStronglyTypedEnum)
    {
        enum class TestEnum16 : uint16_t
        {
            TestEnumValue = 123,
            OtherTestEnumValue
        };

        enum class TestEnum32 : uint32_t
        {
            TestEnumValue = 567,
            OtherTestEnumValue
        };


        Byte buffer[sizeof(uint16_t) + sizeof(uint32_t)];
        TestEnum16 value16 = TestEnum16::TestEnumValue;
        TestEnum32 value32 = TestEnum32::TestEnumValue;

        PlatformMemory::Copy(buffer, &value16, sizeof(value16));
        PlatformMemory::Copy(buffer + sizeof(value16), &value32, sizeof(value32));

        BinaryInputStream inStream(buffer);


        TestEnum16 readValue16 = TestEnum16::OtherTestEnumValue;
        TestEnum32 readValue32 = TestEnum32::OtherTestEnumValue;

        inStream >> readValue16 >> readValue32;

        EXPECT_EQ(TestEnum16::TestEnumValue, readValue16);
        EXPECT_EQ(TestEnum32::TestEnumValue, readValue32);
    }

    TEST(BinaryInputStreamTest, GetReadPosition)
    {
        const Byte buffer[10] = {0};
        BinaryInputStream inStream(buffer);

        char readBuffer[10] = {0};
        EXPECT_EQ(buffer, inStream.readPosition());
        inStream.read(readBuffer, 1);
        EXPECT_EQ(buffer+1, inStream.readPosition());
        inStream.read(readBuffer, 7);
        EXPECT_EQ(buffer+8, inStream.readPosition());
    }

    TEST(BinaryInputStreamTest, CanSkipForward)
    {
        const Byte buffer[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        BinaryInputStream inStream(buffer);
        uint8_t in = 0;
        inStream.skip(3);
        inStream >> in;
        EXPECT_EQ(4u, in);
        inStream.skip(1);
        inStream >> in;
        EXPECT_EQ(6u, in);
        inStream.skip(3);
        inStream >> in;
        EXPECT_EQ(10u, in);
    }

    TEST(BinaryInputStreamTest, CanSkipBackward)
    {
        const Byte buffer[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        BinaryInputStream inStream(buffer);
        uint8_t in = 0;
        inStream >> in;
        EXPECT_EQ(1u, in);
        inStream.skip(-1);
        inStream >> in;
        EXPECT_EQ(1u, in);
        inStream.skip(8);
        inStream.skip(-6);
        inStream >> in;
        EXPECT_EQ(4u, in);
    }

    TEST(BinaryInputStreamTest, currentReadBytesStartsWithZero)
    {
        const Byte buffer[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        BinaryInputStream inStream(buffer);

        EXPECT_EQ(0u, inStream.getCurrentReadBytes());
    }

    TEST(BinaryInputStreamTest, currentReadBytesIsIncrementedByReading)
    {
        const Byte buffer[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        BinaryInputStream inStream(buffer);

        uint8_t in = 0;
        inStream >> in;
        EXPECT_EQ(1u, inStream.getCurrentReadBytes());
        uint32_t in2 = 0;
        inStream >> in2;
        EXPECT_EQ(5u, inStream.getCurrentReadBytes());
    }

    TEST(BinaryInputStreamTest, currentReadBytesIsIncrementedBySkip)
    {
        const Byte buffer[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        BinaryInputStream inStream(buffer);

        inStream.skip(1234);
        EXPECT_EQ(1234u, inStream.getCurrentReadBytes());
        inStream.skip(5432);
        EXPECT_EQ(6666u, inStream.getCurrentReadBytes());
    }

    TEST(BinaryInputStreamTest, currentReadBytesIsDecrementedBySkippingBackwards)
    {
        const Byte buffer[10] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
        BinaryInputStream inStream(buffer);

        inStream.skip(5555);
        EXPECT_EQ(5555u, inStream.getCurrentReadBytes());
        inStream.skip(-555);
        EXPECT_EQ(5000u, inStream.getCurrentReadBytes());
    }
}
