//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/RawBinaryOutputStream.h"
#include "gtest/gtest.h"
#include <numeric>

namespace ramses_internal
{
    TEST(ARawBinaryOutputStream, hasExpectedDefaultValues)
    {
        std::vector<Byte> buffer(10);
        RawBinaryOutputStream os(buffer.data(), buffer.size());

        EXPECT_EQ(buffer.data(), os.getData());
        EXPECT_EQ(buffer.size(), os.getSize());
        EXPECT_EQ(0u, os.getBytesWritten());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, os.getPos(position));
        EXPECT_EQ(0u, position);
    }

    TEST(ARawBinaryOutputStream, canDoZeroWrite)
    {
        std::vector<Byte> buffer(10);
        RawBinaryOutputStream os(buffer.data(), buffer.size());

        const uint32_t d = 123;
        os.write(&d, 0);
        EXPECT_EQ(0u, os.getBytesWritten());

        os.write(nullptr, 0);
        EXPECT_EQ(0u, os.getBytesWritten());

        size_t position = std::numeric_limits<size_t>::max();
        EXPECT_EQ(EStatus::Ok, os.getPos(position));
        EXPECT_EQ(0u, position);
    }

    TEST(ARawBinaryOutputStream, canWriteData)
    {
        std::vector<Byte>     buffer(450);
        std::vector<Byte>     refBuffer(450);
        RawBinaryOutputStream os(buffer.data(), buffer.size());

        size_t position = std::numeric_limits<size_t>::max();
        const uint8_t     d8  = 123;
        const uint16_t    d16 = 65531;
        const uint64_t    d64 = 0xf897abd898798;
        std::vector<Byte> dVec(432);
        std::iota(dVec.begin(), dVec.end(), static_cast<Byte>(127));

        EXPECT_EQ(refBuffer, buffer);

        os.write(&d8, sizeof(d8));
        std::memcpy(refBuffer.data(), &d8, sizeof(d8));
        EXPECT_EQ(1u, os.getBytesWritten());
        EXPECT_EQ(refBuffer, buffer);
        EXPECT_EQ(EStatus::Ok, os.getPos(position));
        EXPECT_EQ(position, os.getBytesWritten());

        os.write(&d64, sizeof(d64));
        std::memcpy(refBuffer.data()+1, &d64, sizeof(d64));
        EXPECT_EQ(9u, os.getBytesWritten());
        EXPECT_EQ(refBuffer, buffer);
        EXPECT_EQ(EStatus::Ok, os.getPos(position));
        EXPECT_EQ(position, os.getBytesWritten());

        os.write(dVec.data(), dVec.size());
        std::memcpy(refBuffer.data()+9, dVec.data(), dVec.size());
        EXPECT_EQ(441u, os.getBytesWritten());
        EXPECT_EQ(refBuffer, buffer);
        EXPECT_EQ(EStatus::Ok, os.getPos(position));
        EXPECT_EQ(position, os.getBytesWritten());

        os.write(&d16, sizeof(d16));
        std::memcpy(refBuffer.data()+441, &d16, sizeof(d16));
        EXPECT_EQ(443u, os.getBytesWritten());
        EXPECT_EQ(refBuffer, buffer);
        EXPECT_EQ(EStatus::Ok, os.getPos(position));
        EXPECT_EQ(position, os.getBytesWritten());

        EXPECT_EQ(buffer.data(), os.getData());
        EXPECT_EQ(buffer.size(), os.getSize());
    }

    TEST(ARawBinaryOutputStream, canWriteExactlyFittingData)
    {
        std::vector<Byte>     buffer(4);
        std::vector<Byte>     refBuffer(4);
        RawBinaryOutputStream os(buffer.data(), buffer.size());

        const uint32_t d = 123456789;
        os.write(&d, sizeof(d));
        EXPECT_EQ(4u, os.getBytesWritten());

        std::memcpy(refBuffer.data(), &d, sizeof(d));
        EXPECT_EQ(refBuffer, buffer);
    }
}
