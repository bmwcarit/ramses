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
#include "Components/IManagedResourceDeleterCallback.h"
#include "Components/ResourceHashUsage.h"
#include "Components/IResourceHashUsageCallback.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Collections/HashMap.h"
#include "Resource/ResourceInfo.h"
#include "Utils/StatisticCollection.h"

namespace ramses_internal
{
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
        explicit ResourceStorage(PlatformLock& lockToUse, StatisticCollectionFramework& statistics);
        ~ResourceStorage() override;

        [[nodiscard]] ResourceInfoVector getAllResourceInfo() const;
        ManagedResource manageResource(const IResource& resource, bool deletionAllowed = false);
        ManagedResourceVector getResources();
        ManagedResource getResource(ResourceContentHash hash);
        ResourceHashUsage getResourceHashUsage(const ResourceContentHash& hash);
        void storeResourceInfo(const ResourceContentHash& hash, const ResourceInfo& resourceInfo);
        [[nodiscard]] const ResourceInfo& getResourceInfo(const ResourceContentHash& hash) const;

        void managedResourceDeleted(const IResource& resourceToRemove) override;
        void resourceHashUsageZero(const ResourceContentHash& hash) override;
        void reserveResourceCount(uint32_t totalCount);

        void markDeletionDisallowed(const ResourceContentHash& hash);
        bool isFileResourceInUseAnywhereElse(const ResourceContentHash& hash);
        [[nodiscard]] bool knowsResource(const ResourceContentHash& hash) const;

    private:
        ManagedResource createManagedResource(const IResource* resource);
        void referenceResource(const ResourceContentHash& hash);
        void checkForDeletion(RefCntResource& entry, const ResourceContentHash& hash);

        PlatformLock& m_resourceMapLock;
        StatisticCollectionFramework& m_statistics;

        using ResourceMap = HashMap<ResourceContentHash, RefCntResource>;
        ResourceMap m_resourceMap;
    };
}

#endif
