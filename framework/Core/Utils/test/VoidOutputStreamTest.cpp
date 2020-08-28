//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/VoidOutputStream.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Math3d/Matrix44f.h"
#include "Collections/String.h"
#include "Collections/Guid.h"
#include "gmock/gmock.h"

namespace ramses_internal
{

    class VoidOutputStreamTest : public testing::Test
    {
    protected:
        VoidOutputStream stream;
    };

    TEST_F(VoidOutputStreamTest, Constructor)
    {
        const UInt32 expectedSize = 0u;
        EXPECT_EQ(expectedSize,  stream.getSize());
    }

    TEST_F(VoidOutputStreamTest, InsertUInt16)
    {
        const UInt16 value = 5u;
        stream << value;
        EXPECT_EQ( sizeof(UInt16),  stream.getSize());
    }

    TEST_F(VoidOutputStreamTest, InsertBool)
    {
        const bool value = true;
        stream << value;
        EXPECT_EQ( sizeof(bool),  stream.getSize());
    }

    TEST_F(VoidOutputStreamTest, InsertInt32)
    {
        const Int32 value = -7;
        stream << value;
        EXPECT_EQ( sizeof(Int32),  stream.getSize());
    }

    TEST_F(VoidOutputStreamTest, InsertUInt32)
    {
        const UInt32 value = 25u;
        stream << value;
        EXPECT_EQ( sizeof(UInt32),  stream.getSize());
    }

    TEST_F(VoidOutputStreamTest, InsertInt64)
    {
        const Int64 value = -333;
        stream << value;
        EXPECT_EQ( sizeof(Int64),  stream.getSize());
    }

    TEST_F(VoidOutputStreamTest, InsertUInt64)
    {
        const UInt64 value = 5432u;
        stream << value;
        EXPECT_EQ( sizeof(UInt64),  stream.getSize());
    }

    TEST_F(VoidOutputStreamTest, InsertFloat)
    {
        const Float value = 42.23f;
        stream << value;
        EXPECT_EQ( sizeof(Float),  stream.getSize());
    }

    TEST_F(VoidOutputStreamTest, InsertGuid)
    {
        const Guid value;
        stream << value;
        EXPECT_EQ(sizeof(Guid::value_type),  stream.getSize());
    }

    TEST_F(VoidOutputStreamTest, InsertString)
    {
        const String value("Hello World with a lot of characters");
        const UInt32 expectedSize = sizeof(UInt32) + static_cast<UInt32>(value.size()); // length info + string
        stream << value;
        EXPECT_EQ( expectedSize,  stream.getSize());
    }

    TEST_F(VoidOutputStreamTest, InsertResourceContextHash)
    {
        const ResourceContentHash value;
        stream << value;
        EXPECT_EQ(sizeof(ResourceContentHash), stream.getSize());
    }

    TEST_F(VoidOutputStreamTest, InsertRawData)
    {
        const UInt32 sentSize = 37u;
        stream.write(nullptr, sentSize);
        EXPECT_EQ( sentSize,  stream.getSize());
    }


    TEST_F(VoidOutputStreamTest, InsertMultipleData)
    {
        const UInt64 intValue   = 42u;
        const String testString = "abcdefgh";
        const Float  floatValue = 22.22f;
        const UInt32 expectedSize = sizeof(UInt64) + sizeof(UInt32) + 8 + sizeof(Float);
        stream << intValue << testString << floatValue;
        EXPECT_EQ( expectedSize, stream.getSize());
    }
}
