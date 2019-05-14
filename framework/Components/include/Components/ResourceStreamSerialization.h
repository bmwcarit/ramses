//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCESTREAMSERIALIZATION_H
#define RAMSES_RESOURCESTREAMSERIALIZATION_H

#include "PlatformAbstraction/PlatformTypeInfo.h"
#include "Collections/Pair.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Components/ManagedResource.h"
#include "Collections/ArrayView.h"

namespace ramses_internal
{
    class ResourceStreamSerializer
    {
    public:
        using PreparePacketFun = std::function<std::pair<Byte*, UInt32>(UInt32)>;
        using FinishedPacketFun = std::function<void(UInt32)>;

        void serialize(const PreparePacketFun& preparePacketFun, const FinishedPacketFun& finishedPacketFun, const ManagedResourceVector& resources);

        static const UInt32 FrameSize = 2 * sizeof(UInt32) + sizeof(ResourceContentHash);

    private:
        struct SerializationInfo
        {
            ResourceContentHash hash;
            HeapArray<Byte> metadata;
            UInt32 blobSize;
            Byte* blobData;
        };

        struct PacketInfo
        {
            Byte* data;
            UInt32 writeIdx;
            UInt32 size;

            Byte* writePos()
            {
                return data + writeIdx;
            }

            UInt32 sizeRemaining() const
            {
                return size - writeIdx;
            }
        };

        PreparePacketFun m_preparePacketFun;
        FinishedPacketFun m_finishedPacketFun;

        UInt32 m_packetNum;
        UInt32 m_remainingSize;
        PacketInfo m_currentPacket;

        static HeapArray<Byte> GetSerializedMetadata(const IResource& resource);
        static void WriteResourceFrame(PacketInfo& pi, const SerializationInfo& info);
        static UInt32 WriteBlobPartial(PacketInfo& pi, const Byte* in, UInt32 inSize);

        PacketInfo initializeNewPacket(UInt32 packetNum, UInt32 size);
        void writeBlobFull(const Byte* data, UInt32 size);
    };

    class ResourceStreamDeserializer
    {
    public:
        ResourceStreamDeserializer();

        std::vector<IResource*> processData(const ByteArrayView& data);
        bool processingFinished() const;
        bool processingFailed() const;

    private:
        enum class EState {
            None = 0,
            Metadata,
            Blob,
            Fail
        };

        UInt32 m_nextPacketNum;
        EState m_state;

        ResourceContentHash m_currentHash;
        std::vector<Byte> m_currentMetadata;
        UInt32 m_metadataRead;
        std::unique_ptr<IResource> m_currentResource;

        UInt32 m_currentBlobSize;
        UInt32 m_blobRead;
    };
}

#endif
