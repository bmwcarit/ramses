//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Resource/ResourceBase.h"

namespace ramses::internal
{
    class BufferResource : public ResourceBase
    {
    public:
        BufferResource(EResourceType typeID, uint32_t dataSize, const void* data, std::string_view name)
            : ResourceBase(typeID, name)
        {
            // TODO(tobias) this might create an empty resource blob that will be thrown away later.
            // needs more refactoring to solve properly
            if (dataSize != 0u)
                setResourceData(ResourceBlob(dataSize, static_cast<const std::byte*>(data)));
        }
    };
}
