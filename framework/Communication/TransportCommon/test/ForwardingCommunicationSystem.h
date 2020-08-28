//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FORWARDINGCOMMUNICATIONSYSTEM_H
#define RAMSES_FORWARDINGCOMMUNICATIONSYSTEM_H

#include "TransportCommon/ICommunicationSystem.h"
#include "TransportCommon/ConnectionStatusUpdateNotifier.h"
#include "MockConnectionStatusUpdateNotifier.h"

namespace ramses_internal
{
    class ForwardingCommunicationSystem : public ICommunicationSystem
    {
    public:
        explicit ForwardingCommunicationSystem(const Guid& id);
        virtual ~ForwardingCommunicationSystem() override;

        void setForwardingTarget(ForwardingCommunicationSystem* target);

        bool connectServices() override;
        bool disconnectServices() override;

        IConnectionStatusUpdateNotifier& getRamsesConnectionStatusUpdateNotifier() override;
        IConnectionStatusUpdateNotifier& getDcsmConnectionStatusUpdateNotifier() override;

        // resource
        virtual bool sendRequestResources(const Guid& to, const ResourceContentHashVector& resources) override;
        virtual bool sendResourcesNotAvailable(const Guid& to, const ResourceContentHashVector& resources) override;
        virtual bool sendResources(const Guid& to, const ISceneUpdateSerializer& serializer) override;

        // scene
        virtual bool broadcastNewScenesAvailable(const SceneInfoVector& newScenes) override;
        virtual bool broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes) override;
        virtual bool sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes) override;

        virtual bool sendSubscribeScene(const Guid& to, const SceneId& sceneId) override;
        virtual bool sendUnsubscribeScene(const Guid& to, const SceneId& sceneId) override;

        virtual bool sendInitializeScene(const Guid& to, const SceneId& sceneId) override;
        virtual bool sendSceneUpdate(const Guid& to, const SceneId& sceneId, const ISceneUpdateSerializer& serializer) override;


        virtual bool sendRendererEvent(const Guid& to, const SceneId& sceneId, const std::vector<Byte>& data) override;

        // dcsm client -> renderer
        virtual bool sendDcsmBroadcastOfferContent(ContentID contentID, Category, const std::string&) override;
        virtual bool sendDcsmOfferContent(const Guid& to, ContentID contentID, Category, const std::string&) override;
        virtual bool sendDcsmContentDescription(const Guid& to, ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor) override;
        virtual bool sendDcsmContentReady(const Guid& to, ContentID contentID) override;
        virtual bool sendDcsmContentEnableFocusRequest(const Guid& to, ContentID contentID, int32_t focusRequest) override;
        virtual bool sendDcsmContentDisableFocusRequest(const Guid& to, ContentID contentID, int32_t focusRequest) override;
        virtual bool sendDcsmBroadcastRequestStopOfferContent(ContentID contentID) override;
        virtual bool sendDcsmBroadcastForceStopOfferContent(ContentID contentID) override;
        virtual bool sendDcsmUpdateContentMetadata(const Guid& to, ContentID contentID, const DcsmMetadata& metadata) override;

        // dcsm renderer -> client
        virtual bool sendDcsmCanvasSizeChange(const Guid& to, ContentID contentID, const CategoryInfo& sizeinfo, AnimationInformation) override;
        virtual bool sendDcsmContentStateChange(const Guid& to, ContentID contentID, EDcsmState status, const CategoryInfo&, AnimationInformation) override;

        void setResourceProviderServiceHandler(IResourceProviderServiceHandler* handler) override;
        void setResourceConsumerServiceHandler(IResourceConsumerServiceHandler* handler) override;
        void setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler) override;
        void setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler) override;
        void setDcsmProviderServiceHandler(IDcsmProviderServiceHandler* handler) override;
        void setDcsmConsumerServiceHandler(IDcsmConsumerServiceHandler* handler) override;

        virtual void logConnectionInfo() override;
        virtual void triggerLogMessageForPeriodicLog() override;

    private:
        Guid m_id;
        ForwardingCommunicationSystem* m_targetCommunicationSystem;
        testing::NiceMock<MockConnectionStatusUpdateNotifier> m_ramsesConnectionStatusUpdateNotifier;
        testing::NiceMock<MockConnectionStatusUpdateNotifier> m_dcsmConnectionStatusUpdateNotifier;

        IResourceConsumerServiceHandler* m_resourceConsumerHandler;
        IResourceProviderServiceHandler* m_resourceProviderHandler;
        ISceneProviderServiceHandler* m_sceneProviderHandler;
        ISceneRendererServiceHandler* m_sceneRendererHandler;
        IDcsmProviderServiceHandler* m_dcsmProviderHandler;
        IDcsmConsumerServiceHandler* m_dcsmConsumerHandler;
    };
}

#endif
