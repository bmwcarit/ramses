//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Utils/BinaryInputStream.h"
#include "Math3d/Matrix44f.h"

namespace ramses_internal
{
    TEST(BinaryInputStreamTest, ReadInt32Value)
    {
        char buffer[4];
        int32_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(int32_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5, value);
    }

    TEST(BinaryInputStreamTest, ReadUInt32Value)
    {
        char buffer[4];
        uint32_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(uint32_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5u, value);
    }

    TEST(BinaryInputStreamTest, ReadInt64Value)
    {
        char buffer[8];
        int64_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(int64_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5, value);
    }

    TEST(BinaryInputStreamTest, ReadUInt64Value)
    {
        char buffer[8];

        uint64_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(uint64_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5u, value);
    }

    TEST(BinaryInputStreamTest, ReadUInt16Value)
    {
        char buffer[2];
        uint16_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(uint16_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5, value);
    }

    TEST(BinaryInputStreamTest, ReadStringValue)
    {
        char buffer[16];
        String value = "Hello World";
        uint32_t strlen = static_cast<uint32_t>(value.getLength());

        PlatformMemory::Copy(buffer, reinterpret_cast<char*>(&strlen), sizeof(uint32_t));
        PlatformMemory::Copy(buffer + sizeof(uint32_t), value.c_str(), value.getLength() + 1);

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_STREQ("Hello World", value.c_str());
    }

    TEST(BinaryInputStreamTest, ReadEmptyStringValue)
    {
        char buffer[16];
        String value = "";
        uint32_t strlen = static_cast<uint32_t>(value.getLength());

        PlatformMemory::Copy(buffer, reinterpret_cast<char*>(&strlen), sizeof(uint32_t));
        PlatformMemory::Copy(buffer + sizeof(uint32_t), value.c_str(), value.getLength() + 1);

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_STREQ("", value.c_str());
    }

    TEST(BinaryInputStreamTest, ReadBoolValue)
    {
        char buffer[4];
        bool value = true;
        PlatformMemory::Copy(buffer, &value, sizeof(bool));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(true, value);
    }

    TEST(BinaryInputStreamTest, ReadFloatValue)
    {
        char buffer[4];
        float value = 0.002f;
        PlatformMemory::Copy(buffer, &value, sizeof(float));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(0.002f, value);
    }

    TEST(BinaryInputStreamTest, ReadMultipleData)
    {
        char buffer[1024];
        int32_t intVal = 5;
        float floatVal = 4.3f;
        String stringVal = "Hello World";
        uint32_t strlen = static_cast<uint32_t>(stringVal.getLength());
        bool boolVal = true;

        UInt offset = 0;

        PlatformMemory::Copy(buffer, &intVal, sizeof(int32_t));
        offset += sizeof(int32_t);
        PlatformMemory::Copy(buffer + offset, &floatVal, sizeof(float));
        offset += sizeof(float);
        PlatformMemory::Copy(buffer + offset, &strlen, sizeof(uint32_t));
        offset += sizeof(uint32_t);
        PlatformMemory::Copy(buffer + offset , stringVal.c_str(), stringVal.getLength() + 1);
        offset += stringVal.getLength();
        PlatformMemory::Copy(buffer + offset, &boolVal, sizeof(bool));

        BinaryInputStream inStream(buffer);

        inStream >> intVal >> floatVal >> stringVal >> boolVal;

        EXPECT_EQ(5, intVal);
        EXPECT_EQ(4.3f, floatVal);
        EXPECT_STREQ("Hello World", stringVal.c_str());
        EXPECT_EQ(true, boolVal);
    }

    TEST(BinaryInputStreamTest, Matrix44f)
    {
        Float data[16] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f, 10.0f, 11.0f, 12.0f, 13.0f,
        14.0f, 15.0f, 16.0f};

        BinaryInputStream s(reinterpret_cast<Char*>(data));

        Matrix44f m;

        s >> m;

        for (UInt32 i = 0; i < 16; i++)
        {
            EXPECT_EQ(Float(i + 1), m.data[i]);
        }
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


        char buffer[sizeof(uint16_t) + sizeof(uint32_t)];
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
        const char buffer[10] = {0};
        BinaryInputStream inStream(buffer);

        char readBuffer[10] = {0};
        EXPECT_EQ(buffer, inStream.readPosition());
        inStream.read(readBuffer, 1);
        EXPECT_EQ(buffer+1, inStream.readPosition());
        inStream.read(readBuffer, 7);
        EXPECT_EQ(buffer+8, inStream.readPosition());
    }
}
