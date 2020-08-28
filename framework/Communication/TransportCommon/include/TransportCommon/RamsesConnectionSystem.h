//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESCONNECTIONSYSTEM_H
#define RAMSES_RAMSESCONNECTIONSYSTEM_H

#include "TransportCommon/ConnectionSystemBase.h"
#include "TransportCommon/ISomeIPRamsesStack.h"
#include "Components/ManagedResource.h"

namespace ramses_internal
{
    class IResourceProviderServiceHandler;
    class IResourceConsumerServiceHandler;
    class ISceneProviderServiceHandler;
    class ISceneRendererServiceHandler;
    class ISceneUpdateSerializer;

    class RamsesConnectionSystem : public ConnectionSystemBase<ISomeIPRamsesStackCallbacks>
    {
    public:
        static std::unique_ptr<RamsesConnectionSystem> Construct(std::shared_ptr<ISomeIPRamsesStack> stack, UInt32 communicationUserID, const ParticipantIdentifier& namedPid,
                                                                 UInt32 protocolVersion, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection,
                                                                 std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout,
                                                                 std::function<std::chrono::steady_clock::time_point(void)> steadyClockNow);
        ~RamsesConnectionSystem() override;

        bool sendRequestResources(const Guid& to, const ResourceContentHashVector& resources);
        bool sendResourcesNotAvailable(const Guid& to, const ResourceContentHashVector& resources);
        bool sendResources(const Guid& to, const ISceneUpdateSerializer& serializer);
        bool broadcastNewScenesAvailable(const SceneInfoVector& newScenes);
        bool broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes);
        bool sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes);
        bool sendSubscribeScene(const Guid& to, const SceneId& sceneId);
        bool sendUnsubscribeScene(const Guid& to, const SceneId& sceneId);
        bool sendInitializeScene(const Guid& to, const SceneId& sceneId);
        bool sendSceneUpdate(const Guid& to, const SceneId& sceneId, const ISceneUpdateSerializer& serializer);
        bool sendRendererEvent(const Guid& to, const SceneId& sceneId, const std::vector<Byte>& data);  // TODO add renderer event type / more framing?

        void setResourceProviderServiceHandler(IResourceProviderServiceHandler* handler);
        void setResourceConsumerServiceHandler(IResourceConsumerServiceHandler* handler);
        void setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler);
        void setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler);

    private:
        RamsesConnectionSystem(std::shared_ptr<ISomeIPRamsesStack> stack, UInt32 communicationUserID, const ParticipantIdentifier& namedPid,
                               UInt32 protocolVersion, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection,
                               std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout,
                               std::function<std::chrono::steady_clock::time_point(void)> steadyClockNow);

        bool broadcastSceneAvailabilityChange(const char* callerMethod, const SceneInfoVector& scenes, bool available);
        bool sendSceneAvailabilityChange(const char* callerMethod, const Guid& to, const SceneInfoVector& scenes, bool available);
        bool sendSceneSubscriptionChange(const char* callerMethod, const Guid& to, const SceneId& sceneId, bool subscribed);

        // from ISomeIPRamsesCallbacks
        virtual void handleRendererEvent(const SomeIPMsgHeader& header, SceneId sceneId, const std::vector<Byte>& data) override;
        virtual void handleRequestResources(const SomeIPMsgHeader& header, const ResourceContentHashVector& resources) override;
        virtual void handleResourcesNotAvailable(const SomeIPMsgHeader& header, const ResourceContentHashVector& resources) override;
        virtual void handleResourceTransfer(const SomeIPMsgHeader& header, absl::Span<const Byte> resourceData) override;
        virtual void handleSceneUpdate(const SomeIPMsgHeader& header, SceneId sceneId, absl::Span<const Byte> sceneUpdate) override;
        virtual void handleSceneAvailabilityChange(const SomeIPMsgHeader& header, const std::vector<SceneAvailabilityUpdate>& update) override;
        virtual void handleSceneSubscriptionChange(const SomeIPMsgHeader& header, const std::vector<SceneSubscriptionUpdate>& update) override;
        virtual void handleInitializeScene(const SomeIPMsgHeader& header, SceneId sceneId) override;

        std::shared_ptr<ISomeIPRamsesStack> m_stack;
        // limit blob size for resources and sceneactions, smaller packet size allows for
        // more interleaving of data from different scenes/providers
        const uint32_t m_maxSendBlobSize = 300000;
        std::vector<Byte> m_blobSerializationWorkingMemory;

        IResourceProviderServiceHandler* m_resourceProviderServiceHandler = nullptr;
        IResourceConsumerServiceHandler* m_resourceConsumerServiceHandler = nullptr;
        ISceneProviderServiceHandler* m_sceneProviderServiceHandler = nullptr;
        ISceneRendererServiceHandler* m_sceneRendererServiceHandler = nullptr;
    };
}
#endif
