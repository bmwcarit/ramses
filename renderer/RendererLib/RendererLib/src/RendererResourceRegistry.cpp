//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererResourceRegistry.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    void RendererResourceRegistry::registerResource(const ResourceContentHash& hash)
    {
        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceRegistry::registerResource Registering resource #" << hash);
        if (m_resources.contains(hash))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::registerResource Resource already registered! #" << hash << " (" << getResourceDescriptor(hash).type << " : " << getResourceDescriptor(hash).status << ")");
            assert(false);
            return;
        }

        ResourceDescriptor rd;
        rd.hash = hash;
        rd.status = EResourceStatus::Registered;
        m_resources.put(hash, rd);
    }

    void RendererResourceRegistry::unregisterResource(const ResourceContentHash& hash)
    {
        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceRegistry::unregisterResource Unregistering resource #" << hash);
        if (!m_resources.contains(hash))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::unregisterResource Resource not registered! #" << hash);
            assert(false);
            return;
        }
        const auto& rd = *m_resources.get(hash);
        if (!rd.sceneUsage.empty())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::unregisterResource Resource is still being referenced by one or more scenes! #" << hash);
            assert(false);
            return;
        }

        assert(rd.status != EResourceStatus::ScheduledForUpload);
        updateCachedLists(hash, rd.status, EResourceStatus::Registered);

        m_resources.remove(hash);
        updateListOfResourcesNotInUseByScenes(hash);
    }

    Bool RendererResourceRegistry::containsResource(const ResourceContentHash& hash) const
    {
        return m_resources.contains(hash);
    }

    void RendererResourceRegistry::addResourceRef(const ResourceContentHash& hash, SceneId sceneId)
    {
        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceRegistry::addResourceRef for scene (" << sceneId << ") resource #" << hash);
        if (!m_resources.contains(hash))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::addResourceRef Resource not registered! #" << hash);
            assert(false);
            return;
        }

        auto& sceneUsage = m_resources.get(hash)->sceneUsage;
        if (!contains_c(sceneUsage, sceneId))
            m_resourcesUsedInScenes[sceneId].push_back(hash);
        sceneUsage.push_back(sceneId);
        updateListOfResourcesNotInUseByScenes(hash);
    }

    void RendererResourceRegistry::removeResourceRef(const ResourceContentHash& hash, SceneId sceneId)
    {
        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceRegistry::removeResourceRef for scene (" << sceneId << ") resource #" << hash);
        if (!m_resources.contains(hash))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::removeResourceRef Resource not registered! #" << hash);
            assert(false);
            return;
        }

        ResourceDescriptor& rd = *m_resources.get(hash);
        if (!contains_c(rd.sceneUsage, sceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::removeResourceRef Resource not referenced by scene (" << sceneId << ")! #" << hash);
            assert(false);
            return;
        }

        rd.sceneUsage.erase(find_c(rd.sceneUsage, sceneId));
        if (!contains_c(rd.sceneUsage, sceneId))
        {
            assert(contains_c(m_resourcesUsedInScenes[sceneId], hash));
            auto& resourcesUsed = m_resourcesUsedInScenes[sceneId];
            resourcesUsed.erase(find_c(resourcesUsed, hash));
            if (resourcesUsed.empty())
                m_resourcesUsedInScenes.erase(sceneId);
        }

        updateListOfResourcesNotInUseByScenes(hash);

        // unregister resource if not used by any scene (if scheduled for upload or uploaded it has to wait to be unloaded first)
        if (rd.sceneUsage.empty() && rd.status != EResourceStatus::Uploaded && rd.status != EResourceStatus::ScheduledForUpload)
            unregisterResource(hash);
    }

    const ResourceDescriptor& RendererResourceRegistry::getResourceDescriptor(const ResourceContentHash& hash) const
    {
        const auto res = m_resources.get(hash);
        if (res == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::getResourceDescriptor Resource not registered! #" << hash);
            assert(false);
            static const ResourceDescriptor DummyRD = {};
            return DummyRD;
        }
        return *res;
    }

    void RendererResourceRegistry::setResourceData(const ResourceContentHash& hash, const ManagedResource& resourceObject)
    {
        assert(m_resources.contains(hash));
        assert(resourceObject);
        ResourceDescriptor& rd = *m_resources.get(hash);
        rd.resource = resourceObject;
        rd.type = resourceObject->getTypeID();
        rd.compressedSize = resourceObject->getCompressedDataSize();
        rd.decompressedSize = resourceObject->getDecompressedDataSize();

        setResourceStatus(hash, EResourceStatus::Provided);
    }

    void RendererResourceRegistry::setResourceScheduledForUpload(const ResourceContentHash& hash)
    {
        assert(m_resources.contains(hash));
        assert(!m_resources.get(hash)->deviceHandle.isValid());

        setResourceStatus(hash, EResourceStatus::ScheduledForUpload);
    }

    void RendererResourceRegistry::setResourceUploaded(const ResourceContentHash& hash, DeviceResourceHandle deviceHandle, UInt32 vramSize)
    {
        assert(m_resources.contains(hash));
        ResourceDescriptor& rd = *m_resources.get(hash);
        assert(!rd.deviceHandle.isValid());
        rd.deviceHandle = deviceHandle;
        rd.vramSize = vramSize;
        // release resource data
        rd.resource.reset();

        setResourceStatus(hash, EResourceStatus::Uploaded);
    }

    void RendererResourceRegistry::setResourceBroken(const ResourceContentHash& hash)
    {
        // release resource data
        m_resources.get(hash)->resource.reset();

        setResourceStatus(hash, EResourceStatus::Broken);
    }

    void RendererResourceRegistry::setResourceStatus(const ResourceContentHash& hash, EResourceStatus status)
    {
        assert(m_resources.contains(hash));
        ResourceDescriptor& rd = *m_resources.get(hash);
        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceRegistry::setResourceStatus resource " << hash << " status change: " << rd.status << " -> " << status);
        assert(ValidateStatusChange(rd.status, status));

        updateCachedLists(hash, rd.status, status);

        rd.status = status;
    }

    EResourceStatus RendererResourceRegistry::getResourceStatus(const ResourceContentHash& hash) const
    {
        assert(m_resources.contains(hash));
        return m_resources.get(hash)->status;
    }

    const ResourceDescriptors& RendererResourceRegistry::getAllResourceDescriptors() const
    {
        return m_resources;
    }

    const ResourceContentHashVector& RendererResourceRegistry::getAllProvidedResources() const
    {
        return m_providedResources;
    }

    const ResourceContentHashVector& RendererResourceRegistry::getAllResourcesNotInUseByScenes() const
    {
        return m_resourcesNotInUseByScenes;
    }

    const ResourceContentHashVector* RendererResourceRegistry::getResourcesInUseByScene(SceneId sceneId) const
    {
        const auto it = m_resourcesUsedInScenes.find(sceneId);
        return it != m_resourcesUsedInScenes.cend() ? &it->second : nullptr;
    }

    bool RendererResourceRegistry::hasAnyResourcesScheduledForUpload() const
    {
        return m_countResourcesScheduledForUpload > 0u;
    }

    void RendererResourceRegistry::updateCachedLists(const ResourceContentHash& hash, EResourceStatus currentStatus, EResourceStatus newStatus)
    {
        if (currentStatus == EResourceStatus::Provided)
        {
            assert(contains_c(m_providedResources, hash));
            m_providedResources.erase(find_c(m_providedResources, hash));
        }
        else if (currentStatus == EResourceStatus::ScheduledForUpload)
        {
            assert(m_countResourcesScheduledForUpload > 0u);
            --m_countResourcesScheduledForUpload;
        }

        if (newStatus == EResourceStatus::Provided)
        {
            assert(!contains_c(m_providedResources, hash));
            m_providedResources.push_back(hash);
        }
        else if (newStatus == EResourceStatus::ScheduledForUpload)
            ++m_countResourcesScheduledForUpload;
    }

    void RendererResourceRegistry::updateListOfResourcesNotInUseByScenes(const ResourceContentHash& hash)
    {
        const ResourceDescriptor* rd = m_resources.get(hash);
        const Bool isUnused = ((rd != nullptr) && rd->sceneUsage.empty());

        {
            ResourceContentHashVector::iterator it = find_c(m_resourcesNotInUseByScenes, hash);
            const Bool isContained = (it != m_resourcesNotInUseByScenes.end());
            if (isUnused)
            {
                // put to list if not already contained
                if (!isContained)
                    m_resourcesNotInUseByScenes.push_back(hash);
            }
            else
            {
                // remove from list if contained
                if (isContained)
                    m_resourcesNotInUseByScenes.erase(it);
            }
        }
    }

    Bool RendererResourceRegistry::ValidateStatusChange(EResourceStatus currentStatus, EResourceStatus newStatus)
    {
        if (currentStatus == newStatus)
            return false;

        switch (newStatus)
        {
        case EResourceStatus::Provided:
            return currentStatus == EResourceStatus::Registered;
        case EResourceStatus::Uploaded:
        case EResourceStatus::Broken:
            return currentStatus == EResourceStatus::Provided || currentStatus == EResourceStatus::ScheduledForUpload;
        case EResourceStatus::ScheduledForUpload:
            return currentStatus == EResourceStatus::Provided;
        case EResourceStatus::Registered:
            return false;
        }

        return false;
    }
}
