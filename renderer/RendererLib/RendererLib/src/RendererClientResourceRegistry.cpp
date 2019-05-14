//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererClientResourceRegistry.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    void RendererClientResourceRegistry::registerResource(const ResourceContentHash& hash)
    {
        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceRegistry::registerResource Registering resource #" << StringUtils::HexFromResourceContentHash(hash));
        if (m_resources.contains(hash))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::registerResource Resource already registered! #" << StringUtils::HexFromResourceContentHash(hash) << " (" << EnumToString(getResourceDescriptor(hash).type) << " : " << EnumToString(getResourceDescriptor(hash).status) << ")");
            assert(false);
            return;
        }

        ResourceDescriptor rd;
        rd.hash = hash;

        m_resources.put(hash, rd);
        m_stateChangeSequences[hash].fill('\0');
        setResourceStatus(hash, EResourceStatus_Registered);
    }

    void RendererClientResourceRegistry::unregisterResource(const ResourceContentHash& hash)
    {
        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceRegistry::unregisterResource Unregistering resource #" << StringUtils::HexFromResourceContentHash(hash));
        if (!m_resources.contains(hash))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::unregisterResource Resource not registered! #" << StringUtils::HexFromResourceContentHash(hash));
            assert(false);
            return;
        }
        if (!m_resources.get(hash)->sceneUsage.empty())
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::unregisterResource Resource is still being referenced by one or more scenes! #" << StringUtils::HexFromResourceContentHash(hash));
            assert(false);
            return;
        }

        setResourceStatus(hash, EResourceStatus_Unknown);
        m_resources.remove(hash);

        updateListOfResourcesNotInUseByScenes(hash);
    }

    Bool RendererClientResourceRegistry::containsResource(const ResourceContentHash& hash) const
    {
        return m_resources.contains(hash);
    }

    void RendererClientResourceRegistry::addResourceRef(const ResourceContentHash& hash, SceneId sceneId)
    {
        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceRegistry::addResourceRef for scene (" << sceneId.getValue() << ") resource #" << StringUtils::HexFromResourceContentHash(hash));
        if (!m_resources.contains(hash))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::addResourceRef Resource not registered! #" << StringUtils::HexFromResourceContentHash(hash));
            assert(false);
            return;
        }

        ResourceDescriptor& rd = *m_resources.get(hash);
        if (contains_c(rd.sceneUsage, sceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::addResourceRef Resource already referenced by scene (" << sceneId.getValue() << ")! #" << StringUtils::HexFromResourceContentHash(hash) << " (" << EnumToString(rd.type) << " : " << EnumToString(rd.status) << ")");
            assert(false);
            return;
        }
        rd.sceneUsage.push_back(sceneId);

        updateListOfResourcesNotInUseByScenes(hash);
    }

    void RendererClientResourceRegistry::removeResourceRef(const ResourceContentHash& hash, SceneId sceneId)
    {
        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceRegistry::removeResourceRef for scene (" << sceneId.getValue() << ") resource #" << StringUtils::HexFromResourceContentHash(hash));
        if (!m_resources.contains(hash))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::removeResourceRef Resource not registered! #" << StringUtils::HexFromResourceContentHash(hash));
            assert(false);
            return;
        }

        ResourceDescriptor& rd = *m_resources.get(hash);
        if (!contains_c(rd.sceneUsage, sceneId))
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::removeResourceRef Resource not referenced by scene (" << sceneId.getValue() << ")! #" << StringUtils::HexFromResourceContentHash(hash));
            assert(false);
            return;
        }
        rd.sceneUsage.erase(find_c(rd.sceneUsage, sceneId));

        updateListOfResourcesNotInUseByScenes(hash);
    }

    const ResourceDescriptor& RendererClientResourceRegistry::getResourceDescriptor(const ResourceContentHash& hash) const
    {
        const auto res = m_resources.get(hash);
        if (res == nullptr)
        {
            LOG_ERROR(CONTEXT_RENDERER, "RendererResourceRegistry::getResourceDescriptor Resource not registered! #" << StringUtils::HexFromResourceContentHash(hash));
            assert(false);
            static const ResourceDescriptor DummyRD = {};
            return DummyRD;
        }
        return *res;
    }

    void RendererClientResourceRegistry::setResourceData(const ResourceContentHash& hash, ManagedResource resourceObject, DeviceResourceHandle deviceHandle, EResourceType resourceType)
    {
        assert(m_resources.contains(hash));
        ResourceDescriptor& rd = *m_resources.get(hash);
        rd.resource = resourceObject;
        rd.deviceHandle = deviceHandle;
        rd.type = resourceType;
    }

    void RendererClientResourceRegistry::setResourceSize(const ResourceContentHash& hash, UInt32 compressedSize, UInt32 decompressedSize, UInt32 vramSize)
    {
        assert(m_resources.contains(hash));
        ResourceDescriptor& rd = *m_resources.get(hash);
        rd.compressedSize = compressedSize;
        rd.decompressedSize = decompressedSize;
        rd.vramSize = vramSize;
    }

    void RendererClientResourceRegistry::setResourceStatus(const ResourceContentHash& hash, EResourceStatus status, UInt64 updateFrameCounter)
    {
        assert(m_resources.contains(hash));
        ResourceDescriptor& rd = *m_resources.get(hash);
        rd.lastStatusChangeFrameIdx = updateFrameCounter;
        if (status == EResourceStatus_Requested)
            rd.lastRequestFrameIdx = updateFrameCounter;
        LOG_TRACE(CONTEXT_RENDERER, "RendererResourceRegistry::setResourceStatus resource #" << StringUtils::HexFromResourceContentHash(hash) << " status change: " << EnumToString(rd.status) << " -> " << EnumToString(status));
        assert(ValidateStatusChange(rd.status, status));

        updateCachedLists(hash, rd.status, status);

        rd.status = status;

        auto& seqStr = m_stateChangeSequences[hash];
        auto it = std::find(seqStr.begin(), seqStr.end(), '\0');
        if (it == seqStr.end()) // buffer full, throw away oldest state
            it = std::rotate(seqStr.begin(), seqStr.begin() + 1, seqStr.end());
        *it = '0' + static_cast<char>(status);
    }

    EResourceStatus RendererClientResourceRegistry::getResourceStatus(const ResourceContentHash& hash) const
    {
        assert(m_resources.contains(hash));
        return m_resources.get(hash)->status;
    }

    const ResourceDescriptors& RendererClientResourceRegistry::getAllResourceDescriptors() const
    {
        return m_resources;
    }

    const ResourceContentHashVector& RendererClientResourceRegistry::getAllRegisteredResources() const
    {
        return m_registeredResources;
    }

    const ResourceContentHashVector& RendererClientResourceRegistry::getAllRequestedResources() const
    {
        return m_requestedResources;
    }

    const ResourceContentHashVector& RendererClientResourceRegistry::getAllProvidedResources() const
    {
        return m_providedResources;
    }

    const ResourceContentHashVector& RendererClientResourceRegistry::getAllResourcesNotInUseByScenes() const
    {
        return m_resourcesNotInUseByScenes;
    }

    const ResourceContentHashVector& RendererClientResourceRegistry::getAllResourcesNotInUseByScenesAndNotUploaded() const
    {
        return m_resourcesNotInUseByScenesAndNotUploaded;
    }

    void RendererClientResourceRegistry::updateCachedLists(const ResourceContentHash& hash, EResourceStatus currentStatus, EResourceStatus newStatus)
    {
        if (currentStatus == EResourceStatus_Registered)
        {
            assert(contains_c(m_registeredResources, hash));
            m_registeredResources.erase(find_c(m_registeredResources, hash));
        }
        else if (currentStatus == EResourceStatus_Requested)
        {
            assert(contains_c(m_requestedResources, hash));
            m_requestedResources.erase(find_c(m_requestedResources, hash));
        }
        else if (currentStatus == EResourceStatus_Provided)
        {
            assert(contains_c(m_providedResources, hash));
            m_providedResources.erase(find_c(m_providedResources, hash));
        }

        if (newStatus == EResourceStatus_Registered)
        {
            assert(!contains_c(m_registeredResources, hash));
            m_registeredResources.push_back(hash);
        }
        else if (newStatus == EResourceStatus_Requested)
        {
            assert(!contains_c(m_requestedResources, hash));
            m_requestedResources.push_back(hash);
        }
        else if (newStatus == EResourceStatus_Provided)
        {
            assert(!contains_c(m_providedResources, hash));
            m_providedResources.push_back(hash);
        }
    }

    void RendererClientResourceRegistry::updateListOfResourcesNotInUseByScenes(const ResourceContentHash& hash)
    {
        const ResourceDescriptor* rd = m_resources.get(hash);
        const Bool isUnused = ((rd != NULL) && rd->sceneUsage.empty());

        {
            ResourceContentHashVector::iterator it = find_c(m_resourcesNotInUseByScenes, hash);
            const Bool isContained = (it != m_resourcesNotInUseByScenes.end());
            if (isUnused)
            {
                // put to list if not already contained
                if (!isContained)
                {
                    m_resourcesNotInUseByScenes.push_back(hash);
                }
            }
            else
            {
                // remove from list if contained
                if (isContained)
                {
                    m_resourcesNotInUseByScenes.erase(it);
                }
            }
        }

        {
            ResourceContentHashVector::iterator it = find_c(m_resourcesNotInUseByScenesAndNotUploaded, hash);
            const Bool isContained = (it != m_resourcesNotInUseByScenesAndNotUploaded.end());
            const Bool isUploaded = (rd != NULL) && (rd->status == EResourceStatus_Uploaded);
            if (isUnused && !isUploaded)
            {
                // put to list if not already contained
                if (!isContained)
                {
                    m_resourcesNotInUseByScenesAndNotUploaded.push_back(hash);
                }
            }
            else
            {
                // remove from list if contained
                if (isContained)
                {
                    m_resourcesNotInUseByScenesAndNotUploaded.erase(it);
                }
            }
        }
    }

    Bool RendererClientResourceRegistry::ValidateStatusChange(EResourceStatus currentStatus, EResourceStatus newStatus)
    {
        if (currentStatus == newStatus)
        {
            // it's allowed to stay in requested because of rerequests
            if (currentStatus == EResourceStatus_Requested)
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        switch (newStatus)
        {
        case EResourceStatus_Registered:
            return currentStatus == EResourceStatus_Unknown;
        case EResourceStatus_Requested:
            return currentStatus == EResourceStatus_Registered;
        case EResourceStatus_Provided:
            return currentStatus == EResourceStatus_Requested;
        case EResourceStatus_Uploaded:
        case EResourceStatus_Broken:
            return currentStatus == EResourceStatus_Provided;
        case EResourceStatus_Unknown:
            return true;
        default:
            return false;
        }
    }
}
