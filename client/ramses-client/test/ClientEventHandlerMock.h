//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTEVENTHANDLERMOCK_H
#define RAMSES_CLIENTEVENTHANDLERMOCK_H

#include "gmock/gmock.h"
#include "ramses-client-api/IClientEventHandler.h"
#include "ramses-client-api/SceneReference.h"

namespace ramses
{
    class ClientEventHandlerMock : public IClientEventHandler
    {
    public:
        ClientEventHandlerMock();
        virtual ~ClientEventHandlerMock();

        MOCK_METHOD(void, resourceFileLoadFailed, (const char* filename), (override));
        MOCK_METHOD(void, resourceFileLoadSucceeded, (const char* filename), (override));
        MOCK_METHOD(void, sceneFileLoadFailed, (const char* filename), (override));
        MOCK_METHOD(void, sceneFileLoadSucceeded, (const char* filename, Scene* loadedScene), (override));
        MOCK_METHOD(void, sceneReferenceStateChanged, (SceneReference& sceneRef, RendererSceneState state), (override));
        MOCK_METHOD(void, sceneReferenceFlushed, (SceneReference& sceneRef, sceneVersionTag_t versionTag), (override));
        MOCK_METHOD(void, dataLinked, (sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool success), (override));
        MOCK_METHOD(void, dataUnlinked, (sceneId_t consumerScene, dataConsumerId_t consumerId, bool success), (override));
    };
}

#endif
