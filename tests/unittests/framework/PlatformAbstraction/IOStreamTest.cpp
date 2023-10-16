//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "IOStreamTester.h"
#include "gtest/gtest.h"
#include <limits>

namespace ramses::internal
{
    class AIInutOutputStream : public ::testing::Test, public IOStreamTesterBase
    {
    };

    TEST_F(AIInutOutputStream, verifyUint32)
    {
        expectSame<uint32_t>(1234567891);
        expectSame<uint32_t>(1);
        expectSame<uint32_t>(0, 1);
        expectSame<uint32_t>(std::numeric_limits<uint32_t>::max(), 0);
    }

    TEST_F(AIInutOutputStream, verifyInt32)
    {
        expectSame<int32_t>(1234567891);
        expectSame<int32_t>(-1234567891);
        expectSame<int32_t>(0, 1);
        expectSame<int32_t>(std::numeric_limits<int32_t>::max(), 0);
        expectSame<int32_t>(std::numeric_limits<int32_t>::min(), 0);
    }

    TEST_F(AIInutOutputStream, verifyUint64)
    {
        expectSame<uint64_t>(1234567891);
        expectSame<uint64_t>(1);
        expectSame<uint64_t>(0, 1);
        expectSame<uint64_t>(std::numeric_limits<uint64_t>::max(), 0);
    }

    TEST_F(AIInutOutputStream, verifyInt64)
    {
        expectSame<int64_t>(1234567891);
        expectSame<int64_t>(-1234567891);
        expectSame<int64_t>(0, 1);
        expectSame<int64_t>(std::numeric_limits<int64_t>::max(), 0);
        expectSame<int64_t>(std::numeric_limits<int64_t>::min(), 0);
    }

    TEST_F(AIInutOutputStream, verifyUint16)
    {
        expectSame<uint16_t>(12345);
        expectSame<uint16_t>(1);
        expectSame<uint16_t>(0, 1);
        expectSame<uint16_t>(std::numeric_limits<uint16_t>::max(), 0);
    }

    TEST_F(AIInutOutputStream, verifyInt16)
    {
        expectSame<int16_t>(12345);
        expectSame<int16_t>(-12345);
        expectSame<int16_t>(0, 1);
        expectSame<int16_t>(std::numeric_limits<int16_t>::max(), 0);
        expectSame<int16_t>(std::numeric_limits<int16_t>::min(), 0);
    }

    TEST_F(AIInutOutputStream, verifyUint8)
    {
        expectSame<uint8_t>(123);
        expectSame<uint8_t>(1);
        expectSame<uint8_t>(0, 1);
        expectSame<uint8_t>(std::numeric_limits<uint8_t>::max(), 0);
    }

    TEST_F(AIInutOutputStream, verifyInt8)
    {
        expectSame<int8_t>(123);
        expectSame<int8_t>(-123);
        expectSame<int8_t>(0, 1);
        expectSame<int8_t>(std::numeric_limits<int8_t>::max(), 0);
        expectSame<int8_t>(std::numeric_limits<int8_t>::min(), 0);
    }

    TEST_F(AIInutOutputStream, verifyBool)
    {
        expectSame<bool>(true);
        expectSame<bool>(false, true);
    }

    TEST_F(AIInutOutputStream, verifyFloat)
    {
        expectSame<float>(1.0);
        expectSame<float>(0.0, 1.0);
        expectSame<float>(-1.0);
        expectSame<float>(std::numeric_limits<float>::max());
        expectSame<float>(std::numeric_limits<float>::min());
    }

    TEST_F(AIInutOutputStream, verifyEnum)
    {
        enum class TestEnum
        {
            V1,
            V2,
            V3
        };
        expectSame<TestEnum>(TestEnum::V1, TestEnum::V2);
        expectSame<TestEnum>(TestEnum::V2);
        expectSame<TestEnum>(TestEnum::V3);
    }

    TEST_F(AIInutOutputStream, verifyEnum8)
    {
        enum class TestEnum : uint8_t
        {
            V1,
            V2,
            V3
        };
        expectSame<TestEnum>(TestEnum::V1, TestEnum::V2);
        expectSame<TestEnum>(TestEnum::V2);
        expectSame<TestEnum>(TestEnum::V3);
    }
}
