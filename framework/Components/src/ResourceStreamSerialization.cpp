//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/ResourceStreamSerialization.h"
#include "Components/ResourceSerializationHelper.h"
#include "Utils/RawBinaryOutputStream.h"
#include "Utils/BinaryInputStream.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    HeapArray<Byte> ResourceStreamSerializer::GetSerializedMetadata(const IResource& resource)
    {
        HeapArray<Byte> metadata(ResourceSerializationHelper::ResourceMetadataSize(resource));
        RawBinaryOutputStream stream(metadata.data(), static_cast<UInt32>(metadata.size()));
        ResourceSerializationHelper::SerializeResourceMetadata(stream, resource);
        return metadata;
    }

    void ResourceStreamSerializer::WriteResourceFrame(PacketInfo& pi, const SerializationInfo& info)
    {
        assert(pi.sizeRemaining() >= FrameSize);
        RawBinaryOutputStream stream(pi.writePos(), pi.sizeRemaining());
        stream << static_cast<UInt32>(info.metadata.size());
        stream << info.blobSize;
        stream << info.hash;
        pi.writeIdx += stream.getBytesWritten();
    }

    UInt32 ResourceStreamSerializer::WriteBlobPartial(PacketInfo& pi, const Byte* in, UInt32 inSize)
    {
        const UInt32 writeSize = std::min(pi.sizeRemaining(), inSize);
        PlatformMemory::Copy(pi.writePos(), in, writeSize);
        pi.writeIdx += writeSize;
        return writeSize;
    }

    ResourceStreamSerializer::PacketInfo ResourceStreamSerializer::initializeNewPacket(UInt32 packetNum, UInt32 size)
    {
        const UInt32 neededSize = size + static_cast<UInt32>(sizeof(UInt32));
        auto p = m_preparePacketFun(neededSize);

        PacketInfo pi = { p.first, 0u, p.second };
        assert(pi.size > FrameSize || pi.size >= neededSize);  // sanity check

        RawBinaryOutputStream stream(pi.writePos(), pi.sizeRemaining());
        stream << packetNum;
        pi.writeIdx += stream.getBytesWritten();

        return pi;
    }

    void ResourceStreamSerializer::writeBlobFull(const Byte* data, UInt32 size)
    {
        UInt32 writeIdx = 0;
        while (writeIdx < size)
        {
            const UInt32 sizeRemaining = size - writeIdx;
            const UInt32 written = WriteBlobPartial(m_currentPacket, data + writeIdx, sizeRemaining);
            writeIdx += written;
            if (sizeRemaining != written)
            {
                m_finishedPacketFun(m_currentPacket.writeIdx);
                m_currentPacket = initializeNewPacket(m_packetNum++, m_remainingSize - writeIdx);
            }
        }
        m_remainingSize -= size;
    }

    void ResourceStreamSerializer::serialize(const PreparePacketFun& preparePacketFun, const FinishedPacketFun& finishedPacketFun, const ManagedResourceVector& managedResources)
    {
        if (managedResources.empty())
        {
            return;
        }

        m_preparePacketFun = preparePacketFun;
        m_finishedPacketFun = finishedPacketFun;

        // prepare SerializationInfo
        std::vector<SerializationInfo> serInfos;
        serInfos.reserve(managedResources.size());
        UInt32 overallSize = 0;

        for (const auto& mResource : managedResources)
        {
            const IResource& resource = *mResource.getResourceObject();
            if (resource.isCompressedAvailable())
            {
                const CompressedSceneResourceData& compressedData = resource.getCompressedResourceData();
                serInfos.push_back({ resource.getHash(), GetSerializedMetadata(resource), compressedData->size(), compressedData->getRawData() });
            }
            else
            {
                const SceneResourceData& data = resource.getResourceData();
                serInfos.push_back({ resource.getHash(), GetSerializedMetadata(resource), data->size(), data->getRawData() });
            }

            overallSize += static_cast<UInt32>(serInfos.back().metadata.size()) + static_cast<UInt32>(serInfos.back().blobSize) + FrameSize;
        }

        m_remainingSize = overallSize;
        m_packetNum = 0;
        m_currentPacket = initializeNewPacket(m_packetNum++, m_remainingSize);

        UInt idx = 0;
        while (idx != serInfos.size())
        {
            const SerializationInfo& info = serInfos[idx];

            if (m_currentPacket.sizeRemaining() >= FrameSize)
            {
                WriteResourceFrame(m_currentPacket, info);
                m_remainingSize -= FrameSize;

                writeBlobFull(info.metadata.data(), static_cast<UInt32>(info.metadata.size()));
                writeBlobFull(info.blobData, info.blobSize);

                ++idx;
            }
            else
            {
                m_finishedPacketFun(m_currentPacket.writeIdx);
                m_currentPacket = initializeNewPacket(m_packetNum++, m_remainingSize);
            }
        }
        assert(m_remainingSize == 0);

        m_finishedPacketFun(m_currentPacket.writeIdx);

        m_preparePacketFun = nullptr;
        m_finishedPacketFun = nullptr;
    }

    ResourceStreamDeserializer::ResourceStreamDeserializer()
        : m_nextPacketNum(0)
        , m_state(EState::None)
    {
    }

    std::vector<IResource*> ResourceStreamDeserializer::processData(const ByteArrayView& data)
    {
        UInt32 packetNum = 0;
        auto it = data.begin();
        {
            BinaryInputStream stream(it.get());
            stream >> packetNum;
            it += sizeof(packetNum);
        }

        if (m_state == EState::Fail)
        {
            LOG_DEBUG(CONTEXT_FRAMEWORK, "ResourceStreamDeserializer::processData: Remaining in EState::Fail by previous error");
            return {};
        }
        else if (packetNum == 0)
        {
            if (m_state != EState::None)
            {
                LOG_ERROR(CONTEXT_FRAMEWORK, "ResourceStreamDeserializer::processData: Going to EState::Fail because unexpected packetNum zero");
                m_state = EState::Fail;
                return {};
            }
            m_nextPacketNum = 1;
        }
        else if (packetNum != m_nextPacketNum)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "ResourceStreamDeserializer::processData: Going to EState::Fail because skipped packetNum (expected "
                << m_nextPacketNum << ", got " << packetNum << ")");
            m_state = EState::Fail;
            return{};
        }
        else
        {
            ++m_nextPacketNum;
        }

        std::vector<IResource*> result;

        while (it != data.end())
        {
            switch (m_state)
            {
            case EState::None:
            {
                UInt32 metadataSize = 0;
                BinaryInputStream stream(it.get());
                stream >> metadataSize;
                stream >> m_currentBlobSize;
                stream >> m_currentHash;
                it += sizeof(metadataSize) + sizeof(m_currentBlobSize) + sizeof(m_currentHash);

                m_currentMetadata.resize(metadataSize);
                m_metadataRead = 0;
                m_blobRead = 0;

                m_state = EState::Metadata;
            }
            break;
            case EState::Metadata:
            {
                const UInt32 metadataMissing = static_cast<UInt32>(m_currentMetadata.size()) - m_metadataRead;
                const UInt32 remainingData = static_cast<UInt32>(data.end() - it);
                const UInt32 dataToCopy = std::min(metadataMissing, remainingData);
                PlatformMemory::Copy(m_currentMetadata.data() + m_metadataRead, it.get(), dataToCopy);
                m_metadataRead += dataToCopy;
                it += dataToCopy;

                if (m_currentMetadata.size() == m_metadataRead)
                {
                    BinaryInputStream stream(m_currentMetadata.data());
                    ResourceSerializationHelper::DeserializedResourceHeader header = ResourceSerializationHelper::ResourceFromMetadataStream(stream);
                    if (header.resource)
                    {
                        m_currentResource.reset(header.resource);

                        if (header.compressionStatus == EResourceCompressionStatus_Compressed)
                        {
                            CompressedSceneResourceData compressedData;
                            compressedData.reset(new CompressedMemoryBlob(header.compressedSize, header.decompressedSize));
                            header.resource->setCompressedResourceData(compressedData, m_currentHash);
                        }
                        else
                        {
                            SceneResourceData uncompressedData;
                            uncompressedData.reset(new MemoryBlob(header.decompressedSize));
                            header.resource->setResourceData(uncompressedData, m_currentHash);
                        }

                        m_currentMetadata.clear();

                        if (m_currentBlobSize > 0)
                        {
                            m_state = EState::Blob;
                        }
                        else
                        {
                            // if there is no blob: finish reading resource and directly go to EState::None
                            result.push_back(m_currentResource.release());
                            m_state = EState::None;
                        }
                    }
                    else
                    {
                        // metadata deserialization went wrong
                        m_state = EState::Fail;
                    }
                }
            }
            break;
            case EState::Blob:
            {
                const UInt32 blobMissing = m_currentBlobSize - m_blobRead;
                const UInt32 remainingData = static_cast<UInt32>(data.end() - it);
                const UInt32 dataToCopy = std::min(blobMissing, remainingData);
                Byte* blobBase = (m_currentResource->isCompressedAvailable() ?
                    m_currentResource->getCompressedResourceData()->getRawData() : m_currentResource->getResourceData()->getRawData());

                PlatformMemory::Copy(blobBase + m_blobRead, it.get(), dataToCopy);
                m_blobRead += dataToCopy;
                it += dataToCopy;

                if (m_blobRead == m_currentBlobSize)
                {
                    result.push_back(m_currentResource.release());
                    m_state = EState::None;
                }
            }
            break;
            case EState::Fail:
                assert(false);
                break;
            }
        }

        return result;
    }

    bool ResourceStreamDeserializer::processingFinished() const
    {
        return m_state == EState::None;
    }

    bool ResourceStreamDeserializer::processingFailed() const
    {
        return m_state == EState::Fail;
    }
}
