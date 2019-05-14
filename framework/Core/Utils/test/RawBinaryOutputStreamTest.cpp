//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gtest/gtest.h"
#include "Collections/Vector.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Utils/RawBinaryOutputStream.h"
#include "Math3d/Matrix44f.h"
#include "SceneAPI/ResourceContentHash.h"

namespace ramses_internal
{

    template<typename T>
    class RawBinaryOutputStreamBaseTypesTest : public ::testing::Test
    {
    public:
        RawBinaryOutputStreamBaseTypesTest()
        {
        }

        static const T m_value;
    };

    // provide sensible default values for all tested types
    template<> const UInt16         RawBinaryOutputStreamBaseTypesTest<UInt16>::m_value       = 1u;
    template<> const Int32          RawBinaryOutputStreamBaseTypesTest<Int32>::m_value        = -2;
    template<> const UInt32         RawBinaryOutputStreamBaseTypesTest<UInt32>::m_value       = 3u;
    template<> const Int64          RawBinaryOutputStreamBaseTypesTest<Int64>::m_value        = -4;
    template<> const UInt64         RawBinaryOutputStreamBaseTypesTest<UInt64>::m_value       = 5u;
    template<> const Float          RawBinaryOutputStreamBaseTypesTest<Float>::m_value        = 6.0f;
    template<> const Bool           RawBinaryOutputStreamBaseTypesTest<Bool>::m_value         = true;
    template<> const Matrix44f      RawBinaryOutputStreamBaseTypesTest<Matrix44f>::m_value    = Matrix44f(
        1.0f, 2.0f, 3.0f, 4.0f,
        5.0f, 6.0f, 7.0f, 8.0f,
        9.0f, 10.0f, 11.0f, 12.0f,
        13.0f, 14.0f, 15.0f, 16.0f);
    template<> const Guid           RawBinaryOutputStreamBaseTypesTest<Guid>::m_value         = Guid("8d2aeb01-6eea-4acb-8d93-df619186cff9");
    template<> const ResourceContentHash RawBinaryOutputStreamBaseTypesTest<ResourceContentHash>::m_value = ResourceContentHash(0x0123456789abcdef, 0xfedcba9876543210);


    // types to test
    typedef ::testing::Types<
        UInt16,
        Int32,
        UInt32,
        Int64,
        UInt64,
        Float,
        Bool,
        Matrix44f,
        ResourceContentHash> RawBinaryOutputStreamBaseTypesTestTypes;


    class RawBinaryOutputStreamComplexTypesTest : public ::testing::Test
    {
    public:
        RawBinaryOutputStreamComplexTypesTest()
        : m_str(
            "People assume that time is a strict progression of cause \
            to effect, but *actually* from a non-linear, non-subjective \
            viewpoint - it's more like a big ball of wibbly wobbly... \
            time-y wimey... stuff.")
        {
            m_buffer = new UChar[BufferSize];
            PlatformMemory::Set(&m_buffer[0], 0xaa, BufferSize);
        }

        virtual ~RawBinaryOutputStreamComplexTypesTest()
        {
            delete [] m_buffer;
        }

        static const UInt BufferSize;

        String m_str;
        UChar* m_buffer;
    };

    const UInt RawBinaryOutputStreamComplexTypesTest::BufferSize   = 2 * 1024; // 2K



    TYPED_TEST_CASE(RawBinaryOutputStreamBaseTypesTest, RawBinaryOutputStreamBaseTypesTestTypes);


    TYPED_TEST(RawBinaryOutputStreamBaseTypesTest, WriteAndCheckSingleElement)
    {
        const UInt32 dataSize = 64 * 1024; // 64K
        std::vector<UInt8> data;
        data.resize(dataSize);

        RawBinaryOutputStream outstr(&data[0], dataSize);
        EXPECT_EQ(outstr.getData(), &data[0]);
        EXPECT_EQ(outstr.getSize(), dataSize);

        outstr << TestFixture::m_value;

        EXPECT_LT(outstr.getBytesWritten(), outstr.getSize());
        EXPECT_EQ(outstr.getBytesWritten(), sizeof(TypeParam));
        EXPECT_EQ(PlatformMemory::Compare(&data[0], &TestFixture::m_value, sizeof(TypeParam)), 0);
    }

    TYPED_TEST(RawBinaryOutputStreamBaseTypesTest, WriteAndCheckMultipleElements)
    {
        const UInt32 iterations = 1000;
        const UInt32 dataSize = 64 * 1024; // 64K
        std::vector<UInt8> data;
        data.resize(dataSize);

        RawBinaryOutputStream outstr(&data[0], dataSize);
        EXPECT_EQ(outstr.getData(), &data[0]);
        EXPECT_EQ(outstr.getSize(), dataSize);

        for (UInt32 ii = 0; ii < iterations; ++ii)
        {
            outstr << TestFixture::m_value;
        }

        EXPECT_LT(outstr.getBytesWritten(), outstr.getSize());
        EXPECT_EQ(outstr.getBytesWritten(), sizeof(TypeParam) * iterations);

        UInt8* dataPtr = &data[0];
        for (UInt32 ii = 0; ii < iterations; ++ii)
        {
            EXPECT_EQ(PlatformMemory::Compare(dataPtr, &TestFixture::m_value, sizeof(TypeParam)), 0);
            dataPtr += sizeof(TypeParam);
        }
    }

    TEST_F(RawBinaryOutputStreamComplexTypesTest, WriteAndCheckString)
    {
        const UInt32 dataSize = 64 * 1024; // 64K
        std::vector<UInt8> data;
        data.resize(dataSize);

        RawBinaryOutputStream outstr(&data[0], dataSize);
        outstr << m_str;

        // bytes written equals UInt32 for string length + amount string characters
        EXPECT_EQ(outstr.getBytesWritten(), m_str.getLength() + sizeof(UInt32));
        // string length written correctly?
        EXPECT_EQ(*reinterpret_cast<UInt32*>(&data[0]), m_str.getLength());
        // string written correctly to stream?
        EXPECT_EQ(PlatformMemory::Compare(&data[0] + sizeof(UInt32), m_str.c_str(), m_str.getLength()), 0);
    }

    TEST_F(RawBinaryOutputStreamComplexTypesTest, WriteAndCheckBuffer)
    {
        const UInt32 dataSize = 64 * 1024; // 64K
        std::vector<UInt8> data;
        data.resize(dataSize);

        RawBinaryOutputStream outstr(&data[0], dataSize);
        outstr.write(m_buffer, BufferSize);

        EXPECT_EQ(outstr.getBytesWritten(), BufferSize);
        EXPECT_EQ(PlatformMemory::Compare(&data[0], m_buffer, BufferSize), 0);
    }
}
