//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/SceneUpdateSerializationHelper.h"
#include "Utils/VectorBinaryOutputStream.h"
#include "Scene/SceneActionCollection.h"
#include "Utils/LogMacros.h"
#include "Utils/BinaryInputStream.h"
#include "Components/ResourceSerializationHelper.h"

namespace ramses_internal
{
    namespace SceneActionSerialization
    {
        absl::Span<const Byte> SerializeDescription(const SceneActionCollection& actions, std::vector<Byte>& workingMemory)
        {
            VectorBinaryOutputStream os(workingMemory);
            os << static_cast<uint32_t>(actions.numberOfActions());
            for (const auto& sa : actions)
                os << static_cast<uint32_t>(sa.type())
                   << static_cast<uint32_t>(sa.offsetInCollection());
            return workingMemory;
        }

        absl::Span<const Byte> SerializeData(const SceneActionCollection& actions)
        {
            return actions.collectionData();
        }

        SceneActionCollection Deserialize(absl::Span<const Byte> description, absl::Span<const Byte> data)
        {
            BinaryInputStream is(description.data());
            uint32_t numActions = 0;
            is >> numActions;
            assert(description.size() == numActions*2*sizeof(uint32_t) + sizeof(uint32_t));

            SceneActionCollection actions(data.size(), numActions);
            actions.appendRawData(data.data(), data.size());
            for (uint32_t i = 0; i < numActions; ++i)
            {
                uint32_t type = 0;
                uint32_t offset = 0;
                is >> type
                   >> offset;
                actions.addRawSceneActionInformation(static_cast<ESceneActionId>(type), offset);
            }
            return actions;
        }
    }

    namespace ResourceSerialization
    {
        absl::Span<const Byte> SerializeDescription(const IResource& resource, std::vector<Byte>& workingMemory)
        {
            VectorBinaryOutputStream os(workingMemory);
            os << resource.getHash();  // hash must be outside because metadata and blob is used to calculate hash
            ResourceSerializationHelper::SerializeResourceMetadata(os, resource);
            return workingMemory;
        }

        absl::Span<const Byte> SerializeData(const IResource& resource)
        {
            // prefer compressed if available
            if (resource.isCompressedAvailable())
                return resource.getCompressedResourceData();
            else if (resource.isDeCompressedAvailable())
                return resource.getResourceData();
            else
                return {};
        }

        std::unique_ptr<IResource> Deserialize(absl::Span<const Byte> description, absl::Span<const Byte> data)
        {
            BinaryInputStream is(description.data());
            ResourceContentHash hash;
            is >> hash;
            ResourceSerializationHelper::DeserializedResourceHeader header =
                ResourceSerializationHelper::ResourceFromMetadataStream(is);
            std::unique_ptr<IResource> resource(header.resource);
            const size_t expectedDataSize = header.compressionStatus == EResourceCompressionStatus_Compressed ?
                header.compressedSize : header.decompressedSize;
            if (data.size() != expectedDataSize)
            {
                LOG_ERROR_P(CONTEXT_FRAMEWORK, "ResourceSerialization::Deserialize: Expected resource size {} but got {}, compression state {}",
                            expectedDataSize, data.size(), header.compressionStatus);
                return nullptr;
            }
            if (data.size() > 0)
            {
                if (header.compressionStatus == EResourceCompressionStatus_Compressed)
                    resource->setCompressedResourceData(CompressedResouceBlob(data.size(), data.data()), header.decompressedSize, hash);
                else
                    resource->setResourceData(ResourceBlob(data.size(), data.data()), hash);
            }
            return resource;
        }
    }
}
