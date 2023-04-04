//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ICOMMUNICATIONSYSTEM_H
#define RAMSES_ICOMMUNICATIONSYSTEM_H

#include "TransportCommon/ServiceHandlerInterfaces.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/SceneTypes.h"
#include "Components/ManagedResource.h"
#include "Utils/IPeriodicLogSupplier.h"
#include "ramses-framework-api/EFeatureLevel.h"

namespace ramses_internal
{
    class Guid;
    class IConnectionStatusUpdateNotifier;
    class SceneActionCollection;
    class ISceneUpdateSerializer;

    class ICommunicationSystem : public IPeriodicLogSupplier
    {
    public:
        ~ICommunicationSystem() override {}

        // connection management
        virtual bool connectServices() = 0;
        virtual bool disconnectServices() = 0;

        virtual IConnectionStatusUpdateNotifier& getRamsesConnectionStatusUpdateNotifier() = 0;

        // scene
        virtual bool broadcastNewScenesAvailable(const SceneInfoVector& newScenes, ramses::EFeatureLevel featureLevel) = 0;
        virtual bool broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes) = 0;
        virtual bool sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes, ramses::EFeatureLevel featureLevel) = 0;

        virtual bool sendSubscribeScene(const Guid& to, const SceneId& sceneId) = 0;
        virtual bool sendUnsubscribeScene(const Guid& to, const SceneId& sceneId) = 0;

        virtual bool sendInitializeScene(const Guid& to, const SceneId& sceneId) = 0;
        virtual bool sendSceneUpdate(const Guid& to, const SceneId& sceneId, const ISceneUpdateSerializer& serializer) = 0;

        virtual bool sendRendererEvent(const Guid& to, const SceneId& sceneId, const std::vector<Byte>& data) = 0;

        // set service handlers
        virtual void setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler) = 0;
        virtual void setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler) = 0;

        // log triggers
        virtual void logConnectionInfo() = 0;
        void triggerLogMessageForPeriodicLog() override = 0;
    };
}

#endif
