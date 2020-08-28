//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/SingleResourceSerialization.h"
#include "Components/ResourceSerializationHelper.h"
#include "Resource/IResource.h"
#include "Collections/IOutputStream.h"
#include "Resource/EResourceCompressionStatus.h"
#include "Utils/VoidOutputStream.h"
#include "Collections/IInputStream.h"

namespace ramses_internal
{
    UInt32 SingleResourceSerialization::SizeOfSerializedResource(const IResource& resource)
    {
        VoidOutputStream stream;
        SerializeResource(stream, resource);
        return static_cast<uint32_t>(stream.getSize());
    }

    void SingleResourceSerialization::SerializeResource(IOutputStream& output, const IResource& resource)
    {
        // header
        ResourceSerializationHelper::SerializeResourceMetadata(output, resource);

        // data blob
        // prefer compressed if available
        if (resource.isCompressedAvailable())
        {
            // write compressed data to stream
            const CompressedResouceBlob& compressedData = resource.getCompressedResourceData();
            output.write(compressedData.data(), static_cast<uint32_t>(compressedData.size()));
        }
        else
        {
            // write uncompressed data to stream
            const ResourceBlob& data = resource.getResourceData();
            output.write(data.data(), static_cast<uint32_t>(data.size()));
        }
    }

    IResource* SingleResourceSerialization::DeserializeResource(IInputStream& input, ResourceContentHash hash)
    {
        // header
        ResourceSerializationHelper::DeserializedResourceHeader header = ResourceSerializationHelper::ResourceFromMetadataStream(input);
        assert(header.resource != nullptr);

        if (header.resource)
        {
            // data blob
            if (header.compressionStatus == EResourceCompressionStatus_Compressed)
            {
                // read compressed data from stream
                CompressedResouceBlob compressedData(header.compressedSize);
                input.read(reinterpret_cast<char*>(compressedData.data()), static_cast<uint32_t>(compressedData.size()));
                header.resource->setCompressedResourceData(std::move(compressedData), header.decompressedSize, hash);
            }
            else
            {
                // read uncompressed data from stream
                ResourceBlob uncompressedData(header.decompressedSize);
                input.read(reinterpret_cast<char*>(uncompressedData.data()), static_cast<uint32_t>(uncompressedData.size()));
                header.resource->setResourceData(std::move(uncompressedData), hash);
            }
        }
        return header.resource;
    }
}
