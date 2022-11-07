//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SINGLESCENEUPDATEWRITER_H
#define RAMSES_SINGLESCENEUPDATEWRITER_H

#include "Components/SceneUpdate.h"
#include "Utils/RawBinaryOutputStream.h"
#include "absl/types/span.h"

namespace ramses_internal
{
    class StatisticCollectionScene;

    class SingleSceneUpdateWriter
    {
    public:
        SingleSceneUpdateWriter(const SceneUpdate& update, absl::Span<Byte> packetMem, const std::function<bool(size_t)>& writeDoneFunc, StatisticCollectionScene& sceneStatistics);

        bool write();

        enum class BlockType : uint32_t
        {
            SceneActionCollection = 10,
            Resource              = 11,
            FlushInfos            = 12,
        };

        static constexpr const uint32_t hasMorePacketsFlag = 0xCA;
        static constexpr const uint32_t lastPacketFlag = 0xFE;

    private:
        void initializePacket();
        bool finalizePacket(bool more);

        bool writeSceneActionCollection();
        bool writeResource(const IResource& resource);
        bool writeFlushInfos(const FlushInformation& infos);

        bool writeBlock(BlockType type, std::initializer_list<absl::Span<const Byte>> spans);
        bool writeDataToPackets(absl::Span<const Byte> data, bool writeContinuous = false);

        const SceneUpdate&                 m_update;
        const absl::Span<Byte>             m_packetMem;
        const std::function<bool(size_t)>& m_writeDoneFunc;
        RawBinaryOutputStream              m_packetWriter;
        uint32_t                           m_packetNum = 1;
        std::vector<Byte>                  m_temporaryMemToSerializeDescription;  // optimization to avoid allocations
        StatisticCollectionScene&          m_sceneStatistics;
        uint64_t                           m_overallSize{0};
    };
}

#endif
