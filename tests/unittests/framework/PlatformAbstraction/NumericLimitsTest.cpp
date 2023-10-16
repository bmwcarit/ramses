//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"

#include <cstdint>
#include <limits>

namespace ramses::internal
{
    TEST(NumericLimits, ensureAvailableForAlltypes)
    {
        (void)std::numeric_limits<uint64_t>::max();
        (void)std::numeric_limits<int64_t>::max();

        (void)std::numeric_limits<uint32_t>::max();
        (void)std::numeric_limits<int32_t>::max();

        (void)std::numeric_limits<uint16_t>::max();
        (void)std::numeric_limits<int16_t>::max();

        (void)std::numeric_limits<uint8_t>::max();
        (void)std::numeric_limits<int8_t>::max();

        (void)std::numeric_limits<float>::max();
        (void)std::numeric_limits<double>::max();

        (void)std::numeric_limits<size_t>::max();
        (void)std::numeric_limits<intptr_t>::max();
    }

    TEST(NumericLimits, testLimits)
    {
        EXPECT_EQ(std::numeric_limits<int64_t>::max(), static_cast<int64_t>(0x7fffffffffffffffLL));
        EXPECT_EQ(std::numeric_limits<int64_t>::min(), static_cast<int64_t>(0x8000000000000000LL));
        EXPECT_EQ(std::numeric_limits<uint64_t>::max(), static_cast<uint64_t>(0xffffffffffffffffULL));
        EXPECT_EQ(std::numeric_limits<uint64_t>::min(), static_cast<uint64_t>(0x0));
        EXPECT_EQ(std::numeric_limits<int32_t>::max(), static_cast<int32_t>(0x7fffffff));
        EXPECT_EQ(std::numeric_limits<int32_t>::min(), static_cast<int32_t>(0x80000000));
        EXPECT_EQ(std::numeric_limits<uint32_t>::max(), static_cast<uint32_t>(0xffffffff));
        EXPECT_EQ(std::numeric_limits<uint32_t>::min(), static_cast<uint32_t>(0x0));
        EXPECT_EQ(std::numeric_limits<int16_t>::max(), static_cast<int32_t>(32767));
        EXPECT_EQ(std::numeric_limits<int16_t>::min(), static_cast<int32_t>(-32768));
        EXPECT_EQ(std::numeric_limits<uint16_t>::max(), static_cast<int32_t>(65535));
        EXPECT_EQ(std::numeric_limits<uint16_t>::min(), static_cast<int32_t>(0));
        EXPECT_EQ(std::numeric_limits<int8_t>::max(), static_cast<int32_t>(127));
        EXPECT_EQ(std::numeric_limits<int8_t>::min(), static_cast<int32_t>(-128));
        EXPECT_EQ(std::numeric_limits<uint8_t>::max(), static_cast<int32_t>(255));
        EXPECT_EQ(std::numeric_limits<uint8_t>::min(), static_cast<int32_t>(0));
    }
}
