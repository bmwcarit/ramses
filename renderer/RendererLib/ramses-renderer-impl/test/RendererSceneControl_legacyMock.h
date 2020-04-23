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

        MOCK_METHOD1(subscribeScene, status_t(sceneId_t));
        MOCK_METHOD1(unsubscribeScene, status_t(sceneId_t));
        MOCK_METHOD2(mapScene, status_t(displayId_t, sceneId_t));
        MOCK_METHOD1(unmapScene, status_t(sceneId_t));
        MOCK_METHOD1(showScene, status_t(sceneId_t));
        MOCK_METHOD1(hideScene, status_t(sceneId_t));
        MOCK_METHOD3(assignSceneToDisplayBuffer, status_t(sceneId_t, displayBufferId_t, int32_t));
        MOCK_METHOD6(setDisplayBufferClearColor, status_t(displayId_t, displayBufferId_t, float, float, float, float));
        MOCK_METHOD4(linkData, status_t(sceneId_t, dataProviderId_t, sceneId_t, dataConsumerId_t));
        MOCK_METHOD3(linkOffscreenBufferToSceneData, status_t(displayBufferId_t, sceneId_t, dataConsumerId_t));
        MOCK_METHOD2(unlinkData, status_t(sceneId_t, dataConsumerId_t));
        MOCK_METHOD0(flush, status_t());
        MOCK_METHOD1(dispatchEvents, status_t(IRendererSceneControlEventHandler_legacy&));
    };
}

#endif
