//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENECONTROLEVENTHANDLERMOCK_H
#define RAMSES_RENDERERSCENECONTROLEVENTHANDLERMOCK_H

#include "gmock/gmock.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"

namespace ramses
{
    class RendererSceneControlEventHandlerMock : public IRendererSceneControlEventHandler
    {
    public:
        RendererSceneControlEventHandlerMock();
        virtual ~RendererSceneControlEventHandlerMock();

        MOCK_METHOD1(scenePublished, void(sceneId_t));
        MOCK_METHOD2(sceneStateChanged, void(sceneId_t, RendererSceneState));
        MOCK_METHOD4(offscreenBufferLinked, void(displayBufferId_t, sceneId_t, dataConsumerId_t, bool));
        MOCK_METHOD5(dataLinked, void(sceneId_t, dataProviderId_t, sceneId_t, dataConsumerId_t, bool));
        MOCK_METHOD3(dataUnlinked, void(sceneId_t, dataConsumerId_t, bool));
        MOCK_METHOD2(sceneFlushed, void(sceneId_t, sceneVersionTag_t));
        MOCK_METHOD1(sceneExpired, void(sceneId_t));
        MOCK_METHOD1(sceneRecoveredFromExpiration, void(sceneId_t));
        MOCK_METHOD2(streamAvailabilityChanged, void(streamSource_t, bool));
    };
}

#endif
