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

        MOCK_METHOD1(handleSceneSubscriptionRequest, void(SceneId sceneId));
        MOCK_METHOD2(handleSceneUnsubscriptionRequest, void(SceneId sceneId, bool indirect));
        MOCK_METHOD2(handleSceneMappingRequest, void(SceneId sceneId, DisplayHandle handle));
        MOCK_METHOD1(handleSceneUnmappingRequest, void(SceneId sceneId));
        MOCK_METHOD1(handleSceneShowRequest, void(SceneId sceneId));
        MOCK_METHOD1(handleSceneHideRequest, void(SceneId sceneId));
        MOCK_METHOD3(handleSceneDisplayBufferAssignmentRequest, bool(SceneId sceneId, OffscreenBufferHandle buffer, int32_t sceneRenderOrder));
        MOCK_METHOD4(handleSceneDataLinkRequest, void(SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId));
        MOCK_METHOD3(handleBufferToSceneDataLinkRequest, void(OffscreenBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId));
        MOCK_METHOD2(handleDataUnlinkRequest, void(SceneId consumerSceneId, DataSlotId consumerId));
    };
}

#endif
