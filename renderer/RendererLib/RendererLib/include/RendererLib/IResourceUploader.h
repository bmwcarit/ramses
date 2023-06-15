//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRESOURCEUPLOADER_H
#define RAMSES_IRESOURCEUPLOADER_H

#include "RendererAPI/Types.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Resource/ResourceTypes.h"
#include "Components/ManagedResource.h"
#include "SceneAPI/SceneId.h"
#include <optional>

namespace ramses_internal
{
    class IResource;
    class IRenderBackend;
    struct ResourceDescriptor;

    class IResourceUploader
    {
    public:
        virtual ~IResourceUploader() {}

        virtual std::optional<DeviceResourceHandle> uploadResource(IRenderBackend& renderBackend, const ResourceDescriptor& resourceObject, uint32_t& outVRAMSize) = 0;
        virtual void                 unloadResource(IRenderBackend& renderBackend, EResourceType type, ResourceContentHash hash, DeviceResourceHandle handle) = 0;
        virtual void                 storeShaderInBinaryShaderCache(IRenderBackend& renderBackend, DeviceResourceHandle deviceHandle, const ResourceContentHash& hash, SceneId sceneid) = 0;
    };
}

#endif
