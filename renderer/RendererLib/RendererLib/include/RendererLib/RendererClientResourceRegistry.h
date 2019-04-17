//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCLIENTRESOURCEREGISTRY_H
#define RAMSES_RENDERERCLIENTRESOURCEREGISTRY_H

#include "RendererLib/ResourceDescriptor.h"
#include "Transfer/ResourceTypes.h"
#include <array>

namespace ramses_internal
{
    class RendererClientResourceRegistry
    {
    public:
        void                       registerResource     (const ResourceContentHash& hash);
        void                       unregisterResource   (const ResourceContentHash& hash);
        Bool                       containsResource     (const ResourceContentHash& hash) const;

        void                       addResourceRef       (const ResourceContentHash& hash, SceneId sceneId);
        void                       removeResourceRef    (const ResourceContentHash& hash, SceneId sceneId);

        void                       setResourceStatus    (const ResourceContentHash& hash, EResourceStatus status, UInt64 updateFrameCounter = 0);
        EResourceStatus            getResourceStatus    (const ResourceContentHash& hash) const;
        const ResourceDescriptor&  getResourceDescriptor(const ResourceContentHash& hash) const;

        void                       setResourceData      (const ResourceContentHash& hash, ManagedResource resourceObject, DeviceResourceHandle deviceHandle, EResourceType resourceType);
        void                       setResourceSize      (const ResourceContentHash& hash, UInt32 compressedSize, UInt32 decompressedSize, UInt32 vramSize);

        const ResourceDescriptors& getAllResourceDescriptors() const;

        const ResourceContentHashVector& getAllRegisteredResources() const;
        const ResourceContentHashVector& getAllRequestedResources() const;
        const ResourceContentHashVector& getAllProvidedResources() const;
        const ResourceContentHashVector& getAllResourcesNotInUseByScenes() const;
        const ResourceContentHashVector& getAllResourcesNotInUseByScenesAndNotUploaded() const;

    private:
        void updateCachedLists(const ResourceContentHash& hash, EResourceStatus currentStatus, EResourceStatus newStatus);
        void updateListOfResourcesNotInUseByScenes(const ResourceContentHash& hash);
        static Bool ValidateStatusChange(EResourceStatus currentStatus, EResourceStatus newStatus);

        ResourceDescriptors m_resources;

        // These are cached lists of resources with certain status to optimize querying
        ResourceContentHashVector m_registeredResources;
        ResourceContentHashVector m_requestedResources;
        ResourceContentHashVector m_providedResources;
        ResourceContentHashVector m_resourcesNotInUseByScenes;
        ResourceContentHashVector m_resourcesNotInUseByScenesAndNotUploaded;

        // For logging purposes only
        mutable HashMap<ResourceContentHash, std::array<char, 16>> m_stateChangeSequences;
        friend class RendererLogger;
    };
}

#endif
