//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Resource/ResourceTypes.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "IResource.h"

namespace ramses::internal
{
    struct ResourceInfo
    {
        EResourceType       type{EResourceType::Invalid};
        ResourceContentHash hash;
        uint32_t            decompressedSize{0u};
        uint32_t            compressedSize{0u};

        ResourceInfo(const EResourceType type_, const ResourceContentHash& hash_, const uint32_t decompressedSize_, const uint32_t compressedSize_)
            : type(type_)
            , hash(hash_)
            , decompressedSize(decompressedSize_)
            , compressedSize(compressedSize_)
        {
        }

        ResourceInfo() = default;

        explicit ResourceInfo(const IResource* resourceToGetDataFrom)
            : type(resourceToGetDataFrom->getTypeID())
            , hash(resourceToGetDataFrom->getHash())
            , decompressedSize(resourceToGetDataFrom->getDecompressedDataSize())
            , compressedSize(resourceToGetDataFrom->getCompressedDataSize())
        {
        }

        bool operator==(const ResourceInfo& other) const
        {
            return (type == other.type)
                && (hash == other.hash)
                && (decompressedSize == other.decompressedSize)
                && (compressedSize == other.compressedSize);
        }

        bool operator!=(const ResourceInfo& other) const
        {
            return !operator==(other);
        }
    };

    using ResourceInfoVector = std::vector<ResourceInfo>;
}
