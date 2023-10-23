//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "ManagedResource.h"
#include "internal/PlatformAbstraction/Collections/Pair.h"
#include <memory>

namespace ramses::internal
{
    class IOutputStream;
    class IInputStream;
    class BinaryFileOutputStream;
    struct ResourceFileEntry;

    class ResourcePersistation
    {
    public:
        static void WriteNamedResourcesWithTOCToStream(IOutputStream& outStream, const ManagedResourceVector& resourcesForFile, bool compress);
        static void WriteOneResourceToStream(IOutputStream& outStream, const ManagedResource& resource);

        static std::unique_ptr<IResource> ReadOneResourceFromStream(IInputStream& inStream, const ResourceContentHash& hash);
        static std::unique_ptr<IResource> RetrieveResourceFromStream(IInputStream& inStream, const ResourceFileEntry& entry);
    };
}
