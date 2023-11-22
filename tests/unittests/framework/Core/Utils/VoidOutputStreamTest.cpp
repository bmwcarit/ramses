//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/VoidOutputStream.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/PlatformAbstraction/Collections/Guid.h"
#include "gmock/gmock.h"

#include <string>

namespace ramses::internal
{

    class VoidOutputStreamTest : public testing::Test
    {
    protected:
        VoidOutputStream stream;
    };

    TEST_F(VoidOutputStreamTest, Constructor)
    {
        const uint32_t expectedSize = 0u;
        EXPECT_EQ(expectedSize,  stream.getSize());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, stream.getPos(position));
        EXPECT_EQ(stream.getSize(), position);
    }

    TEST_F(VoidOutputStreamTest, InsertUInt16)
    {
        const uint16_t value = 5u;
        stream << value;
        EXPECT_EQ( sizeof(uint16_t),  stream.getSize());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, stream.getPos(position));
        EXPECT_EQ(stream.getSize(), position);
    }

    TEST_F(VoidOutputStreamTest, InsertBool)
    {
        const bool value = true;
        stream << value;
        EXPECT_EQ( sizeof(bool),  stream.getSize());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, stream.getPos(position));
        EXPECT_EQ(stream.getSize(), position);
    }

    TEST_F(VoidOutputStreamTest, InsertInt32)
    {
        const int32_t value = -7;
        stream << value;
        EXPECT_EQ( sizeof(int32_t),  stream.getSize());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, stream.getPos(position));
        EXPECT_EQ(stream.getSize(), position);
    }

    TEST_F(VoidOutputStreamTest, InsertUInt32)
    {
        const uint32_t value = 25u;
        stream << value;
        EXPECT_EQ( sizeof(uint32_t),  stream.getSize());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, stream.getPos(position));
        EXPECT_EQ(stream.getSize(), position);
    }

    TEST_F(VoidOutputStreamTest, InsertInt64)
    {
        const int64_t value = -333;
        stream << value;
        EXPECT_EQ( sizeof(int64_t),  stream.getSize());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, stream.getPos(position));
        EXPECT_EQ(stream.getSize(), position);
    }

    TEST_F(VoidOutputStreamTest, InsertUInt64)
    {
        const uint64_t value = 5432u;
        stream << value;
        EXPECT_EQ( sizeof(uint64_t),  stream.getSize());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, stream.getPos(position));
        EXPECT_EQ(stream.getSize(), position);
    }

    TEST_F(VoidOutputStreamTest, InsertFloat)
    {
        const float value = 42.23f;
        stream << value;
        EXPECT_EQ( sizeof(float),  stream.getSize());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, stream.getPos(position));
        EXPECT_EQ(stream.getSize(), position);
    }

    TEST_F(VoidOutputStreamTest, InsertGuid)
    {
        const Guid value;
        stream << value;
        EXPECT_EQ(sizeof(Guid::value_type),  stream.getSize());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, stream.getPos(position));
        EXPECT_EQ(stream.getSize(), position);
    }

    TEST_F(VoidOutputStreamTest, InsertString)
    {
        const std::string value("Hello World with a lot of characters");
        const uint32_t expectedSize = sizeof(uint32_t) + static_cast<uint32_t>(value.size()); // length info + string
        stream << value;
        EXPECT_EQ( expectedSize,  stream.getSize());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, stream.getPos(position));
        EXPECT_EQ(stream.getSize(), position);
    }

    TEST_F(VoidOutputStreamTest, InsertResourceContextHash)
    {
        const ResourceContentHash value;
        stream << value;
        EXPECT_EQ(sizeof(ResourceContentHash), stream.getSize());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, stream.getPos(position));
        EXPECT_EQ(stream.getSize(), position);
    }

    TEST_F(VoidOutputStreamTest, InsertRawData)
    {
        const uint32_t sentSize = 37u;
        stream.write(nullptr, sentSize);
        EXPECT_EQ( sentSize,  stream.getSize());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, stream.getPos(position));
        EXPECT_EQ(stream.getSize(), position);
    }


    TEST_F(VoidOutputStreamTest, InsertMultipleData)
    {
        const uint64_t intValue   = 42u;
        const std::string testString = "abcdefgh";
        const float  floatValue = 22.22f;
        const uint32_t expectedSize = sizeof(uint64_t) + sizeof(uint32_t) + 8 + sizeof(float);
        stream << intValue << testString << floatValue;
        EXPECT_EQ( expectedSize, stream.getSize());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, stream.getPos(position));
        EXPECT_EQ(stream.getSize(), position);
    }
}
