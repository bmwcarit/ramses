//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERRESOURCEREGISTRY_H
#define RAMSES_RENDERERRESOURCEREGISTRY_H

#include "RendererLib/ResourceDescriptor.h"
#include <unordered_map>
#include <array>

namespace ramses_internal
{
    class RendererResourceRegistry
    {
    public:
        void                       registerResource     (const ResourceContentHash& hash);
        void                       unregisterResource   (const ResourceContentHash& hash);
        [[nodiscard]] bool                       containsResource     (const ResourceContentHash& hash) const;

        void                       setResourceData      (const ResourceContentHash& hash, const ManagedResource& resourceObject);
        void                       setResourceScheduledForUpload(const ResourceContentHash& hash);
        void                       setResourceUploaded  (const ResourceContentHash& hash, DeviceResourceHandle deviceHandle, uint32_t vramSize);
        void                       setResourceBroken    (const ResourceContentHash& hash);

        void                       addResourceRef       (const ResourceContentHash& hash, SceneId sceneId);
        void                       removeResourceRef    (const ResourceContentHash& hash, SceneId sceneId);

        [[nodiscard]] EResourceStatus            getResourceStatus    (const ResourceContentHash& hash) const;
        [[nodiscard]] const ResourceDescriptor&  getResourceDescriptor(const ResourceContentHash& hash) const;

        [[nodiscard]] const ResourceDescriptors& getAllResourceDescriptors() const;
        [[nodiscard]] const ResourceContentHashVector& getAllProvidedResources() const;
        [[nodiscard]] const ResourceContentHashVector& getAllResourcesNotInUseByScenes() const;
        [[nodiscard]] const ResourceContentHashVector* getResourcesInUseByScene(SceneId sceneId) const;
        [[nodiscard]] bool hasAnyResourcesScheduledForUpload() const;

    private:
        void setResourceStatus(const ResourceContentHash& hash, EResourceStatus status);
        void updateCachedLists(const ResourceContentHash& hash, EResourceStatus currentStatus, EResourceStatus newStatus);
        void updateListOfResourcesNotInUseByScenes(const ResourceContentHash& hash);
        static bool ValidateStatusChange(EResourceStatus currentStatus, EResourceStatus newStatus);

        ResourceDescriptors m_resources;

        // These are cached lists of resources to optimize querying for resources to be uploaded and unloaded
        ResourceContentHashVector m_providedResources;
        ResourceContentHashVector m_resourcesNotInUseByScenes;
        std::unordered_map<SceneId, ResourceContentHashVector> m_resourcesUsedInScenes;
        uint32_t m_countResourcesScheduledForUpload = 0u;

        // For logging purposes only
        friend class RendererLogger;
    };
}

#endif
