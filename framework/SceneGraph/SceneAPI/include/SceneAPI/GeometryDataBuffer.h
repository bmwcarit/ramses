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
#include "Utils/AssertMovable.h"

namespace ramses_internal
{
    struct GeometryDataBuffer
    {
        EDataBufferType bufferType = EDataBufferType::Invalid;
        EDataType       dataType = EDataType::Invalid;
        UInt32          usedSize = 0u;
        std::vector<Byte>    data;
    };

    ASSERT_MOVABLE(GeometryDataBuffer)
}

#endif
