//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/Core/Utils/BinaryOutputStream.h"
#include "internal/Core/Utils/BinaryInputStream.h"
#include "internal/PlatformAbstraction/Collections/Guid.h"
#include "UnsafeTestMemoryHelpers.h"

namespace ramses::internal
{
    TEST(BinaryOutputStreamTest, Constructor)
    {
        BinaryOutputStream outStream;

        EXPECT_EQ(0u,  outStream.getSize());
        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, outStream.getPos(position));
        EXPECT_EQ(0u, position);
        EXPECT_EQ(16u, outStream.getCapacity());
    }

    TEST(BinaryOutputStreamTest, ConstructorWithCapacity)
    {
        BinaryOutputStream outStream(32);

        EXPECT_EQ(0u,  outStream.getSize());
        EXPECT_EQ(32u, outStream.getCapacity());
    }

    TEST(BinaryOutputStreamTest, InsertUInt16)
    {
        BinaryOutputStream outStream;

        outStream << static_cast<uint16_t>(5) << static_cast<uint16_t>(6) << static_cast<uint16_t>(7);

        const std::byte* data = outStream.getData();
        EXPECT_EQ(5, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint16_t>(data));
        data += sizeof(uint16_t);
        EXPECT_EQ(6, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint16_t>(data));
        data += sizeof(uint16_t);
        EXPECT_EQ(7, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint16_t>(data));

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, outStream.getPos(position));
        EXPECT_EQ(3 * sizeof(uint16_t), position);
    }

    TEST(BinaryOutputStreamTest, InsertBool)
    {
        BinaryOutputStream outStream;

        outStream << true << false << true;

        const std::byte* data = outStream.getData();
        EXPECT_TRUE(UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<bool>(data));
        data += sizeof(bool);
        EXPECT_FALSE(UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<bool>(data));
        data += sizeof(bool);
        EXPECT_TRUE(UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<bool>(data));

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, outStream.getPos(position));
        EXPECT_EQ(3 * sizeof(bool), position);
    }

    TEST(BinaryOutputStreamTest, InsertInt)
    {
        BinaryOutputStream outStream;

        outStream << 5 << 6 << 7;

        const std::byte* data = outStream.getData();
        EXPECT_EQ(5, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<int32_t>(data));
        data += sizeof(int32_t);
        EXPECT_EQ(6, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<int32_t>(data));
        data += sizeof(int32_t);
        EXPECT_EQ(7, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<int32_t>(data));

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, outStream.getPos(position));
        EXPECT_EQ(3 * sizeof(int32_t), position);
    }

    TEST(BinaryOutputStreamTest, InsertUInt)
    {
        BinaryOutputStream outStream;
        outStream << 0u << 5u << 6u << 7u << std::numeric_limits<uint32_t>::max();

        const std::byte* data = outStream.getData();
        EXPECT_EQ(0u, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(data));
        data += sizeof(uint32_t);
        EXPECT_EQ(5u, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(data));
        data += sizeof(uint32_t);
        EXPECT_EQ(6u, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(data));
        data += sizeof(uint32_t);
        EXPECT_EQ(7u, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(data));
        data += sizeof(uint32_t);
        EXPECT_EQ(std::numeric_limits<uint32_t>::max(), UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(data));

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, outStream.getPos(position));
        EXPECT_EQ(5 * sizeof(uint32_t), position);
    }

    TEST(BinaryOutputStreamTest, InsertInt64)
    {
        BinaryOutputStream outStream;

        outStream << static_cast<int64_t>(5) << static_cast<int64_t>(6) << static_cast<int64_t>(7);

        const std::byte* data = outStream.getData();
        EXPECT_EQ(5, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<int64_t>(data));
        data += sizeof(int64_t);
        EXPECT_EQ(6, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<int64_t>(data));
        data += sizeof(int64_t);
        EXPECT_EQ(7, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<int64_t>(data));

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, outStream.getPos(position));
        EXPECT_EQ(3 * sizeof(uint64_t), position);
    }

    TEST(BinaryOutputStreamTest, InsertUInt64)
    {
        BinaryOutputStream outStream;
        outStream << static_cast<uint64_t>(0u) << static_cast<uint64_t>(5u) << static_cast<uint64_t>(6u) << static_cast<uint64_t>(7u) << std::numeric_limits<uint64_t>::max();

        const std::byte* data = outStream.getData();
        EXPECT_EQ(0u, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint64_t>(data));
        data += sizeof(uint64_t);
        EXPECT_EQ(5u, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint64_t>(data));
        data += sizeof(uint64_t);
        EXPECT_EQ(6u, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint64_t>(data));
        data += sizeof(uint64_t);
        EXPECT_EQ(7u, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint64_t>(data));
        data += sizeof(uint64_t);
        EXPECT_EQ(std::numeric_limits<uint64_t>::max(), UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint64_t>(data));

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, outStream.getPos(position));
        EXPECT_EQ(5 * sizeof(uint64_t), position);
    }

    TEST(BinaryOutputStreamTest, InsertFloat)
    {
        BinaryOutputStream outStream;
        outStream << 5.0f << 6.0f << 7.0f;

        const std::byte* data = outStream.getData();
        EXPECT_EQ(5.0f, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<float>(data));
        data += sizeof(int32_t);
        EXPECT_EQ(6.0f, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<float>(data));
        data += sizeof(int32_t);
        EXPECT_EQ(7.0f, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<float>(data));

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, outStream.getPos(position));
        EXPECT_EQ(3 * sizeof(float), position);
    }

    TEST(BinaryOutputStreamTest, InsertGuid)
    {
        BinaryOutputStream outStream;
        Guid guid1;
        Guid guid2;
        outStream << guid1 << guid2;

        const std::byte* data = outStream.getData();

        BinaryInputStream in(data);
        Guid fromStreamGuid1;
        Guid fromStreamGuid2;
        in >> fromStreamGuid1 >> fromStreamGuid2;

        EXPECT_EQ(guid1, fromStreamGuid1);
        EXPECT_EQ(guid2, fromStreamGuid2);

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, outStream.getPos(position));
        EXPECT_EQ(2 * sizeof(Guid::value_type), position);
    }

    TEST(BinaryOutputStreamTest, InsertMultipleData)
    {
        BinaryOutputStream outStream;
        const std::string testString = "abcdefgh";
        outStream << 5 << testString << 7.0f;

        const std::byte* data = outStream.getData();
        EXPECT_EQ(5, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<int32_t>(data));
        data += sizeof(int32_t);
        const auto len = UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(data);
        EXPECT_EQ(len, static_cast<uint32_t>(testString.size()));
        data += sizeof(int32_t);
        EXPECT_EQ(0, std::memcmp(testString.data(), data, testString.size()));
        data += testString.size() * sizeof(char);
        EXPECT_EQ(7.0f, UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<float>(data));
    }

    TEST(BinaryOutputStreamTest, InsertString)
    {
        BinaryOutputStream outStream;

        outStream << std::string("Hello World with a lot of characters");

        const auto strlen = UnsafeTestMemoryHelpers::GetTypedValueFromMemoryBlob<uint32_t>(outStream.getData());

        std::vector<char> buffer(strlen + 1);
        std::memcpy(buffer.data(), outStream.getData() + sizeof(uint32_t), strlen);
        buffer[strlen] = 0;

        EXPECT_STREQ("Hello World with a lot of characters", buffer.data());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, outStream.getPos(position));
        EXPECT_EQ(strlen + sizeof(uint32_t), position);
    }

    TEST(BinaryOutputStreamTest, InserStronglyTypedEnum)
    {
        enum class TestEnum16 : uint16_t
        {
            TestEnumValue = 123
        };

        enum class TestEnum32 : uint32_t
        {
            TestEnumValue = 567
        };

        BinaryOutputStream outStream;

        outStream << TestEnum16::TestEnumValue << TestEnum32::TestEnumValue;

        ASSERT_EQ(sizeof(uint16_t) + sizeof(uint32_t), outStream.getSize());
        const std::byte* data = outStream.getData();
        TestEnum16 value16;
        std::memcpy(&value16, data, sizeof(value16));
        EXPECT_EQ(TestEnum16::TestEnumValue, value16);

        data += sizeof(uint16_t);
        TestEnum32 value32;
        std::memcpy(&value32, data, sizeof(value32));
        EXPECT_EQ(TestEnum32::TestEnumValue, value32);
    }

    TEST(BinaryOutputStreamTest, Release)
    {
        BinaryOutputStream stream;
        stream << static_cast<uint32_t>(1);
        EXPECT_EQ(sizeof(uint32_t), stream.getSize());

        std::vector<std::byte> vec = stream.release();
        EXPECT_EQ(sizeof(uint32_t), vec.size());
        EXPECT_EQ(0u, stream.getSize());
        EXPECT_TRUE(nullptr == stream.getData());
    }
}
