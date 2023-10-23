//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"

#include <cstdint>
#include <memory>

namespace ramses::internal
{
    class IOutputStream;
    class IResource;
    class IInputStream;

    class SingleResourceSerialization
    {
    public:
        static uint32_t SizeOfSerializedResource(const IResource& resource);
        static void SerializeResource(IOutputStream& output, const IResource& resource);

        static std::unique_ptr<IResource> DeserializeResource(IInputStream& input, ResourceContentHash hash);
    };
}
