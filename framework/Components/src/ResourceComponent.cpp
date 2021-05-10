//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/ResourceComponent.h"
#include "Components/ResourceTableOfContents.h"
#include "Components/ResourceFilesRegistry.h"
#include "Components/SceneFileHandle.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    ResourceComponent::ResourceComponent(StatisticCollectionFramework& statistics, PlatformLock& frameworkLock)
        : m_resourceStorage(frameworkLock, statistics)
        , m_statistics(statistics)
    {
    }

    ResourceComponent::~ResourceComponent()
    {
    }

    ramses_internal::ManagedResource ResourceComponent::getResource(ResourceContentHash hash)
    {
        return m_resourceStorage.getResource(hash);
    }

    ramses_internal::ResourceHashUsage ResourceComponent::getResourceHashUsage(const ResourceContentHash& hash)
    {
        return m_resourceStorage.getResourceHashUsage(hash);
    }

    ManagedResourceVector ResourceComponent::getResources()
    {
        return m_resourceStorage.getResources();
    }

    ramses_internal::ManagedResource ResourceComponent::manageResource(const IResource& resource, bool deletionAllowed)
    {
        return m_resourceStorage.manageResource(resource, deletionAllowed);
    }

    SceneFileHandle ResourceComponent::addResourceFile(InputStreamContainerSPtr resourceFileInputStream, const ramses_internal::ResourceTableOfContents& toc)
    {
        for (const auto& item : toc.getFileContents())
        {
            m_resourceStorage.storeResourceInfo(item.key, item.value.resourceInfo);
        }
        return m_resourceFiles.registerResourceFile(resourceFileInputStream, toc, m_resourceStorage);
    }

    void ResourceComponent::loadResourceFromFile(SceneFileHandle handle)
    {
        // If resources of a file are loaded, check if they are in use by any scene object (=hashusage) or as a resource
        // a) If they are in use, we need to load them from file, also remove the deletion allowed flag from
        // them, because they is not supposed to be loadable anymore.
        // b) If a resource is unused, nothing is to be done since there wouldn't be any entry in the resource storage for it
        const FileContentsMap* content = m_resourceFiles.getContentsOfResourceFile(handle);
        if (!content)
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "ResourceComponent::loadResourceFromFile: handle " << handle << " unknown, can't force load");
            return;
        }

        for (auto const& entry : *content)
        {
            auto const& id = entry.key;
            if (m_resourceStorage.isFileResourceInUseAnywhereElse(id))
            {
                ManagedResource res;
                if (!m_resourceStorage.getResource(id))
                    res = loadResource(id);

                m_resourceStorage.markDeletionDisallowed(id);
            }
        }
    }

    void ResourceComponent::removeResourceFile(SceneFileHandle handle)
    {
        m_resourceFiles.unregisterResourceFile(handle);
    }

    bool ResourceComponent::hasResourceFile(SceneFileHandle handle) const
    {
        return m_resourceFiles.getContentsOfResourceFile(handle) != nullptr;
    }

    ManagedResource ResourceComponent::loadResource(const ResourceContentHash& hash)
    {
        IInputStream* resourceStream = nullptr;
        ResourceFileEntry entry;
        const EStatus canLoadFromFile = m_resourceFiles.getEntry(hash, resourceStream, entry);
        if (canLoadFromFile == EStatus::Ok)
        {
            m_statistics.statResourcesLoadedFromFileNumber.incCounter(1);
            m_statistics.statResourcesLoadedFromFileSize.incCounter(entry.sizeInBytes);

            IResource* lowLevelResource = ResourcePersistation::RetrieveResourceFromStream(*resourceStream, entry);
            if (lowLevelResource)
                return m_resourceStorage.manageResource(*lowLevelResource, true);
            else
                LOG_ERROR_P(CONTEXT_CLIENT, "ResourceComponent::loadResource: RetrieveResourceFromStream failed for {} ({})", entry.resourceInfo.type, entry.resourceInfo.hash);
        }

        return ManagedResource();
    }

    void ResourceComponent::reserveResourceCount(uint32_t totalCount)
    {
        m_resourceStorage.reserveResourceCount(totalCount);
    }

    ramses_internal::ManagedResourceVector ResourceComponent::resolveResources(ResourceContentHashVector& hashes)
    {
        ManagedResourceVector result;
        result.reserve(hashes.size());
        ResourceContentHashVector failed;
        for (auto& hash : hashes)
        {
            ManagedResource mr = getResource(hash);
            if (!mr)
                mr = loadResource(hash);

            if (mr)
                result.push_back(mr);
            else
                failed.push_back(hash);
        }

        if (!failed.empty())
            LOG_ERROR_P(CONTEXT_CLIENT, "ResourceComponent::resolveResources: failed to load resources: {}", failed);

        return result;
    }

    ResourceInfo const& ResourceComponent::getResourceInfo(ResourceContentHash const& hash)
    {
        return m_resourceStorage.getResourceInfo(hash);
    }
}
