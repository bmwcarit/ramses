//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRENDERERRESOURCECACHE_H
#define RAMSES_IRENDERERRESOURCECACHE_H

#include "Resource/IResource.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/ResourceContentHash.h"
#include "RendererAPI/Types.h"

namespace ramses_internal
{
    // Internal version of ramses::IRendererResourceCache; same members, but using the corresponding internal datatypes
    class IRendererResourceCache
    {
    public:
        virtual ~IRendererResourceCache() {};

        virtual bool hasResource(ResourceContentHash resourceId, UInt32& size) const = 0;
        virtual bool getResourceData(ResourceContentHash resourceId, uint8_t* buffer, UInt32 bufferSize) const = 0;
        virtual bool shouldResourceBeCached(ResourceContentHash resourceId, UInt32 resourceDataSize, ResourceCacheFlag cacheFlag, SceneId sceneId) const = 0;
        virtual void storeResource(ResourceContentHash resourceId, const uint8_t* resourceData, UInt32 resourceDataSize, ResourceCacheFlag cacheFlag, SceneId sceneId) = 0;
    };
}

#endif
