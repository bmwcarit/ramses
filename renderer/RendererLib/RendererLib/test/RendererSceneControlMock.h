//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENECONTROLMOCK_H
#define RAMSES_RENDERERSCENECONTROLMOCK_H

#include "gmock/gmock.h"
#include "RendererLib/IRendererSceneControl.h"

namespace ramses_internal
{
    class RendererSceneControlMock : public IRendererSceneControl
    {
    public:
        RendererSceneControlMock();
        virtual ~RendererSceneControlMock();

        MOCK_METHOD(void, handleSceneSubscriptionRequest, (SceneId sceneId), (override));
        MOCK_METHOD(void, handleSceneUnsubscriptionRequest, (SceneId sceneId, bool indirect), (override));
        MOCK_METHOD(void, handleSceneMappingRequest, (SceneId sceneId, DisplayHandle handle), (override));
        MOCK_METHOD(void, handleSceneUnmappingRequest, (SceneId sceneId), (override));
        MOCK_METHOD(void, handleSceneShowRequest, (SceneId sceneId), (override));
        MOCK_METHOD(void, handleSceneHideRequest, (SceneId sceneId), (override));
        MOCK_METHOD(bool, handleSceneDisplayBufferAssignmentRequest, (SceneId sceneId, OffscreenBufferHandle buffer, int32_t sceneRenderOrder), (override));
        MOCK_METHOD(void, handleSceneDataLinkRequest, (SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId), (override));
        MOCK_METHOD(void, handleBufferToSceneDataLinkRequest, (OffscreenBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId), (override));
        MOCK_METHOD(void, handleDataUnlinkRequest, (SceneId consumerSceneId, DataSlotId consumerId), (override));
    };
}

#endif
