//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/RendererSceneState.h"
#include "internal/SceneGraph/SceneAPI/SceneVersionTag.h"
#include "internal/SceneGraph/SceneAPI/DataSlot.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"

namespace ramses::internal
{
    class IRendererSceneEventSender
    {
    public:
        virtual ~IRendererSceneEventSender() = default;

        virtual void sendSubscribeScene(SceneId sceneId) = 0;
        virtual void sendUnsubscribeScene(SceneId sceneId) = 0;

        virtual void sendSceneStateChanged(SceneId masterScene, SceneId referencedScene, RendererSceneState newState) = 0;
        virtual void sendSceneFlushed(SceneId masterScene, SceneId referencedScene, SceneVersionTag tag) = 0;
        virtual void sendDataLinked(SceneId masterScene, SceneId providerScene, DataSlotId provider, SceneId consumerScene, DataSlotId consumer, bool success) = 0;
        virtual void sendDataUnlinked(SceneId masterScene, SceneId consumerScene, DataSlotId consumer, bool success) = 0;
    };
}

