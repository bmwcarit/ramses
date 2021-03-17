//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SOMEIPCONNECTIONSYSTEMMULTIPLEXER_H
#define RAMSES_SOMEIPCONNECTIONSYSTEMMULTIPLEXER_H

#include "Collections/StringOutputStream.h"
#include "TransportCommon/ICommunicationSystem.h"
#include "Common/ParticipantIdentifier.h"
#include "PlatformAbstraction/PlatformLock.h"

namespace ramses_internal
{
    class DcsmConnectionSystem;
    class RamsesConnectionSystem;
    class StatisticCollectionFramework;

    class SomeIPConnectionSystemMultiplexer : public ICommunicationSystem
    {
    public:
        virtual ~SomeIPConnectionSystemMultiplexer() override;

        // connection management
        virtual bool connectServices() override;
        virtual bool disconnectServices() override;

        virtual IConnectionStatusUpdateNotifier& getRamsesConnectionStatusUpdateNotifier() override;
        virtual IConnectionStatusUpdateNotifier& getDcsmConnectionStatusUpdateNotifier() override;

        // scene
        virtual bool broadcastNewScenesAvailable(const SceneInfoVector& newScenes) override;
        virtual bool broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes) override;
        virtual bool sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes) override;

        virtual bool sendSubscribeScene(const Guid& to, const SceneId& sceneId) override;
        virtual bool sendUnsubscribeScene(const Guid& to, const SceneId& sceneId) override;

        virtual bool sendInitializeScene(const Guid& to, const SceneId& sceneId) override;
        virtual bool sendSceneUpdate(const Guid& to, const SceneId& sceneId, const ISceneUpdateSerializer& serializer) override;

        virtual bool sendRendererEvent(const Guid& to, const SceneId& sceneId, const std::vector<Byte>& data) override;

        // dcsm provider -> consumer
        virtual bool sendDcsmBroadcastOfferContent(ContentID contentID, Category, ETechnicalContentType technicalContentType, const std::string& friendlyName) override;
        virtual bool sendDcsmOfferContent(const Guid& to, ContentID contentID, Category, ETechnicalContentType technicalContentType, const std::string& friendlyName) override;
        virtual bool sendDcsmContentDescription(const Guid& to, ContentID contentID, TechnicalContentDescriptor technicalContentDescriptor) override;
        virtual bool sendDcsmContentReady(const Guid& to, ContentID contentID) override;
        virtual bool sendDcsmContentEnableFocusRequest(const Guid& to, ContentID contentID, int32_t focusRequest) override;
        virtual bool sendDcsmContentDisableFocusRequest(const Guid& to, ContentID contentID, int32_t focusRequest) override;
        virtual bool sendDcsmBroadcastRequestStopOfferContent(ContentID contentID) override;
        virtual bool sendDcsmBroadcastForceStopOfferContent(ContentID contentID) override;
        virtual bool sendDcsmUpdateContentMetadata(const Guid& to, ContentID contentID, const DcsmMetadata& metadata) override;

        // dcsm consumer -> provider
        virtual bool sendDcsmCanvasSizeChange(const Guid& to, ContentID contentID, const CategoryInfo& categoryInfo, AnimationInformation) override;
        virtual bool sendDcsmContentStateChange(const Guid& to, ContentID contentID, EDcsmState status, const CategoryInfo&, AnimationInformation) override;


        // set service handlers
        virtual void setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler) override;
        virtual void setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler) override;

        virtual void setDcsmProviderServiceHandler(IDcsmProviderServiceHandler* handler) override;
        virtual void setDcsmConsumerServiceHandler(IDcsmConsumerServiceHandler* handler) override;


        // log triggers
        void writeStateForLog(StringOutputStream& sos);
        virtual void logConnectionInfo() override;
        virtual void triggerLogMessageForPeriodicLog() override;

    protected:
        SomeIPConnectionSystemMultiplexer(uint32_t someipCommunicationUserID,
                                          const ParticipantIdentifier& participantIdentifier,
                                          PlatformLock& frameworkLock,
                                          std::unique_ptr<DcsmConnectionSystem> dcsmConnectionSystem,
                                          std::unique_ptr<RamsesConnectionSystem> ramsesConnectionSystem);

        virtual bool doPrepareConnect() { return true; }
        virtual void doFinalizeConnect() {}
        virtual void doFinalizeDisconnect() {}

        const uint32_t m_someipCommunicationUserID;
        const ParticipantIdentifier m_participantIdentifier;
        PlatformLock& m_frameworkLock;
        bool m_isConnected = false;

        std::unique_ptr<DcsmConnectionSystem> m_dcsmConnectionSystem;
        std::unique_ptr<RamsesConnectionSystem> m_ramsesConnectionSystem;
    };
}

#endif
