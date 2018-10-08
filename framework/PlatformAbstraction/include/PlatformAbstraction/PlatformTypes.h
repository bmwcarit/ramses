//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2010 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMTYPES_H
#define RAMSES_PLATFORMTYPES_H

#include <ramses-capu/Config.h>

#define UNUSED(x) {(void)(x);}

namespace ramses_internal
{
    typedef int8_t   Int8;
    typedef int16_t  Int16;
    typedef int32_t  Int32;
    typedef int64_t  Int64;
    typedef uint8_t  UInt8;
    typedef uint16_t UInt16;
    typedef uint32_t UInt32;
    typedef uint64_t UInt64;
    typedef ramses_capu::uint_t   UInt;
    typedef ramses_capu::int_t    Int;
    typedef unsigned char  UChar;
    typedef float  Float;
    typedef double Double;
    typedef char   Char;
    typedef bool   Bool;
    typedef unsigned char Byte;
}

#endif
