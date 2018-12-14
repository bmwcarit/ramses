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
        return stream.getSize();
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
            const CompressedSceneResourceData& compressedData = resource.getCompressedResourceData();
            output.write(compressedData->getRawData(), compressedData->size());
        }
        else
        {
            // write uncompressed data to stream
            const SceneResourceData& data = resource.getResourceData();
            output.write(data->getRawData(), data->size());
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
                CompressedSceneResourceData compressedData;
                //do not use infos.compressedSize as this information might not have been available when resourceInfo was sent if resource was compressed later
                compressedData.reset(new CompressedMemoryBlob(header.compressedSize, header.decompressedSize));
                input.read(reinterpret_cast<Char*>(compressedData->getRawData()), header.compressedSize);
                header.resource->setCompressedResourceData(compressedData, hash);
            }
            else
            {
                // read uncompressed data from stream
                SceneResourceData uncompressedData;
                uncompressedData.reset(new MemoryBlob(header.decompressedSize));
                input.read(reinterpret_cast<Char*>(uncompressedData->getRawData()), header.decompressedSize);
                header.resource->setResourceData(uncompressedData, hash);
            }
        }
        return header.resource;
    }
}
