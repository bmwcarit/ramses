/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include "ramses-capu/Config.h"

namespace ramses_capu
{
    TEST(ConfigTest, Types)
    {
        EXPECT_EQ(1u, sizeof(int8_t));
        EXPECT_EQ(1u, sizeof(int8_t));
        EXPECT_EQ(1u, sizeof(uint8_t));
        EXPECT_EQ(2u, sizeof(int16_t));
        EXPECT_EQ(2u, sizeof(uint16_t));
        EXPECT_EQ(4u, sizeof(int32_t));
        EXPECT_EQ(8u, sizeof(int64_t));
        EXPECT_EQ(4u, sizeof(uint32_t));
        EXPECT_EQ(8u, sizeof(uint64_t));
        EXPECT_EQ(4u, sizeof(float));
        EXPECT_EQ(8u, sizeof(double));
        EXPECT_EQ(1u, sizeof(bool));
        EXPECT_EQ(1u, sizeof(char));
        EXPECT_EQ(1u, sizeof(Byte));

#if defined (OS_WINDOWS)
        EXPECT_EQ(8u, sizeof(time_t));
#elif defined (OS_INTEGRITY)
        EXPECT_EQ(8u, sizeof(time_t));
#elif defined (CAPU_ARCH_32)
        EXPECT_EQ(4u, sizeof(time_t));
#elif defined (CAPU_ARCH_64)
        EXPECT_EQ(8u, sizeof(time_t));
#elif defined (ARCH_ARMV7L)
        EXPECT_EQ(4u, sizeof(time_t));
#elif defined (ARCH_ARM64)
        EXPECT_EQ(8u, sizeof(time_t));
#endif

#if defined (CAPU_ARCH_32)
        EXPECT_EQ(4u, sizeof(int_t));
        EXPECT_EQ(4u, sizeof(uint_t));
#elif defined (CAPU_ARCH_64)
        EXPECT_EQ(8u, sizeof(int_t));
        EXPECT_EQ(8u, sizeof(uint_t));
#elif defined (ARCH_ARMV7L)
        EXPECT_EQ(4u, sizeof(int_t));
        EXPECT_EQ(4u, sizeof(uint_t));
#elif defined (ARCH_ARM64)
        EXPECT_EQ(8u, sizeof(int_t));
        EXPECT_EQ(8u, sizeof(uint_t));
#else
#error
#endif
    }
}
