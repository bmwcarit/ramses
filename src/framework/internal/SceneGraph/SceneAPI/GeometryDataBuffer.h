//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/EDataBufferType.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/Core/Utils/AssertMovable.h"

namespace ramses::internal
{
    struct GeometryDataBuffer
    {
        EDataBufferType bufferType = EDataBufferType::Invalid;
        EDataType       dataType = EDataType::Invalid;
        uint32_t        usedSize = 0u;
        std::vector<std::byte>    data;
    };

    ASSERT_MOVABLE(GeometryDataBuffer)
}
