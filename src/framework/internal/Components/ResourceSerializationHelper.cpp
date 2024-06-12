//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Components/ResourceSerializationHelper.h"
#include "internal/Core/Utils/VoidOutputStream.h"
#include "internal/SceneGraph/Resource/TextureResource.h"
#include "internal/SceneGraph/Resource/ArrayResource.h"
#include "internal/SceneGraph/Resource/EffectResource.h"
#include "internal/Core/Utils/LogMacros.h"

#include <string>

namespace ramses::internal
{
    namespace ResourceSerializationHelper
    {
        void SerializeResourceMetadata(IOutputStream& output, const IResource& resource)
        {
            output << static_cast<uint32_t>(resource.getTypeID());
            output << resource.getName();

            // prefer compressed if available
            output << static_cast<uint32_t>(resource.isCompressedAvailable() ? EResourceCompressionStatus::Compressed : EResourceCompressionStatus::Uncompressed);
            output << resource.getCompressedDataSize();
            output << resource.getDecompressedDataSize();

            resource.serializeResourceMetadataToStream(output);
        }

        uint32_t ResourceMetadataSize(const IResource& resource)
        {
            VoidOutputStream stream;
            SerializeResourceMetadata(stream, resource);
            return static_cast<uint32_t>(stream.getSize());
        }

        DeserializedResourceHeader ResourceFromMetadataStream(IInputStream& input, EFeatureLevel featureLevel)
        {
            uint32_t resourceTypeValue = 0;
            std::string name;
            uint32_t compressionStatusValue = 0;
            uint32_t compressedSize = 0;
            uint32_t decompressedSize = 0;

            input >> resourceTypeValue;
            input >> name;
            input >> compressionStatusValue;
            input >> compressedSize;
            input >> decompressedSize;

            const auto resourceType = static_cast<EResourceType>(resourceTypeValue);
            const auto compressionStatus = static_cast<EResourceCompressionStatus>(compressionStatusValue);

            std::unique_ptr<IResource> resource;
            switch (resourceType)
            {
            case EResourceType::Texture2D:
            case EResourceType::Texture3D:
            case EResourceType::TextureCube:
                resource = TextureResource::CreateResourceFromMetadataStream(input, resourceType, name);
                break;
            case EResourceType::VertexArray:
            case EResourceType::IndexArray:
                resource = ArrayResource::CreateResourceFromMetadataStream(input, resourceType, name);
                break;
            case EResourceType::Effect:
                resource = EffectResource::CreateResourceFromMetadataStream(input, name, featureLevel);
                break;
            default:
                LOG_ERROR(CONTEXT_FRAMEWORK, "ResourceSerializationHelper::ResourceFromMetadataStream: Failed for unknown resource type {}", resourceType);
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
