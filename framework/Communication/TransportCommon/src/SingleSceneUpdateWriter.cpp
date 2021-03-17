//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/SingleSceneUpdateWriter.h"
#include "TransportCommon/SceneUpdateSerializationHelper.h"
#include "Utils/StatisticCollection.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    SingleSceneUpdateWriter::SingleSceneUpdateWriter(const SceneUpdate& update, absl::Span<Byte> packetMem, const std::function<bool(size_t)>& writeDoneFunc, StatisticCollectionScene& sceneStatistics)
        : m_update(update)
        , m_packetMem(packetMem)
        , m_writeDoneFunc(writeDoneFunc)
        , m_packetWriter(m_packetMem.data(), static_cast<uint32_t>(m_packetMem.size()))
        , m_sceneStatistics(sceneStatistics)
    {
        /*
          Packet format
          - Packet num : uint32_t
            starts at 1, increment on each packet belonging to same update
          - More packets (yes: hasMorePacketsFlag, no: lastPacketFlag) : uint32_t
            expect start of new update with packet counter 1 when lastPacketFlag
          - data

          Data format (may be split over packets)
          - block type : uint32_t (BlockType)
          - block size : uint32_t
          - block data of given size

          SceneAction data
          - type list length : uin32_t
          - data length : uint32_t
          - type list blob
          - data blob

          Resource data
          - metadata length : uin32_t
          - blob length : uint32_t
          - metadata
          - blob
         */
    }

    bool SingleSceneUpdateWriter::write()
    {
        if (m_packetMem.size() < 50)
        {
            LOG_FATAL_P(CONTEXT_COMMUNICATION, "SingleSceneUpdateWriter::write: Packet size of {} is too small", m_packetMem.size());
            return false;
        }

        initializePacket();

        if (!writeSceneActionCollection())
            return false;

        for (const auto& res : m_update.resources)
        {
            if (!writeResource(*res))
                return false;
        }

        if (m_update.flushInfos.containsValidInformation)
            if (!writeFlushInfos(m_update.flushInfos))
                return false;

        if (m_packetWriter.getBytesWritten() > 0)
        {
            if (!finalizePacket(false))
                return false;
        }

        return true;
    }

    bool SingleSceneUpdateWriter::writeSceneActionCollection()
    {
        m_temporaryMemToSerializeDescription.clear();
        const auto descSpan = SceneActionSerialization::SerializeDescription(m_update.actions, m_temporaryMemToSerializeDescription);
        const auto dataSpan = SceneActionSerialization::SerializeData(m_update.actions);

        Byte header[sizeof(uint32_t)*2];
        RawBinaryOutputStream os(header, sizeof(header));
        os << static_cast<uint32_t>(descSpan.size())
           << static_cast<uint32_t>(dataSpan.size());
        return writeBlock(BlockType::SceneActionCollection, {{os.getData(), os.getSize()}, descSpan, dataSpan});
    }

    bool SingleSceneUpdateWriter::writeResource(const IResource& res)
    {
        m_temporaryMemToSerializeDescription.clear();
        const auto descSpan = ResourceSerialization::SerializeDescription(res, m_temporaryMemToSerializeDescription);
        const auto dataSpan = ResourceSerialization::SerializeData(res);

        Byte header[sizeof(uint32_t)*2];
        RawBinaryOutputStream os(header, sizeof(header));
        os << static_cast<uint32_t>(descSpan.size())
           << static_cast<uint32_t>(dataSpan.size());
        return writeBlock(BlockType::Resource, {{os.getData(), os.getSize()}, descSpan, dataSpan});
    }

    bool SingleSceneUpdateWriter::writeFlushInfos(const FlushInformation& infos)
    {
        m_temporaryMemToSerializeDescription.clear();
        const auto descSpan = FlushInformationSerialization::SerializeInfos(infos, m_temporaryMemToSerializeDescription);

        Byte header[sizeof(uint32_t)];
        RawBinaryOutputStream os(header, sizeof(header));
        os << static_cast<uint32_t>(descSpan.size());
        return writeBlock(BlockType::FlushInfos, { {os.getData(), os.getSize()}, descSpan });
    }

    void SingleSceneUpdateWriter::initializePacket()
    {
        m_packetWriter = RawBinaryOutputStream(m_packetMem.data(), static_cast<uint32_t>(m_packetMem.size()));
        // reserve space for packet header, will be written at end
        m_packetWriter << static_cast<uint32_t>(0)
                       << static_cast<uint32_t>(0);  // placeholders
    }

    bool SingleSceneUpdateWriter::writeBlock(BlockType type, std::initializer_list<absl::Span<const Byte>> spans)
    {
        size_t blockSize = 0;
        for (const auto s : spans)
            blockSize += s.size();
        Byte header[sizeof(uint32_t)*2];
        RawBinaryOutputStream os(header, sizeof(header));
        os << static_cast<uint32_t>(type)
           << static_cast<uint32_t>(blockSize);
        if (!writeDataToPackets({os.getData(), os.getSize()}, true))
            return false;
        for (const auto s : spans)
        {
            if (!writeDataToPackets(s))
                return false;
        }
        return true;
    }

    bool SingleSceneUpdateWriter::writeDataToPackets(absl::Span<const Byte> data, bool writeContinuous)
    {
        while (data.size() > 0)
        {
            const bool startNewPacket = writeContinuous ?
                (m_packetMem.size() < m_packetWriter.getBytesWritten() + data.size()) :
                (m_packetMem.size() == m_packetWriter.getBytesWritten());

            if (startNewPacket)
            {
                // need new packet
                if (!finalizePacket(true))
                    return false;
                initializePacket();
            }

            const size_t capacityRemaining = m_packetMem.size() - m_packetWriter.getBytesWritten();
            const size_t writeBytes = std::min(capacityRemaining, data.size());

            m_packetWriter.write(data.data(), static_cast<uint32_t>(writeBytes));
            data = data.subspan(writeBytes);
        }
        return true;
    }

    bool SingleSceneUpdateWriter::finalizePacket(bool more)
    {
        RawBinaryOutputStream headerWriter(m_packetMem.data(), static_cast<uint32_t>(m_packetMem.size()));
        headerWriter << m_packetNum
                     << static_cast<uint32_t>(more ? hasMorePacketsFlag : lastPacketFlag);

        if (!m_writeDoneFunc(m_packetWriter.getBytesWritten()))
        {
            LOG_ERROR_P(CONTEXT_COMMUNICATION, "SingleSceneUpdateWriter::finalizePacket: Packet write failed (size {})", m_packetWriter.getBytesWritten());
            return false;
        }
        m_sceneStatistics.statSceneUpdatesGeneratedSize.incCounter(static_cast<uint32_t>(m_packetWriter.getBytesWritten()));
        m_sceneStatistics.statSceneUpdatesGeneratedPackets.incCounter(1);

        ++m_packetNum;
        return true;
    }
}
