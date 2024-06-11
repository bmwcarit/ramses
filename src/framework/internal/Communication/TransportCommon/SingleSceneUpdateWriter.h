//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Components/SceneUpdate.h"
#include "internal/Core/Utils/RawBinaryOutputStream.h"
#include "ramses/framework/EFeatureLevel.h"
#include "absl/types/span.h"

namespace ramses::internal
{
    class StatisticCollectionScene;

    class SingleSceneUpdateWriter
    {
    public:
        SingleSceneUpdateWriter(
            const SceneUpdate& update,
            absl::Span<std::byte> packetMem,
            const std::function<bool(size_t)>& writeDoneFunc,
            StatisticCollectionScene& sceneStatistics,
            EFeatureLevel featureLevel);

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

        bool writeBlock(BlockType type, std::initializer_list<absl::Span<const std::byte>> spans);
        bool writeDataToPackets(absl::Span<const std::byte> data, bool writeContinuous = false);

        const SceneUpdate&                 m_update;
        const absl::Span<std::byte>        m_packetMem;
        const std::function<bool(size_t)>& m_writeDoneFunc;
        RawBinaryOutputStream              m_packetWriter;
        uint32_t                           m_packetNum = 1;
        std::vector<std::byte>             m_temporaryMemToSerializeDescription;  // optimization to avoid allocations
        StatisticCollectionScene&          m_sceneStatistics;
        uint64_t                           m_overallSize{0};
        EFeatureLevel                      m_featureLevel = EFeatureLevel_Latest;
    };
}
