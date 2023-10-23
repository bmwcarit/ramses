//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/PlatformLock.h"
#include "internal/Components/ManagedResource.h"
#include "internal/Components/IManagedResourceDeleterCallback.h"
#include "internal/Components/ResourceHashUsage.h"
#include "internal/Components/IResourceHashUsageCallback.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/SceneGraph/Resource/ResourceInfo.h"
#include "internal/Core/Utils/StatisticCollection.h"

namespace ramses::internal
{
    class ResourceStorage: public IManagedResourceDeleterCallback, public IResourceHashUsageCallback
    {
        struct RefCntResource
        {
            int                  hashUsages{1};
            int                  refCount{0};
            ResourceContentHash* hash{nullptr};
            const IResource*     resource{nullptr};
            ResourceInfo         resourceInfo;
            bool                 deletionAllowed{false};
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
