//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEDESCRIPTOR_H
#define RAMSES_RESOURCEDESCRIPTOR_H

#include "RendererLib/EResourceStatus.h"
#include "Resource/ResourceTypes.h"
#include "RendererAPI/Types.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Components/ManagedResource.h"
#include "Collections/HashMap.h"

namespace ramses_internal
{
    struct ResourceDescriptor
    {
        EResourceStatus status = EResourceStatus::Registered;
        EResourceType type = EResourceType_Invalid;
        DeviceResourceHandle deviceHandle;
        ResourceContentHash hash;
        SceneIdVector sceneUsage;
        ManagedResource resource;
        UInt32 compressedSize = 0;
        UInt32 decompressedSize = 0;
        UInt32 vramSize = 0;
    };

    using ResourceDescriptors = HashMap<ResourceContentHash, ResourceDescriptor>;
}

#endif
