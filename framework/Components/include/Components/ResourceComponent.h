//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCECOMPONENT_H
#define RAMSES_RESOURCECOMPONENT_H

#include "ResourceStorage.h"
#include "Components/ResourceHashUsage.h"
#include "ResourceFilesRegistry.h"

#include "IResourceProviderComponent.h"
#include "Utils/StatisticCollection.h"

namespace ramses_internal
{
    class ResourceComponent : public IResourceProviderComponent
    {
    public:
        ResourceComponent(StatisticCollectionFramework& statistics, PlatformLock& frameworkLock);
        virtual ~ResourceComponent() override;

        // implement IResourceProviderComponent
        virtual ManagedResource manageResource(const IResource& resource, bool deletionAllowed = false) override;
        virtual ManagedResource getResource(ResourceContentHash hash) override;
        virtual ManagedResource loadResource(const ResourceContentHash& hash) override;

        virtual ResourceHashUsage getResourceHashUsage(const ResourceContentHash& hash) override;
        virtual void addResourceFile(ResourceFileInputStreamSPtr resourceFileInputStream, const ramses_internal::ResourceTableOfContents& toc) override;
        virtual bool hasResourceFile(const String& resourceFileName) const override;
        virtual void loadResourceFromFile(const String& resourceFileName) override;
        virtual void removeResourceFile(const String& resourceFileName) override;

        virtual void reserveResourceCount(uint32_t totalCount) override;
        virtual ManagedResourceVector resolveResources(ResourceContentHashVector& vec) override;

        virtual ResourceInfo const& getResourceInfo(ResourceContentHash const& hash) override;

        ManagedResourceVector getResources();

    private:
        ResourceStorage m_resourceStorage;
        ResourceFilesRegistry m_resourceFiles;

        StatisticCollectionFramework& m_statistics;
    };
}

#endif
