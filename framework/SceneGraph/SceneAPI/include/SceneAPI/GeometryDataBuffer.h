//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERNAL_GEOMETRYDATABUFFER_H
#define RAMSES_INTERNAL_GEOMETRYDATABUFFER_H

#include "SceneAPI/EDataBufferType.h"
#include "SceneAPI/EDataType.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    struct GeometryDataBuffer
    {
        GeometryDataBuffer() = default;
        GeometryDataBuffer(const GeometryDataBuffer&) = default;
        GeometryDataBuffer& operator=(const  GeometryDataBuffer&) = default;
        GeometryDataBuffer(GeometryDataBuffer&&) = default;
        GeometryDataBuffer& operator=(GeometryDataBuffer&&) = default;

        EDataBufferType bufferType = EDataBufferType::Invalid;
        EDataType       dataType = EDataType_Invalid;
        UInt32          usedSize = 0u;
        std::vector<Byte>    data;
    };

    static_assert(std::is_nothrow_move_constructible<GeometryDataBuffer>::value &&
        std::is_nothrow_move_assignable<GeometryDataBuffer>::value, "GeometryDataBuffer must be movable");
}

#endif
