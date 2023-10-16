//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/SceneVersionTag.h"
#include "internal/SceneGraph/SceneAPI/DataSlot.h"
#include "internal/SceneGraph/SceneAPI/RendererSceneState.h"

#include <vector>
#include "internal/Components/ERendererToClientEventType.h"

namespace ramses::internal
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

        static constexpr size_t serializedSize =
            sizeof(ERendererToClientEventType) +
            sizeof(SceneReferenceEventType) +
            sizeof(SceneId) +
            sizeof(SceneId) +
            sizeof(SceneId) +
            sizeof(DataSlotId) +
            sizeof(DataSlotId) +
            sizeof(RendererSceneState) +
            sizeof(SceneVersionTag) +
            sizeof(bool);

        void readFromBlob(std::vector<std::byte> const& blob);
        void writeToBlob(std::vector<std::byte>& blob) const;
    };
}
