//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Enums/EResourceStatus.h"
#include "internal/SceneGraph/Resource/ResourceTypes.h"
#include "internal/RendererLib/Types.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/Components/ManagedResource.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"

namespace ramses::internal
{
    struct ResourceDescriptor
    {
        EResourceStatus status = EResourceStatus::Registered;
        EResourceType type = EResourceType::Invalid;
        DeviceResourceHandle deviceHandle;
        ResourceContentHash hash;
        SceneIdVector sceneUsage;
        ManagedResource resource;
        uint32_t compressedSize = 0;
        uint32_t decompressedSize = 0;
        uint32_t vramSize = 0;
    };

    using ResourceDescriptors = HashMap<ResourceContentHash, ResourceDescriptor>;
}
