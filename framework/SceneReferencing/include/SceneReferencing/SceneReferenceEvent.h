//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEREFERENCEEVENT_H
#define RAMSES_SCENEREFERENCEEVENT_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/SceneVersionTag.h"
#include "SceneAPI/DataSlot.h"
#include "SceneAPI/RendererSceneState.h"

#include <vector>

namespace ramses_internal
{
    enum class SceneReferenceEventType : uint8_t
    {
        SceneStateChanged,
        SceneFlushed,
        DataLinked,
        DataUnlinked,
    };

    struct SceneReferenceEvent
    {
        explicit SceneReferenceEvent(SceneId sceneid)
            : masterSceneId(sceneid) {}

        SceneId masterSceneId; // intentionally not (de)serialized

        SceneReferenceEventType type = SceneReferenceEventType::SceneStateChanged;
        SceneId referencedScene = SceneId::Invalid();
        SceneId consumerScene = SceneId::Invalid();
        SceneId providerScene = SceneId::Invalid();
        DataSlotId dataConsumer = DataSlotId::Invalid();
        DataSlotId dataProvider = DataSlotId::Invalid();
        RendererSceneState sceneState = RendererSceneState::Unavailable;
        SceneVersionTag tag = SceneVersionTag::Invalid();
        bool status = false;

        constexpr static size_t serializedSize =
            sizeof(SceneReferenceEventType) +
            sizeof(SceneId) +
            sizeof(SceneId) +
            sizeof(SceneId) +
            sizeof(DataSlotId) +
            sizeof(DataSlotId) +
            sizeof(RendererSceneState) +
            sizeof(SceneVersionTag) +
            sizeof(bool);

        void readFromBlob(std::vector<Byte> const& blob);
        void writeToBlob(std::vector<Byte>& blob) const;
    };
}

#endif
