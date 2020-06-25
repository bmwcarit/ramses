//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENECONTROL_LEGACYMOCK_H
#define RAMSES_RENDERERSCENECONTROL_LEGACYMOCK_H

#include "gmock/gmock.h"
#include "RendererSceneControlImpl_legacy.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler_legacy.h"

namespace ramses
{
    class RendererSceneControl_legacyMock : public IRendererSceneControlImpl_legacy
    {
    public:
        RendererSceneControl_legacyMock();
        virtual ~RendererSceneControl_legacyMock();

        MOCK_METHOD(status_t, subscribeScene, (sceneId_t), (override));
        MOCK_METHOD(status_t, unsubscribeScene, (sceneId_t), (override));
        MOCK_METHOD(status_t, mapScene, (displayId_t, sceneId_t), (override));
        MOCK_METHOD(status_t, unmapScene, (sceneId_t), (override));
        MOCK_METHOD(status_t, showScene, (sceneId_t), (override));
        MOCK_METHOD(status_t, hideScene, (sceneId_t), (override));
        MOCK_METHOD(status_t, assignSceneToDisplayBuffer, (sceneId_t, displayBufferId_t, int32_t), (override));
        MOCK_METHOD(status_t, setDisplayBufferClearColor, (displayId_t, displayBufferId_t, float, float, float, float), (override));
        MOCK_METHOD(status_t, linkData, (sceneId_t, dataProviderId_t, sceneId_t, dataConsumerId_t), (override));
        MOCK_METHOD(status_t, linkOffscreenBufferToSceneData, (displayBufferId_t, sceneId_t, dataConsumerId_t), (override));
        MOCK_METHOD(status_t, unlinkData, (sceneId_t, dataConsumerId_t), (override));
        MOCK_METHOD(status_t, flush, (), (override));
        MOCK_METHOD(status_t, dispatchEvents, (IRendererSceneControlEventHandler_legacy&), (override));
    };
}

#endif
