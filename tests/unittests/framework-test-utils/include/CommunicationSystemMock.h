//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <gmock/gmock.h>
#include "internal/Communication/TransportCommon/ICommunicationSystem.h"
#include "internal/Communication/TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "internal/Components/ManagedResource.h"
#include "internal/Communication/TransportCommon/ISceneUpdateSerializer.h"

namespace ramses::internal
{
    class CommunicationSystemMock : public ICommunicationSystem
    {
    public:
        CommunicationSystemMock();
        ~CommunicationSystemMock() override;

        MOCK_METHOD(bool, connectServices, (), (override));
        MOCK_METHOD(bool, disconnectServices, (), (override));

        MOCK_METHOD(IConnectionStatusUpdateNotifier&, getRamsesConnectionStatusUpdateNotifier, (), (override));

        // scene
        MOCK_METHOD(bool, broadcastNewScenesAvailable, (const SceneInfoVector& newScenes, EFeatureLevel featureLevel), (override));
        MOCK_METHOD(bool, broadcastScenesBecameUnavailable, (const SceneInfoVector& unavailableScenes), (override));
        MOCK_METHOD(bool, sendScenesAvailable, (const Guid& to, const SceneInfoVector& availableScenes, EFeatureLevel featureLevel), (override));

        MOCK_METHOD(bool, sendSubscribeScene, (const Guid& to, const SceneId& sceneId), (override));
        MOCK_METHOD(bool, sendUnsubscribeScene, (const Guid& to, const SceneId& sceneId), (override));

        MOCK_METHOD(bool, sendInitializeScene, (const Guid& to, const SceneId& sceneId), (override));
        MOCK_METHOD(bool, sendSceneUpdate, (const Guid& to, const SceneId& sceneId, const ISceneUpdateSerializer& serializer), (override));

        MOCK_METHOD(bool, sendRendererEvent, (const Guid& to, const SceneId& sceneId, const std::vector<std::byte>& data), (override));

        MOCK_METHOD(void, logConnectionInfo, (), (override));
        MOCK_METHOD(void, triggerLogMessageForPeriodicLog, (), (override));

        void setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler) override;
        void setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler) override;
    };
}
