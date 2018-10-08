//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SINGLERESOURCESERIALIZATION_H
#define RAMSES_SINGLERESOURCESERIALIZATION_H

#include "PlatformAbstraction/PlatformTypeInfo.h"
#include "SceneAPI/ResourceContentHash.h"

namespace ramses_internal
{
    class IOutputStream;
    class IResource;
    class IInputStream;

    class SingleResourceSerialization
    {
    public:
        static UInt32 SizeOfSerializedResource(const IResource& resource);
        static void SerializeResource(IOutputStream& output, const IResource& resource);

        static IResource* DeserializeResource(IInputStream& input, ResourceContentHash hash);
    };
}

#endif
