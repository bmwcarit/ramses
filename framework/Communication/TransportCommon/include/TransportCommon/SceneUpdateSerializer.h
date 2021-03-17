//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEUPDATESERIALIZER_H
#define RAMSES_SCENEUPDATESERIALIZER_H

#include "TransportCommon/ISceneUpdateSerializer.h"

namespace ramses_internal
{
    struct SceneUpdate;
    class StatisticCollectionScene;

    class SceneUpdateSerializer : public ISceneUpdateSerializer
    {
    public:
        explicit SceneUpdateSerializer(const SceneUpdate& update, StatisticCollectionScene& sceneStatistics);
        bool writeToPackets(absl::Span<Byte> packetMem, const std::function<bool(size_t)>& writeDoneFunc) const override;

        const SceneUpdate& getUpdate() const;
        const StatisticCollectionScene& getStatisticCollection() const;
    private:
        const SceneUpdate& m_update;
        StatisticCollectionScene& m_sceneStatistics;
    };
}

#endif
