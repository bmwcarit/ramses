//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"

#include <cstdint>

namespace ramses::internal
{
    enum class EDataType;

    struct ResourceField
    {
        ResourceContentHash hash;
        DataBufferHandle dataBuffer;
        uint32_t            instancingDivisor{0u};
        uint16_t            offsetWithinElementInBytes{0u};
        uint16_t            stride{0u};
    };
}
