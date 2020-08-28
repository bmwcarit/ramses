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

namespace ramses_internal
{
    namespace ResourceSerializationHelper
    {
        static IResource*(*gInvalidResourceFun)(IInputStream&, ResourceCacheFlag, const String&) = nullptr;

        void SerializeResourceMetadata(IOutputStream& output, const IResource& resource)
        {
            output << static_cast<UInt32>(resource.getTypeID());
            output << resource.getName();

            // prefer compressed if available
            output << static_cast<UInt32>(resource.isCompressedAvailable() ? EResourceCompressionStatus_Compressed : EResourceCompressionStatus_Uncompressed);
            output << resource.getCompressedDataSize();
            output << resource.getDecompressedDataSize();
            output << resource.getCacheFlag().getValue();

            resource.serializeResourceMetadataToStream(output);
        }

        UInt32 ResourceMetadataSize(const IResource& resource)
        {
            VoidOutputStream stream;
            SerializeResourceMetadata(stream, resource);
            return static_cast<uint32_t>(stream.getSize());
        }

        DeserializedResourceHeader ResourceFromMetadataStream(IInputStream& input)
        {
            UInt32 resourceTypeValue = 0;
            String name;
            UInt32 compressionStatusValue = 0;
            UInt32 compressedSize = 0;
            UInt32 decompressedSize = 0;
            UInt32 cacheFlagValue = 0;

            input >> resourceTypeValue;
            input >> name;
            input >> compressionStatusValue;
            input >> compressedSize;
            input >> decompressedSize;
            input >> cacheFlagValue;

            const ResourceCacheFlag cacheFlag(cacheFlagValue);
            const EResourceType resourceType = static_cast<EResourceType>(resourceTypeValue);
            const EResourceCompressionStatus compressionStatus = static_cast<EResourceCompressionStatus>(compressionStatusValue);

            IResource* resource = nullptr;
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
            case EResourceType_Invalid:
                if (gInvalidResourceFun)
                {
                    resource = gInvalidResourceFun(input, cacheFlag, name);
                }
                else
                {
                    LOG_ERROR(CONTEXT_FRAMEWORK, "ResourceSerializationHelper::ResourceFromMetadataStream: Failed to deserialize unknown resource type " << resourceType);
                    assert(false);
                }
                break;
            default:
                assert(false);
            }

            DeserializedResourceHeader result;
            result.resource = resource;
            result.compressionStatus = compressionStatus;
            result.decompressedSize = decompressedSize;
            result.compressedSize = compressedSize;
            return result;
        }

        void SetInvalidCreateResourceFromMetadataStreamFunction(IResource*(*fun)(IInputStream&, ResourceCacheFlag, const String&))
        {
            gInvalidResourceFun = fun;
        }
    }
}
