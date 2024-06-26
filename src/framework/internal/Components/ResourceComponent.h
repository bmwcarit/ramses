//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ResourceStorage.h"
#include "internal/Components/ResourceHashUsage.h"
#include "ResourceFilesRegistry.h"

#include "IResourceProviderComponent.h"
#include "internal/Core/Utils/StatisticCollection.h"

namespace ramses::internal
{
    class ResourceComponent : public IResourceProviderComponent
    {
    public:
        ResourceComponent(StatisticCollectionFramework& statistics, PlatformLock& frameworkLock, EFeatureLevel featureLevel);
        ~ResourceComponent() override;

        // implement IResourceProviderComponent
        ManagedResource manageResource(const IResource& resource) override;
        ManagedResource getResource(ResourceContentHash hash) override;
        ManagedResource loadResource(const ResourceContentHash& hash) override;

        ResourceHashUsage getResourceHashUsage(const ResourceContentHash& hash) override;
        SceneFileHandle addResourceFile(InputStreamContainerSPtr resourceFileInputStream, const ResourceTableOfContents& toc) override;
        void loadResourceFromFile(SceneFileHandle handle) override;
        void removeResourceFile(SceneFileHandle handle) override;
        [[nodiscard]] bool hasResourceFile(SceneFileHandle handle) const override;

        void reserveResourceCount(uint32_t totalCount) override;
        ManagedResourceVector resolveResources(ResourceContentHashVector& hashes) override;

        ResourceInfo const& getResourceInfo(ResourceContentHash const& hash) override;
        [[nodiscard]] bool knowsResource(const ResourceContentHash& hash) const override;

        ManagedResourceVector getResources();

        ManagedResource manageResourceDeletionAllowed(const IResource& resource);

    private:
        ResourceStorage m_resourceStorage;
        ResourceFilesRegistry m_resourceFiles;

        StatisticCollectionFramework& m_statistics;
        EFeatureLevel m_featureLevel = EFeatureLevel_Latest;
    };
}
