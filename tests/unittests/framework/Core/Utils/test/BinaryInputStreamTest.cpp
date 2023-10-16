//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/BinaryInputStream.h"
#include "gtest/gtest.h"
#include "internal/PlatformAbstraction/PlatformMemory.h"
#include "UnsafeTestMemoryHelpers.h"

#include <string>
namespace ramses::internal
{
    TEST(BinaryInputStreamTest, ReadInt32Value)
    {
        std::byte buffer[4];
        int32_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(int32_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5, value);
    }

    TEST(BinaryInputStreamTest, ReadUInt32Value)
    {
        std::byte buffer[4];
        uint32_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(uint32_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5u, value);
    }

    TEST(BinaryInputStreamTest, ReadInt64Value)
    {
        std::byte buffer[8];
        int64_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(int64_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5, value);
    }

    TEST(BinaryInputStreamTest, ReadUInt64Value)
    {
        std::byte buffer[8];

        uint64_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(uint64_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5u, value);
    }

    TEST(BinaryInputStreamTest, ReadUInt16Value)
    {
        std::byte buffer[2];
        uint16_t value = 5;
        PlatformMemory::Copy(buffer, &value, sizeof(uint16_t));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(5, value);
    }

    TEST(BinaryInputStreamTest, ReadStringValue)
    {
        std::byte buffer[16];
        std::string value = "Hello World";
        auto strlen = static_cast<uint32_t>(value.size());

        UnsafeTestMemoryHelpers::WriteToMemoryBlob(strlen, buffer);
        PlatformMemory::Copy(buffer + sizeof(uint32_t), value.c_str(), value.size() + 1);

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_STREQ("Hello World", value.c_str());
    }

    TEST(BinaryInputStreamTest, ReadEmptyStringValue)
    {
        std::byte buffer[16];
        std::string value;
        auto strlen = static_cast<uint32_t>(value.size());

        UnsafeTestMemoryHelpers::WriteToMemoryBlob(strlen, buffer);
        PlatformMemory::Copy(buffer + sizeof(uint32_t), value.c_str(), value.size() + 1);

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_STREQ("", value.c_str());
    }

    TEST(BinaryInputStreamTest, ReadBoolValue)
    {
        std::byte buffer[4];
        bool value = true;
        PlatformMemory::Copy(buffer, &value, sizeof(bool));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(true, value);
    }

    TEST(BinaryInputStreamTest, ReadFloatValue)
    {
        std::byte buffer[4];
        float value = 0.002f;
        PlatformMemory::Copy(buffer, &value, sizeof(float));

        BinaryInputStream inStream(buffer);

        inStream >> value;

        EXPECT_EQ(0.002f, value);
    }

    TEST(BinaryInputStreamTest, ReadMultipleData)
    {
        std::byte buffer[1024];
        int32_t intVal = 5;
        float floatVal = 4.3f;
        std::string stringVal = "Hello World";
        auto strlen = static_cast<uint32_t>(stringVal.size());
        bool boolVal = true;

        size_t offset = 0;

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


        std::byte buffer[sizeof(uint16_t) + sizeof(uint32_t)];
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
        const std::byte buffer[10] = {std::byte{0}};
        BinaryInputStream inStream(buffer);

        char readBuffer[10] = {0};
        EXPECT_EQ(buffer, inStream.readPosition());
        inStream.read(readBuffer, 1);
        EXPECT_EQ(buffer+1, inStream.readPosition());
        inStream.read(readBuffer, 7);
        EXPECT_EQ(buffer+8, inStream.readPosition());
    }

    TEST(BinaryInputStreamTest, canGetPos)
    {
        const std::byte buffer[10] = {std::byte{0}};
        BinaryInputStream inStream(buffer);

        size_t pos = 0;
        EXPECT_EQ(EStatus::Ok, inStream.getPos(pos));
        EXPECT_EQ(0u, pos);

        inStream.skip(1);
        EXPECT_EQ(EStatus::Ok, inStream.getPos(pos));
        EXPECT_EQ(1u, pos);

        inStream.skip(7);
        EXPECT_EQ(EStatus::Ok, inStream.getPos(pos));
        EXPECT_EQ(8u, pos);
    }

    TEST(BinaryInputStreamTest, CanSkipForward)
    {
        const std::byte buffer[10] = {std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6}, std::byte{7}, std::byte{8}, std::byte{9}, std::byte{10}};
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
        const std::byte buffer[10] = {std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6}, std::byte{7}, std::byte{8}, std::byte{9}, std::byte{10}};
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
        const std::byte buffer[10] = {std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6}, std::byte{7}, std::byte{8}, std::byte{9}, std::byte{10}};
        BinaryInputStream inStream(buffer);

        EXPECT_EQ(0u, inStream.getCurrentReadBytes());
    }

    TEST(BinaryInputStreamTest, currentReadBytesIsIncrementedByReading)
    {
        const std::byte buffer[10] = {std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6}, std::byte{7}, std::byte{8}, std::byte{9}, std::byte{10}};
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
        const std::byte buffer[10] = {std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6}, std::byte{7}, std::byte{8}, std::byte{9}, std::byte{10}};
        BinaryInputStream inStream(buffer);

        inStream.skip(1234);
        EXPECT_EQ(1234u, inStream.getCurrentReadBytes());
        inStream.skip(5432);
        EXPECT_EQ(6666u, inStream.getCurrentReadBytes());
    }

    TEST(BinaryInputStreamTest, currentReadBytesIsDecrementedBySkippingBackwards)
    {
        const std::byte buffer[10] = {std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6}, std::byte{7}, std::byte{8}, std::byte{9}, std::byte{10}};
        BinaryInputStream inStream(buffer);

        inStream.skip(5555);
        EXPECT_EQ(5555u, inStream.getCurrentReadBytes());
        inStream.skip(-555);
        EXPECT_EQ(5000u, inStream.getCurrentReadBytes());
    }

    TEST(BinaryInputStreamTest, canSeekFromBeginning)
    {
        const std::byte buffer[10] = {std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6}, std::byte{7}, std::byte{8}, std::byte{9}, std::byte{10}};
        BinaryInputStream inStream(buffer);

        EXPECT_EQ(0u, inStream.getCurrentReadBytes());
        EXPECT_EQ(EStatus::Ok, inStream.seek(1, IInputStream::Seek::FromBeginning));
        EXPECT_EQ(1u, inStream.getCurrentReadBytes());
        EXPECT_EQ(EStatus::Ok, inStream.seek(5, IInputStream::Seek::FromBeginning));
        EXPECT_EQ(5u, inStream.getCurrentReadBytes());
        EXPECT_EQ(EStatus::Ok, inStream.seek(9, IInputStream::Seek::FromBeginning));
        EXPECT_EQ(9u, inStream.getCurrentReadBytes());
        EXPECT_EQ(EStatus::Ok, inStream.seek(0, IInputStream::Seek::FromBeginning));
        EXPECT_EQ(0u, inStream.getCurrentReadBytes());
    }

    TEST(BinaryInputStreamTest, canSeekRelative)
    {
        const std::byte buffer[10] = {std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4}, std::byte{5}, std::byte{6}, std::byte{7}, std::byte{8}, std::byte{9}, std::byte{10}};
        BinaryInputStream inStream(buffer);

        EXPECT_EQ(0u, inStream.getCurrentReadBytes());
        EXPECT_EQ(EStatus::Ok, inStream.seek(1, IInputStream::Seek::Relative));
        EXPECT_EQ(1u, inStream.getCurrentReadBytes());
        EXPECT_EQ(EStatus::Ok, inStream.seek(5, IInputStream::Seek::Relative));
        EXPECT_EQ(6u, inStream.getCurrentReadBytes());
        EXPECT_EQ(EStatus::Ok, inStream.seek(-5, IInputStream::Seek::Relative));
        EXPECT_EQ(1u, inStream.getCurrentReadBytes());
        EXPECT_EQ(EStatus::Ok, inStream.seek(0, IInputStream::Seek::Relative));
        EXPECT_EQ(1u, inStream.getCurrentReadBytes());
    }
}
