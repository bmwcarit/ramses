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

        MOCK_METHOD(void, sceneStateChanged, (sceneId_t, RendererSceneState), (override));
        MOCK_METHOD(void, offscreenBufferLinked, (displayBufferId_t, sceneId_t, dataConsumerId_t, bool), (override));
        MOCK_METHOD(void, dataLinked, (sceneId_t, dataProviderId_t, sceneId_t, dataConsumerId_t, bool), (override));
        MOCK_METHOD(void, objectsPicked, (sceneId_t, const pickableObjectId_t*, uint32_t), (override));
        MOCK_METHOD(void, dataUnlinked, (sceneId_t, dataConsumerId_t, bool), (override));
        MOCK_METHOD(void, dataProviderCreated, (sceneId_t, dataProviderId_t), (override));
        MOCK_METHOD(void, dataProviderDestroyed, (sceneId_t, dataProviderId_t), (override));
        MOCK_METHOD(void, dataConsumerCreated, (sceneId_t, dataConsumerId_t), (override));
        MOCK_METHOD(void, dataConsumerDestroyed, (sceneId_t, dataConsumerId_t), (override));
        MOCK_METHOD(void, sceneFlushed, (sceneId_t, sceneVersionTag_t), (override));
        MOCK_METHOD(void, sceneExpirationMonitoringEnabled, (sceneId_t), (override));
        MOCK_METHOD(void, sceneExpirationMonitoringDisabled, (sceneId_t), (override));
        MOCK_METHOD(void, sceneExpired, (sceneId_t), (override));
        MOCK_METHOD(void, sceneRecoveredFromExpiration, (sceneId_t), (override));
        MOCK_METHOD(void, streamAvailabilityChanged, (waylandIviSurfaceId_t, bool), (override));
    };
}

#endif
