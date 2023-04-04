//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEUPDATESERIALIZERTESTHELPER_H
#define RAMSES_SCENEUPDATESERIALIZERTESTHELPER_H

#include "TransportCommon/ISceneUpdateSerializer.h"
#include "gmock/gmock.h"
#include <cstring>

namespace ramses_internal
{
    class SceneUpdateSerializerMock : public ISceneUpdateSerializer
    {
    public:
        MOCK_METHOD(bool, writeToPackets, (absl::Span<Byte> packetMem, const std::function<bool(size_t)>& writeDoneFunc), (const, override));
    };


    class FakseSceneUpdateSerializer : public ISceneUpdateSerializer
    {
    public:
        FakseSceneUpdateSerializer(const std::vector<std::vector<Byte>>& data_, size_t expectedSize_)
            : data(data_)
            , expectedSize(expectedSize_)
        {
        }

        bool writeToPackets(absl::Span<Byte> packetMem, const std::function<bool(size_t)>& writeDoneFunc) const override
        {
            EXPECT_EQ(expectedSize, packetMem.size());
            for (const auto& d : data)
            {
                assert(packetMem.size() >= d.size());
                assert(d.data());
                std::memcpy(packetMem.data(), d.data(), d.size());
                if (!writeDoneFunc(d.size()))
                    return false;
            }
            return true;
        }

        std::vector<std::vector<Byte>> data;
        const size_t expectedSize;
    };

    inline std::vector<std::vector<Byte>> TestSerializeSceneUpdateToVectorChunked(const ISceneUpdateSerializer& serializer, uint32_t chunkSize = 100000)
    {
        std::vector<std::vector<Byte>> result;
        std::vector<Byte> pkt(chunkSize);
        bool ok = serializer.writeToPackets({pkt.data(), pkt.size()}, [&](size_t size) {
            result.push_back(pkt);
            result.back().resize(size);
            return true;
        });
        assert(ok);
        (void)ok;
        assert(!result.empty());
        return result;
    }
}

#endif
