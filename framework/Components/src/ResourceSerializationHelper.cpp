//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/ResourceSerializationHelper.h"
#include "Utils/VoidOutputStream.h"
#include "Resource/TextureResource.h"
#include "Resource/ArrayResource.h"
#include "Resource/EffectResource.h"
#include "Utils/LogMacros.h"

#include <string>

namespace ramses_internal
{
    namespace ResourceSerializationHelper
    {
        void SerializeResourceMetadata(IOutputStream& output, const IResource& resource)
        {
            output << static_cast<uint32_t>(resource.getTypeID());
            output << resource.getName();

            // prefer compressed if available
            output << static_cast<uint32_t>(resource.isCompressedAvailable() ? EResourceCompressionStatus_Compressed : EResourceCompressionStatus_Uncompressed);
            output << resource.getCompressedDataSize();
            output << resource.getDecompressedDataSize();
            output << resource.getCacheFlag().getValue();

            resource.serializeResourceMetadataToStream(output);
        }

        uint32_t ResourceMetadataSize(const IResource& resource)
        {
            VoidOutputStream stream;
            SerializeResourceMetadata(stream, resource);
            return static_cast<uint32_t>(stream.getSize());
        }

        DeserializedResourceHeader ResourceFromMetadataStream(IInputStream& input)
        {
            uint32_t resourceTypeValue = 0;
            std::string name;
            uint32_t compressionStatusValue = 0;
            uint32_t compressedSize = 0;
            uint32_t decompressedSize = 0;
            uint32_t cacheFlagValue = 0;

            input >> resourceTypeValue;
            input >> name;
            input >> compressionStatusValue;
            input >> compressedSize;
            input >> decompressedSize;
            input >> cacheFlagValue;

            const ResourceCacheFlag cacheFlag(cacheFlagValue);
            const EResourceType resourceType = static_cast<EResourceType>(resourceTypeValue);
            const EResourceCompressionStatus compressionStatus = static_cast<EResourceCompressionStatus>(compressionStatusValue);

            std::unique_ptr<IResource> resource;
            switch (resourceType)
            {
            case EResourceType_Texture2D:
            case EResourceType_Texture3D:
            case EResourceType_TextureCube:
                resource = TextureResource::CreateResourceFromMetadataStream(input, resourceType, cacheFlag, name);
                break;
            case EResourceType_VertexArray:
            case EResourceType_IndexArray:
                resource = ArrayResource::CreateResourceFromMetadataStream(input, cacheFlag, resourceType, name);
                break;
            case EResourceType_Effect:
                resource = EffectResource::CreateResourceFromMetadataStream(input, cacheFlag, name);
                break;
            default:
                LOG_ERROR_P(CONTEXT_FRAMEWORK, "ResourceSerializationHelper::ResourceFromMetadataStream: Failed for unknown resource type {}", resourceType);
            }

            DeserializedResourceHeader result;
            result.resource = std::move(resource);
            result.compressionStatus = compressionStatus;
            result.decompressedSize = decompressedSize;
            result.compressedSize = compressedSize;
            return result;
        }
    }
}
