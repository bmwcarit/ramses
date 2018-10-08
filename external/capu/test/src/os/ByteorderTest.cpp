/*
 * Copyright (C) 2017 BMW Car IT GmbH
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

#include "gmock/gmock.h"
#include <ramses-capu/os/Byteorder.h>

namespace ramses_capu
{

    const uint32_t NetworkOrderValueLong  = 0x78563412;
    const uint16_t NetworkOrderValueShort = 0x3412;

// default for little endian
#if defined (__BYTE_ORDER__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
    const uint32_t HostOrderValueLong     = 0x78563412;
    const uint32_t HostOrderValueShort    = 0x3412;
#else
    const uint32_t HostOrderValueLong     = 0x12345678;
    const uint32_t HostOrderValueShort    = 0x1234;
#endif

    TEST(Byteorder,CanConvert32BitNetworkToHost)
    {
        EXPECT_EQ(HostOrderValueLong, ramses_capu::Byteorder::NtoHl(NetworkOrderValueLong));
    }

    TEST(Byteorder,CanConvert16BitNetworkToHost)
    {
        EXPECT_EQ(HostOrderValueShort, ramses_capu::Byteorder::NtoHs(NetworkOrderValueShort));
    }

    TEST(Byteorder,CanConvert32BitHostToNetwork)
    {
        EXPECT_EQ(NetworkOrderValueLong, ramses_capu::Byteorder::HtoNl(HostOrderValueLong));
    }

    TEST(Byteorder,CanConvert16BitHostToNetwork)
    {
        EXPECT_EQ(NetworkOrderValueShort, ramses_capu::Byteorder::HtoNs(HostOrderValueShort));
    }
}
