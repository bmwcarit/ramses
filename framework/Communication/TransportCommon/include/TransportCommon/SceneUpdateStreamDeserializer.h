//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEUPDATESTREAMDESERIALIZER_H
#define RAMSES_SCENEUPDATESTREAMDESERIALIZER_H

#include "Scene/SceneActionCollection.h"
#include "absl/types/span.h"


namespace ramses_internal
{
    class IResource;
    class BinaryInputStream;

    class SceneUpdateStreamDeserializer
    {
    public:
        enum class ResultType
        {
            Failed,
            Empty,
            HasData
        };

        struct Result
        {
            ResultType                              result;
            SceneActionCollection                   actions;
            std::vector<std::unique_ptr<IResource>> resources;
        };

        Result processData(absl::Span<const Byte> data);

    private:
        void continueReadingBlock(BinaryInputStream& is, size_t dataSize);
        bool startReadingNewBlock(BinaryInputStream& is, size_t dataSize);
        Result fail();

        bool finalizeBlock();
        bool handleSceneActionCollection();
        bool handleResource();

        uint32_t m_nextExpectedPacketNum = 1;
        bool m_hasFailed = false;
        uint32_t m_currentBlockSize = 0;
        uint32_t m_blockType;
        std::vector<Byte> m_currentBlock;
        Result m_currentResult;
    };
}

#endif
