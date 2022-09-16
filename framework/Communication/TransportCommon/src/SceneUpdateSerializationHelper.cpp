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
#include "Components/FlushInformation.h"


namespace
{
    template <typename ELEMENTTYPE>
    void putDataArray(ramses_internal::VectorBinaryOutputStream& stream, const std::vector<ELEMENTTYPE>& dataArray)
    {
        const ramses_internal::UInt32 numElements = static_cast<ramses_internal::UInt32>(dataArray.size());
        stream << numElements;
        if (numElements > 0u)
        {
            const ramses_internal::UInt32 size = numElements * sizeof(ELEMENTTYPE);
            stream << size;
            stream.write(dataArray.data(), size);
        }
    }

    template <typename ELEMENTTYPE>
    void getDataArray(ramses_internal::BinaryInputStream& in, std::vector<ELEMENTTYPE>& dataArray)
    {
        assert(dataArray.empty());
        ramses_internal::UInt32 numElements = 0u;
        in >> numElements;

        if (numElements > 0u)
        {
            ramses_internal::UInt32 size = 0u;
            in >> size;
            dataArray.resize(numElements);
            in.read(dataArray.data(), size);
        }
    }
}

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

    namespace FlushInformationSerialization
    {
        enum class FlushBits : uint8_t
        {
            HasSizeInfo = 1u,
            HasEffectTimeSync = 2u
        };

        absl::Span<const Byte> SerializeInfos(const FlushInformation& flushInfos, std::vector<Byte>& workingMemory)
        {
            const UInt estimatedDataSize =
                sizeof(flushInfos.containsValidInformation) +
                sizeof(flushInfos.flushCounter) +
                sizeof(uint8_t) + // flushflags
                (flushInfos.hasSizeInfo ? sizeof(SceneSizeInformation) : 0) +
                flushInfos.resourceChanges.getPutSizeEstimate() +
                sizeof(uint32_t) + sizeof(SceneReferenceAction) * flushInfos.sceneReferences.size() +
                2 * sizeof(uint64_t) + sizeof(uint32_t) + // time info
                sizeof(SceneVersionTag::BaseType);
            workingMemory.reserve(workingMemory.size() + estimatedDataSize);

            VectorBinaryOutputStream os(workingMemory);
            uint8_t flushFlags = 0u;
            if (flushInfos.hasSizeInfo)
            {
                flushFlags |= static_cast<uint8_t>(FlushBits::HasSizeInfo);
            }
            if (flushInfos.flushTimeInfo.isEffectTimeSync)
            {
                flushFlags |= static_cast<uint8_t>(FlushBits::HasEffectTimeSync);
            }

            os << flushInfos.containsValidInformation;
            os << flushInfos.flushCounter;
            os << flushFlags;
            if (flushInfos.hasSizeInfo)
            {
                os << flushInfos.sizeInfo.nodeCount;
                os << flushInfos.sizeInfo.cameraCount;
                os << flushInfos.sizeInfo.transformCount;
                os << flushInfos.sizeInfo.renderableCount;
                os << flushInfos.sizeInfo.renderStateCount;
                os << flushInfos.sizeInfo.datalayoutCount;
                os << flushInfos.sizeInfo.datainstanceCount;
                os << flushInfos.sizeInfo.renderGroupCount;
                os << flushInfos.sizeInfo.renderPassCount;
                os << flushInfos.sizeInfo.blitPassCount;
                os << flushInfos.sizeInfo.renderTargetCount;
                os << flushInfos.sizeInfo.renderBufferCount;
                os << flushInfos.sizeInfo.textureSamplerCount;
                os << flushInfos.sizeInfo.streamTextureCount;
                os << flushInfos.sizeInfo.dataSlotCount;
                os << flushInfos.sizeInfo.dataBufferCount;
                os << flushInfos.sizeInfo.animationSystemCount;
                os << flushInfos.sizeInfo.textureBufferCount;
                os << flushInfos.sizeInfo.pickableObjectCount;
                os << flushInfos.sizeInfo.sceneReferenceCount;
            }
            putDataArray(os, flushInfos.resourceChanges.m_resourcesAdded);
            putDataArray(os, flushInfos.resourceChanges.m_resourcesRemoved);
            putDataArray(os, flushInfos.resourceChanges.m_sceneResourceActions);

            os << uint32_t(flushInfos.sceneReferences.size());
            for (const auto& a : flushInfos.sceneReferences)
            {
                os << a.type;
                os << a.consumerScene;
                os << a.consumerId.getValue();
                os << a.providerScene;
                os << a.providerId.getValue();
            }

            os << (static_cast<uint64_t>(std::chrono::time_point_cast<std::chrono::milliseconds>(flushInfos.flushTimeInfo.expirationTimestamp).time_since_epoch().count()));
            os << (static_cast<uint64_t>(std::chrono::time_point_cast<std::chrono::milliseconds>(flushInfos.flushTimeInfo.internalTimestamp).time_since_epoch().count()));
            os << (static_cast<uint32_t>(flushInfos.flushTimeInfo.clock_type));

            os << flushInfos.versionTag.getValue();

            return workingMemory;
        }

        FlushInformation Deserialize(absl::Span<const Byte> flushInfoBlob)
        {
            assert(flushInfoBlob.size() >= FlushInformation::getMinimumSize());
            BinaryInputStream is(flushInfoBlob.data());

            FlushInformation infos;
            is >> infos.containsValidInformation;
            is >> infos.flushCounter;
            uint8_t flushFlags = 0u;
            is >> flushFlags;
            infos.hasSizeInfo = (flushFlags & static_cast<uint8_t>(FlushBits::HasSizeInfo)) != 0;
            const bool hasEffectTimeSync = (flushFlags & static_cast<uint8_t>(FlushBits::HasEffectTimeSync)) != 0;
            if (infos.hasSizeInfo)
            {
                is >> infos.sizeInfo.nodeCount;
                is >> infos.sizeInfo.cameraCount;
                is >> infos.sizeInfo.transformCount;
                is >> infos.sizeInfo.renderableCount;
                is >> infos.sizeInfo.renderStateCount;
                is >> infos.sizeInfo.datalayoutCount;
                is >> infos.sizeInfo.datainstanceCount;
                is >> infos.sizeInfo.renderGroupCount;
                is >> infos.sizeInfo.renderPassCount;
                is >> infos.sizeInfo.blitPassCount;
                is >> infos.sizeInfo.renderTargetCount;
                is >> infos.sizeInfo.renderBufferCount;
                is >> infos.sizeInfo.textureSamplerCount;
                is >> infos.sizeInfo.streamTextureCount;
                is >> infos.sizeInfo.dataSlotCount;
                is >> infos.sizeInfo.dataBufferCount;
                is >> infos.sizeInfo.animationSystemCount;
                is >> infos.sizeInfo.textureBufferCount;
                is >> infos.sizeInfo.pickableObjectCount;
                is >> infos.sizeInfo.sceneReferenceCount;
            }
            getDataArray(is, infos.resourceChanges.m_resourcesAdded);
            getDataArray(is, infos.resourceChanges.m_resourcesRemoved);
            getDataArray(is, infos.resourceChanges.m_sceneResourceActions);
            uint32_t count = 0;
            is >> count;
            infos.sceneReferences.reserve(infos.sceneReferences.size() + count);
            for (uint32_t i = 0u; i < count; ++i)
            {
                infos.sceneReferences.push_back({});
                is >> infos.sceneReferences.back().type;
                is >> infos.sceneReferences.back().consumerScene;
                is >> infos.sceneReferences.back().consumerId.getReference();
                is >> infos.sceneReferences.back().providerScene;
                is >> infos.sceneReferences.back().providerId.getReference();
            }

            UInt64 tsVal = 0;
            is >> tsVal;
            infos.flushTimeInfo.expirationTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(tsVal));
            is >> tsVal;
            infos.flushTimeInfo.internalTimestamp = FlushTime::Clock::time_point(std::chrono::milliseconds(tsVal));
            infos.flushTimeInfo.isEffectTimeSync = hasEffectTimeSync;
            uint32_t clockTypeAsUInt = 0;
            is >> clockTypeAsUInt;
            infos.flushTimeInfo.clock_type = static_cast<synchronized_clock_type>(clockTypeAsUInt);
            is >> infos.versionTag.getReference();

            return infos;
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
                // TODO(Carsten): We need to set a compression level here, but we simply don't know which one it is.
                // We just set offline for now to avoid any potential recompressing, but there shouldn't be
                // any compressing on renderer side anyway.To implement correctly, we need to break network/file
                // compatibility by serializing the IResource::CompressionLevel instead of EResourceCompressionStatus
                if (header.compressionStatus == EResourceCompressionStatus_Compressed)
                    header.resource->setCompressedResourceData(CompressedResourceBlob(data.size(), data.data()), IResource::CompressionLevel::Offline, header.decompressedSize, hash);
                else
                    header.resource->setResourceData(ResourceBlob(data.size(), data.data()), hash);
            }
            return std::move(header.resource);
        }
    }
}
