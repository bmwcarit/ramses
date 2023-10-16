//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <cstdint>
#include <cstddef>

namespace ramses::internal
{
    static_assert(1u == sizeof(int8_t), "Unexpected sizeof(int8_t)");
    static_assert(1u == sizeof(int8_t), "Unexpected sizeof(int8_t)");
    static_assert(1u == sizeof(uint8_t), "Unexpected sizeof(uint8_t)");
    static_assert(2u == sizeof(int16_t), "Unexpected sizeof(int16_t)");
    static_assert(2u == sizeof(uint16_t), "Unexpected sizeof(uint16_t)");
    static_assert(4u == sizeof(int32_t), "Unexpected sizeof(int32_t)");
    static_assert(8u == sizeof(int64_t), "Unexpected sizeof(int64_t)");
    static_assert(4u == sizeof(uint32_t), "Unexpected sizeof(uint32_t)");
    static_assert(8u == sizeof(uint64_t), "Unexpected sizeof(uint64_t)");
    static_assert(4u == sizeof(float), "Unexpected sizeof(float)");
    static_assert(8u == sizeof(double), "Unexpected sizeof(double)");
    static_assert(1u == sizeof(bool), "Unexpected sizeof(bool)");
    static_assert(1u == sizeof(char), "Unexpected sizeof(char)");
    static_assert(1u == sizeof(std::byte), "Unexpected sizeof(std::byte)");

    static_assert(sizeof(void*) == sizeof(intptr_t), "Unexpected sizeof(intptr_t)");
    static_assert(sizeof(void*) == sizeof(uintptr_t), "Unexpected sizeof(uintptr_t)");
}
