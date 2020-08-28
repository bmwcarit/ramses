//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRESOURCEPROVIDERCOMPONENT_H
#define RAMSES_IRESOURCEPROVIDERCOMPONENT_H

#include "Components/ResourceHashUsage.h"
#include "Components/ResourceFileInputStream.h"
#include "ManagedResource.h"
#include "Collections/String.h"

namespace ramses_internal
{
    class Guid;
    class ResourceTableOfContents;

    class IResourceProviderComponent
    {
    public:
        virtual ~IResourceProviderComponent() {}

        virtual ManagedResource manageResource(const IResource& resource, bool deletionAllowed = false) = 0;
        virtual ManagedResourceVector getResources() = 0;
        virtual ManagedResource getResource(ResourceContentHash hash) = 0;
        virtual ManagedResource forceLoadResource(const ResourceContentHash& hash) = 0;

        virtual ResourceHashUsage getResourceHashUsage(const ResourceContentHash& hash) = 0;
        virtual void addResourceFile(ResourceFileInputStreamSPtr resourceFileInputStream, const ResourceTableOfContents& toc) = 0;
        virtual bool hasResourceFile(const String& resourceFileName) const = 0;
        virtual void forceLoadFromResourceFile(const String& resourceFileName) = 0;
        virtual void removeResourceFile(const String& resourceFileName) = 0;

        virtual void reserveResourceCount(uint32_t totalCount) = 0;

        virtual ManagedResourceVector resolveResources(ResourceContentHashVector& vec) = 0;

    };
}

#endif
