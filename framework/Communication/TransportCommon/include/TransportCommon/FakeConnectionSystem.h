//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FAKECONNECTIONSYSTEM_H
#define RAMSES_FAKECONNECTIONSYSTEM_H

#include "TransportCommon/FakeConnectionStatusUpdateNotifier.h"
#include "ICommunicationSystem.h"
#include "Transfer/ResourceTypes.h"
#include "Components/ManagedResource.h"

namespace ramses_internal
{
    class SceneActionCollection;

    class FakeConnectionSystem : public ICommunicationSystem
    {
    public:
        virtual bool connectServices() override
        {
            return true;
        }

        virtual bool disconnectServices() override
        {
            return true;
        }

        virtual IConnectionStatusUpdateNotifier& getConnectionStatusUpdateNotifier() override
        {
            static FakeConnectionStatusUpdateNotifier fake;
            return fake;
        }

        virtual bool sendRequestResources(const Guid& /*to*/, const ResourceContentHashVector& /*resources*/) override
        {
            return true;
        }

        virtual bool sendResourcesNotAvailable(const Guid& /*to*/, const ResourceContentHashVector& /*resources*/) override
        {
            return true;
        }

        virtual bool sendResources(const Guid& /*to*/, const ManagedResourceVector& /*resource*/) override
        {
            return true;
        }

        virtual bool broadcastNewScenesAvailable(const SceneInfoVector& /*newScenes*/) override
        {
            return true;
        }

        virtual bool broadcastScenesBecameUnavailable(const SceneInfoVector& /*unavailableScenes*/) override
        {
            return true;
        }

        virtual bool sendScenesAvailable(const Guid& /*to*/, const SceneInfoVector& /*availableScenes*/) override
        {
            return true;
        }

        virtual bool sendSubscribeScene(const Guid& /*to*/, const SceneId& /*sceneId*/) override
        {
            return true;
        }

        virtual bool sendUnsubscribeScene(const Guid& /*to*/, const SceneId& /*sceneId*/) override
        {
            return true;
        }

        virtual bool sendSceneNotAvailable(const Guid& /*to*/, const SceneId& /*sceneId*/) override
        {
            return true;
        }

        virtual bool sendInitializeScene(const Guid& /*to*/, const SceneInfo& /*sceneInfo*/) override
        {
            return true;
        }

        virtual uint64_t sendSceneActionList(const Guid& /*to*/, const SceneId& /*sceneId*/, const SceneActionCollection& /*actions*/, const uint64_t& ) override
        {
            return 0u;
        }

        virtual void setResourceProviderServiceHandler(IResourceProviderServiceHandler* /*handler*/) override
        {
        }

        virtual void setResourceConsumerServiceHandler(IResourceConsumerServiceHandler* /*handler*/) override
        {
        }

        virtual void setSceneProviderServiceHandler(ISceneProviderServiceHandler* /*handler*/) override
        {
        }

        virtual void setSceneRendererServiceHandler(ISceneRendererServiceHandler* /*handler*/) override
        {
        }

        virtual CommunicationSendDataSizes getSendDataSizes() const override
        {
            return CommunicationSendDataSizes{ std::numeric_limits<UInt32>::max() , std::numeric_limits<UInt32>::max() , std::numeric_limits<UInt32>::max(),
                std::numeric_limits<UInt32>::max(), std::numeric_limits<UInt32>::max(), std::numeric_limits<UInt32>::max() };
        }

        virtual void setSendDataSizes(const CommunicationSendDataSizes& /*sizes*/) override
        {
        }

        virtual void logConnectionInfo() const override
        {
        }

        virtual void triggerLogMessageForPeriodicLog() const override
        {
        }
    };
}

#endif
