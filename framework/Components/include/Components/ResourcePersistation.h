//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEPERSISTATION_H
#define RAMSES_RESOURCEPERSISTATION_H

#include "Collections/Vector.h"
#include "ManagedResource.h"
#include "Collections/Pair.h"
#include "Transfer/ResourceTypes.h"

namespace ramses_internal
{
    class IOutputStream;
    class IInputStream;
    class BinaryFileInputStream;
    class BinaryFileOutputStream;
    struct ResourceFileEntry;

    class ResourcePersistation
    {
    public:
        static void WriteNamedResourcesWithTOCToStream(BinaryFileOutputStream& outStream, const ManagedResourceVector& resourcesForFile, bool compress);
        static void WriteOneResourceToStream(IOutputStream& outStream, const ManagedResource& resource);

        static IResource* ReadOneResourceFromStream(IInputStream& inStream, const ResourceContentHash& hash);
        static IResource* RetrieveResourceFromStream(BinaryFileInputStream& inStream, const ResourceFileEntry& entry);
    };
}

#endif
