//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Components/ResourceHashUsage.h"
#include "internal/Components/InputStreamContainer.h"
#include "internal/Components/SceneFileHandle.h"
#include "ManagedResource.h"
#include "internal/SceneGraph/Resource/ResourceInfo.h"

namespace ramses::internal
{
    class Guid;
    class ResourceTableOfContents;

    class IResourceProviderComponent
    {
    public:
        virtual ~IResourceProviderComponent() = default;

        virtual ManagedResource manageResource(const IResource& resource) = 0;
        virtual ManagedResource getResource(ResourceContentHash hash) = 0;
        virtual ManagedResource loadResource(const ResourceContentHash& hash) = 0;

        virtual ResourceHashUsage getResourceHashUsage(const ResourceContentHash& hash) = 0;
        virtual SceneFileHandle addResourceFile(InputStreamContainerSPtr resourceFileInputStream, const ResourceTableOfContents& toc) = 0;
        virtual void loadResourceFromFile(SceneFileHandle handle) = 0;
        virtual void removeResourceFile(SceneFileHandle handle) = 0;
        [[nodiscard]] virtual bool hasResourceFile(SceneFileHandle handle) const = 0;

        virtual void reserveResourceCount(uint32_t totalCount) = 0;

        virtual ManagedResourceVector resolveResources(ResourceContentHashVector& vec) = 0;
        virtual ResourceInfo const& getResourceInfo(ResourceContentHash const& hash) = 0;

        [[nodiscard]] virtual bool knowsResource(const ResourceContentHash& hash) const = 0;
    };
}
