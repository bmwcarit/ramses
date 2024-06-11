//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/Scene/SceneActionCollection.h"
#include "internal/Components/FlushInformation.h"
#include "ramses/framework/EFeatureLevel.h"
#include "absl/types/span.h"

namespace ramses::internal
{
    class IResource;
    class BinaryInputStream;

    class SceneUpdateStreamDeserializer
    {
    public:
        explicit SceneUpdateStreamDeserializer(EFeatureLevel featureLevel);

        enum class ResultType
        {
            Failed,
            Empty,
            HasData
        };

        struct Result
        {
            Result() = default;
            Result(Result&&) = default;
            Result& operator=(Result&&) = default;

            Result(Result const&) = delete;
            Result& operator=(Result const& rhs) = delete;

            ResultType                              result{ResultType::Failed};
            SceneActionCollection                   actions;
            FlushInformation                        flushInfos;
            std::vector<std::unique_ptr<IResource>> resources;
        };

        Result processData(absl::Span<const std::byte> data);

    private:
        void continueReadingBlock(BinaryInputStream& is, size_t dataSize);
        bool startReadingNewBlock(BinaryInputStream& is, size_t dataSize);
        Result fail();

        bool finalizeBlock();
        bool handleSceneActionCollection();
        bool handleResource();
        bool handleFlushInfos();

        uint32_t m_nextExpectedPacketNum = 1;
        bool m_hasFailed = false;
        uint32_t m_currentBlockSize = 0;
        uint32_t m_blockType = 0;
        std::vector<std::byte> m_currentBlock;
        Result m_currentResult;

        EFeatureLevel m_featureLevel = EFeatureLevel_Latest;
    };
}
