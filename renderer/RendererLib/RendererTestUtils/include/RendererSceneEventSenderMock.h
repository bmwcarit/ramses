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
        virtual ~RendererSceneEventSenderMock() {}

        MOCK_METHOD1(sendSubscribeScene, void(SceneId sceneId));
        MOCK_METHOD1(sendUnsubscribeScene, void(SceneId sceneId));

        MOCK_METHOD3(sendSceneStateChanged, void(SceneId masterScene, SceneId referencedScene, RendererSceneState newState));
        MOCK_METHOD3(sendSceneFlushed, void(SceneId masterScene, SceneId referencedScene, SceneVersionTag tag));
        MOCK_METHOD6(sendDataLinked, void(SceneId masterScene, SceneId providerScene, DataSlotId provider, SceneId consumerScene, DataSlotId consumer, bool success));
        MOCK_METHOD4(sendDataUnlinked, void(SceneId masterScene, SceneId consumerScene, DataSlotId consumer, bool success));
    };
}
#endif
