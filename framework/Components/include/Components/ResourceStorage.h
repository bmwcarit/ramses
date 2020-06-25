//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCESTORAGE_H
#define RAMSES_RESOURCESTORAGE_H

#include "PlatformAbstraction/PlatformLock.h"
#include "Components/ManagedResource.h"
#include "IResourceHashUsageCallback.h"
#include "Components/ResourceHashUsage.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Collections/HashMap.h"
#include "Resource/ResourceInfo.h"

namespace ramses_internal
{
    class IResourceStorageChangeListener;

    class ResourceStorage: public IManagedResourceDeleterCallback, public IResourceHashUsageCallback
    {
        struct RefCntResource
        {
            int hashUsages;
            int refCount;
            ResourceContentHash* hash;
            const IResource* resource;
            ResourceInfo resourceInfo;
            bool deletionAllowed;
        };
    public:
        explicit ResourceStorage(PlatformLock& lockToUse);
        virtual ~ResourceStorage();

        void setListener(IResourceStorageChangeListener& listener);
        ResourceInfoVector getAllResourceInfo() const;
        ManagedResource manageResource(const IResource& resource, bool deletionAllowed = false);
        ManagedResourceVector getResources();
        ManagedResource getResource(ResourceContentHash hash);
        ResourceHashUsage getResourceHashUsage(const ResourceContentHash& hash);
        void storeResourceInfo(const ResourceContentHash& hash, const ResourceInfo& resourceInfo);
        const ResourceInfo& getResourceInfo(const ResourceContentHash& hash) const;

        virtual void managedResourceDeleted(const IResource& resourceToRemove) override;
        virtual void resourceHashUsageZero(const ResourceContentHash& hash) override;
        uint64_t getBytesUsedByResourcesInMemory() const;
        void reserveResourceCount(uint32_t totalCount);
    private:
        ManagedResource createManagedResource(const IResource* resource);
        void referenceResource(const ResourceContentHash& hash);
        void checkForDeletion(RefCntResource& entry, const ResourceContentHash& hash);

        PlatformLock& m_resourceMapLock;

        typedef HashMap<ResourceContentHash, RefCntResource> ResourceMap;
        ResourceMap m_resourceMap;
        IResourceStorageChangeListener* m_listener;
        uint64_t m_bytesCurrentlyUsedByResourcesInMemory;
    };
}

#endif
