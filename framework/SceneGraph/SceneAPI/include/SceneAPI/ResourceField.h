//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEAPI_RESOURCEFIELD_H
#define RAMSES_SCENEAPI_RESOURCEFIELD_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "SceneAPI/ResourceContentHash.h"
#include "SceneAPI/Handles.h"

namespace ramses_internal
{
    enum class EDataType;

    struct ResourceField
    {
        ResourceContentHash hash;
        DataBufferHandle dataBuffer;
        UInt32 instancingDivisor;
        UInt16 offsetWithinElementInBytes;
        UInt16 stride;
    };
}

#endif
