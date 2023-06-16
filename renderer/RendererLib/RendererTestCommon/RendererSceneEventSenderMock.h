//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENEEVENTSENDERMOCK_H
#define RAMSES_RENDERERSCENEEVENTSENDERMOCK_H

#include "RendererFramework/IRendererSceneEventSender.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    class RendererSceneEventSenderMock : public IRendererSceneEventSender
    {
    public:
        ~RendererSceneEventSenderMock() override = default;

        MOCK_METHOD(void, sendSubscribeScene, (SceneId sceneId), (override));
        MOCK_METHOD(void, sendUnsubscribeScene, (SceneId sceneId), (override));

        MOCK_METHOD(void, sendSceneStateChanged, (SceneId masterScene, SceneId referencedScene, RendererSceneState newState), (override));
        MOCK_METHOD(void, sendSceneFlushed, (SceneId masterScene, SceneId referencedScene, SceneVersionTag tag), (override));
        MOCK_METHOD(void, sendDataLinked, (SceneId masterScene, SceneId providerScene, DataSlotId provider, SceneId consumerScene, DataSlotId consumer, bool success), (override));
        MOCK_METHOD(void, sendDataUnlinked, (SceneId masterScene, SceneId consumerScene, DataSlotId consumer, bool success), (override));
    };
}
#endif
