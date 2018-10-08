//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/ResourceStorage.h"
#include "Components/ResourceDeleterCallingCallback.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "Components/IResourceStorageChangeListener.h"
#include "Utils/StringUtils.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    ResourceStorage::ResourceStorage(PlatformLock& lockToUse)
        : m_resourceMapLock(lockToUse)
        , m_listener(0)
        , m_bytesCurrentlyUsedByResourcesInMemory(0)
    {
    }

    ResourceStorage::~ResourceStorage()
    {
        assert(m_resourceMap.count() == 0);  //all ManagedResources have be destructed before ResourceStorage is destructed
    }

    void ResourceStorage::setListener(IResourceStorageChangeListener& listener)
    {
        m_listener = &listener;
    }

    ResourceInfoVector ResourceStorage::getAllResourceInfo() const
    {
        PlatformGuard lock(m_resourceMapLock);
        ResourceInfoVector result;
        for (const auto& item : m_resourceMap)
        {
            result.push_back(item.value.resourceInfo);
        }
        return result;
    }

    ManagedResourceVector ResourceStorage::getResources()
    {
        ManagedResourceVector result;
        m_resourceMapLock.lock();
        for (const auto& item : m_resourceMap)
        {
            if (item.value.resource != 0)
            {
                ManagedResource res = createManagedResource(item.value.resource);
                result.push_back(res);
            }
        }
        m_resourceMapLock.unlock();
        return result;
    }

    ramses_internal::ManagedResource ResourceStorage::getResource(ResourceContentHash hash)
    {
        PlatformGuard lock(m_resourceMapLock);
        RefCntResource* entry = m_resourceMap.get(hash);
        if (entry && entry->resource != 0)
        {
            ManagedResource managedRes = createManagedResource(entry->resource);
            return managedRes;
        }

        return ManagedResource();
    }

    ManagedResource ResourceStorage::createManagedResource(const IResource* resource)
    {
        ResourceDeleterCallingCallback deleter(*this);
        ManagedResource managedRes(*resource, deleter);
        referenceResource(resource->getHash());
        return managedRes;
    }

    void ResourceStorage::referenceResource(const ResourceContentHash& hash)
    {
        RefCntResource* entry = m_resourceMap.get(hash);
        assert(entry);
        ++entry->refCount;
    }


    ramses_internal::ResourceHashUsage ResourceStorage::getResourceHashUsage(const ResourceContentHash& hash)
    {
        ResourceHashUsageCallback deleter(*this);
        ResourceContentHash* hashObjectToUse = 0;
        m_resourceMapLock.lock();
        RefCntResource* entry = m_resourceMap.get(hash);
        if (entry)
        {
            ++entry->hashUsages;
            hashObjectToUse = entry->hash;
        }
        else
        {
            RefCntResource newEntry;
            newEntry.refCount = 0;
            newEntry.hashUsages = 1;
            newEntry.resource = 0;
            newEntry.deletionAllowed = false;
            newEntry.hash = new ResourceContentHash(hash);
            m_resourceMap.put(hash, newEntry);
            hashObjectToUse = newEntry.hash;
        }
        m_resourceMapLock.unlock();

        return ResourceHashUsage(*hashObjectToUse, deleter);
    }

    void ResourceStorage::storeResourceInfo(const ResourceContentHash& hash, const ResourceInfo& resourceInfo)
    {
        RefCntResource* entry = m_resourceMap.get(hash);
        if (entry)
        {
            entry->resourceInfo = resourceInfo;
        }
        else
        {
            RefCntResource newEntry;
            newEntry.refCount = 0;
            newEntry.hashUsages = 0;
            newEntry.resource = 0;
            newEntry.resourceInfo = resourceInfo;
            newEntry.deletionAllowed = false;
            newEntry.hash = new ResourceContentHash(hash);
            m_resourceMap.put(hash, newEntry);
        }
    }

    const ResourceInfo& ResourceStorage::getResourceInfo(const ResourceContentHash& hash) const
    {
        RefCntResource* entry = m_resourceMap.get(hash);
        assert(entry);
        //if real resource is available, update internally stored information first (could have changed, e.g. due to compression)
        if (entry->resource != 0)
        {
            entry->resourceInfo = ResourceInfo(entry->resource);
        }
        return entry->resourceInfo;
    }

    ramses_internal::ManagedResource ResourceStorage::manageResource(const IResource& resource, Bool deletionAllowed)
    {
        ResourceDeleterCallingCallback deleter(*this);

        const ResourceContentHash hash = resource.getHash();
        LOG_TRACE(CONTEXT_FRAMEWORK, "Adding resource:" << StringUtils::HexFromResourceContentHash(hash));
        const IResource* resourceToReturn = 0;
        m_resourceMapLock.lock();
        RefCntResource* entry = m_resourceMap.get(hash);
        if (entry)
        {
            ++entry->refCount;
            if (0 == entry->resource)
            {
                entry->resource = &resource;
                entry->resourceInfo = ResourceInfo(&resource);
                m_bytesCurrentlyUsedByResourcesInMemory += resource.getDecompressedDataSize();
            }
            else
            {
                // resource hash already exists, don't keep deplicate copy
                delete &resource;
            }

            if (deletionAllowed)
            {
                // only update when deletion is now allowed, never downgrade
                entry->deletionAllowed = deletionAllowed;
            }

            resourceToReturn = entry->resource;
        }
        else
        {
            RefCntResource newEntry;
            newEntry.refCount = 1;
            newEntry.hashUsages = 0;
            newEntry.resource = &resource;
            newEntry.resourceInfo = ResourceInfo(&resource);
            newEntry.deletionAllowed = deletionAllowed;
            newEntry.hash = new ResourceContentHash(hash);
            resourceToReturn = &resource;
            m_resourceMap.put(hash, newEntry);
            m_bytesCurrentlyUsedByResourcesInMemory += resourceToReturn->getDecompressedDataSize();
        }

        assert(resourceToReturn->getDecompressedDataSize() > 0);
        m_resourceMapLock.unlock();

        ManagedResource managedRes(*resourceToReturn, deleter);
        return managedRes;
    }

    void ResourceStorage::resourceHashUsageZero(const ResourceContentHash& hash)
    {
        LOG_TRACE(CONTEXT_FRAMEWORK, "ResourceStorage::resourceHashUsageZero resource:" << StringUtils::HexFromResourceContentHash(hash));
        m_resourceMapLock.lock();
        RefCntResource* entry = m_resourceMap.get(hash);
        assert(entry && entry->hashUsages > 0);
        --entry->hashUsages;
        checkForDeletion(*entry, hash);
        m_resourceMapLock.unlock();
    }

    uint64_t ResourceStorage::getBytesUsedByResourcesInMemory() const
    {
        return m_bytesCurrentlyUsedByResourcesInMemory;
    }

    void ResourceStorage::managedResourceDeleted(const IResource& resourceToRemove)
    {
        const ResourceContentHash hashToRemove = resourceToRemove.getHash();
        LOG_TRACE(CONTEXT_FRAMEWORK, "ResourceStorage::managedResourceDeleted unreference resource:" << StringUtils::HexFromResourceContentHash(hashToRemove));
        m_resourceMapLock.lock();
        RefCntResource* entry = m_resourceMap.get(hashToRemove);
        assert(entry && entry->refCount > 0);
        --entry->refCount;
        assert(entry->resource == &resourceToRemove);
        checkForDeletion(*entry, hashToRemove);
        m_resourceMapLock.unlock();
    }

    void ResourceStorage::checkForDeletion(RefCntResource& entry, const ResourceContentHash& hash)
    {
        // refcount zero is mandatory for deletion, it means noone has pointers to the data right now
        if (0 == entry.refCount)
        {
            // actual deletion is ok, if either
            //   - also noone has hashreference (noone knows anything of the resource anymore)
            //   - we have hashreference but can obtain the resource again should it be needed, ie deletion allowed flag is set
            if (0 == entry.hashUsages || entry.deletionAllowed)
            {
                const IResource* internalResource = entry.resource;
                if (internalResource)
                {
                    const UInt32 decompressedSize = internalResource->getDecompressedDataSize();
                    assert(decompressedSize > 0);
                    assert(decompressedSize <= m_bytesCurrentlyUsedByResourcesInMemory);
                    if (m_bytesCurrentlyUsedByResourcesInMemory >= decompressedSize)
                    {
                        m_bytesCurrentlyUsedByResourcesInMemory -= decompressedSize;
                    }
                    else
                    {
                        LOG_ERROR(CONTEXT_FRAMEWORK, "ResourceStorage::checkForDeletion:: prevented underflow of m_bytesCurrentlyUsedByResourcesInMemory (" << m_bytesCurrentlyUsedByResourcesInMemory << " bytes) from releasing resource: " << internalResource->getHash() << " (" << decompressedSize << " bytes)");
                        m_bytesCurrentlyUsedByResourcesInMemory = 0;
                    }
                }
                delete internalResource;
                entry.resource = 0;

                if (entry.hashUsages == 0)
                {
                    LOG_TRACE(CONTEXT_FRAMEWORK, "ResourceStorage::checkForDeletion hashusages is zero, really delete:" << StringUtils::HexFromResourceContentHash(hash));
                    ResourceContentHash* hashObject = entry.hash;
                    m_resourceMap.remove(hash);
                    delete hashObject;
                }
                if (0 != m_listener)
                {
                    m_listener->onBytesNeededByStorageDecreased(m_bytesCurrentlyUsedByResourcesInMemory);
                }
            }
        }
    }

    void ResourceStorage::reserveResourceCount(uint32_t totalCount)
    {
        m_resourceMapLock.lock();
        m_resourceMap.reserve(totalCount);
        m_resourceMapLock.unlock();
    }

}
