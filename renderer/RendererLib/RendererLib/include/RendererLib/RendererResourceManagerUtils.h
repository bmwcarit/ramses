//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERRESOURCEMANAGERUTILS_H
#define RAMSES_RENDERERRESOURCEMANAGERUTILS_H

#include "Components/ManagedResource.h"
#include "SceneAPI/SceneId.h"

namespace ramses_internal
{
    class IResource;
    class IRendererResourceCache;

    class RendererResourceManagerUtils
    {
    public:

        static ManagedResource TryLoadResource(ResourceContentHash resourceId, UInt32 resourceSize, const IRendererResourceCache* cache);
        static void StoreResource(IRendererResourceCache* cache, const IResource* resource, SceneId sceneId);
    };
}

#endif
