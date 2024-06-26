//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Communication/TransportCommon/FakeConnectionStatusUpdateNotifier.h"
#include "ICommunicationSystem.h"
#include "internal/Components/ManagedResource.h"

namespace ramses::internal
{
    class SceneActionCollection;

    class FakeConnectionSystem : public ICommunicationSystem
    {
    public:
        bool connectServices() override
        {
            return true;
        }

        bool disconnectServices() override
        {
            return true;
        }

        IConnectionStatusUpdateNotifier& getRamsesConnectionStatusUpdateNotifier() override
        {
            static FakeConnectionStatusUpdateNotifier fake;
            return fake;
        }

        bool broadcastNewScenesAvailable(const SceneInfoVector& /*newScenes*/, EFeatureLevel /*unused*/) override
        {
            return true;
        }

        bool broadcastScenesBecameUnavailable(const SceneInfoVector& /*unavailableScenes*/) override
        {
            return true;
        }

        bool sendScenesAvailable(const Guid& /*to*/, const SceneInfoVector& /*availableScenes*/, EFeatureLevel /*unused*/) override
        {
            return true;
        }

        bool sendSubscribeScene(const Guid& /*to*/, const SceneId& /*sceneId*/) override
        {
            return true;
        }

        bool sendUnsubscribeScene(const Guid& /*to*/, const SceneId& /*sceneId*/) override
        {
            return true;
        }

        bool sendInitializeScene(const Guid& /*to*/, const SceneId& /*sceneId*/) override
        {
            return true;
        }

        bool sendSceneUpdate(const Guid& /*to*/, const SceneId& /*sceneId*/, const ISceneUpdateSerializer& /*serializer*/) override
        {
            return true;
        }

        bool sendRendererEvent(const Guid& /*to*/, const SceneId& /*sceneId*/, const std::vector<std::byte>& /*data*/) override
        {
            return true;
        }

        void setSceneProviderServiceHandler(ISceneProviderServiceHandler* /*handler*/) override
        {
        }

        void setSceneRendererServiceHandler(ISceneRendererServiceHandler* /*handler*/) override
        {
        }

        void logConnectionInfo() override
        {
        }

        void triggerLogMessageForPeriodicLog() override
        {
        }
    };
}
