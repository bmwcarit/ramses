//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2010 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMTYPES_H
#define RAMSES_PLATFORMTYPES_H

#include <cstdint>

#define UNUSED(x) ((void)(x))

namespace ramses_internal
{
    using Int8 = int8_t;
    using Int16 = int16_t;
    using Int32 = int32_t;
    using Int64 = int64_t;
    using UInt8 = uint8_t;
    using UInt16 = uint16_t;
    using UInt32 = uint32_t;
    using UInt64 = uint64_t;
    using UInt = std::uintptr_t;
    using Int = std::intptr_t;
    using Float = float;
    using Char = char;
    using Byte = unsigned char;
}

#endif
