//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRESOURCE_H
#define RAMSES_IRESOURCE_H

#include "SceneAPI/ResourceContentHash.h"
#include "Collections/String.h"
#include "Resource/ResourceTypes.h"

namespace ramses_internal
{
    class IOutputStream;

    class IResource
    {
    public:

        // ordered from least/fastest compression to best/slowest compression
        enum class CompressionLevel
        {
            None,
            Realtime,
            Offline,
        };

        IResource() {}
        IResource(const IResource&) = delete;
        IResource(IResource&&) = delete;

        IResource& operator=(const IResource&) = delete;
        IResource& operator=(IResource&&) = delete;

        virtual ~IResource(){};
        [[nodiscard]] virtual const ResourceBlob& getResourceData() const = 0;
        [[nodiscard]] virtual const CompressedResourceBlob& getCompressedResourceData() const = 0;
        [[nodiscard]] virtual UInt32 getDecompressedDataSize() const = 0;
        [[nodiscard]] virtual UInt32 getCompressedDataSize() const = 0;
        virtual void setResourceData(ResourceBlob data) = 0;
        virtual void setResourceData(ResourceBlob data, const ResourceContentHash& hash) = 0;
        virtual void setCompressedResourceData(CompressedResourceBlob compressedData, CompressionLevel compressionLevel, uint32_t uncompressedSize, const ResourceContentHash& hash) = 0;
        [[nodiscard]] virtual EResourceType getTypeID() const = 0;
        [[nodiscard]] virtual const ResourceContentHash& getHash() const = 0;
        virtual void compress(CompressionLevel level) const = 0;
        virtual void decompress() const = 0;
        [[nodiscard]] virtual bool isCompressedAvailable() const = 0;
        [[nodiscard]] virtual bool isDeCompressedAvailable() const = 0;
        [[nodiscard]] virtual ResourceCacheFlag getCacheFlag() const = 0;
        [[nodiscard]] virtual const String& getName() const = 0;

        virtual void serializeResourceMetadataToStream(IOutputStream& output) const = 0;

        template<typename T>
        [[nodiscard]] T* convertTo()
        {
            return static_cast<T*>(this);
        }

        template<typename T>
        [[nodiscard]] const T* convertTo() const
        {
            return static_cast<const T*>(this);
        }
    };
}

#endif
