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
#include "Resource/EResourceType.h"
#include "Components/ManagedResource.h"

namespace ramses_internal
{
    class IResource;
    class IRenderBackend;

    class IResourceUploader
    {
    public:
        virtual ~IResourceUploader() {}

        virtual DeviceResourceHandle uploadResource(IRenderBackend& renderBackend, ManagedResource resourceObject, UInt32& outVRAMSize) = 0;
        virtual void                 unloadResource(IRenderBackend& renderBackend, EResourceType type, ResourceContentHash hash, DeviceResourceHandle handle) = 0;
    };
}

#endif
