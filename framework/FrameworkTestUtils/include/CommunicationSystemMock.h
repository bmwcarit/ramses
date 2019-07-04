//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMUNICATIONSYSTEMMOCK_H
#define RAMSES_COMMUNICATIONSYSTEMMOCK_H

#include <gmock/gmock.h>
#include "framework_common_gmock_header.h"
#include "TransportCommon/ICommunicationSystem.h"
#include "TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "Components/ManagedResource.h"

namespace ramses_internal
{
    class CommunicationSystemMock : public ICommunicationSystem
    {
    public:
        CommunicationSystemMock();
        ~CommunicationSystemMock() override;

        MOCK_METHOD0(connectServices, bool());
        MOCK_METHOD0(disconnectServices, bool());

        MOCK_METHOD0(getRamsesConnectionStatusUpdateNotifier, IConnectionStatusUpdateNotifier&());
        MOCK_METHOD0(getDcsmConnectionStatusUpdateNotifier, IConnectionStatusUpdateNotifier&());

        // resource
        MOCK_METHOD2(sendRequestResources, bool(const Guid& to, const ResourceContentHashVector& resources));
        MOCK_METHOD2(sendResourcesNotAvailable, bool(const Guid& to, const ResourceContentHashVector& resources));
        MOCK_METHOD2(sendResources, bool(const Guid& to, const ManagedResourceVector& resources));

        // scene
        MOCK_METHOD1(broadcastNewScenesAvailable, bool(const SceneInfoVector& newScenes));
        MOCK_METHOD1(broadcastScenesBecameUnavailable, bool(const SceneInfoVector& unavailableScenes));
        MOCK_METHOD2(sendScenesAvailable, bool(const Guid& to, const SceneInfoVector& availableScenes));

        MOCK_METHOD2(sendSubscribeScene, bool(const Guid& to, const SceneId& sceneId));
        MOCK_METHOD2(sendUnsubscribeScene, bool(const Guid& to, const SceneId& sceneId));
        MOCK_METHOD2(sendSceneNotAvailable, bool(const Guid& to, const SceneId& sceneId));

        MOCK_METHOD2(sendInitializeScene, bool(const Guid& to, const SceneInfo& sceneInfo));
        MOCK_METHOD4(sendSceneActionList, uint64_t(const Guid& to, const SceneId& sceneId, const SceneActionCollection& actions, const uint64_t& actionListCounter));

        MOCK_METHOD2(sendDcsmBroadcastOfferContent, bool(ContentID contentID, Category));
        MOCK_METHOD3(sendDcsmOfferContent, bool(const Guid& to, ContentID contentID, Category));
        MOCK_METHOD4(sendDcsmContentReady, bool(const Guid& to, ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor));
        MOCK_METHOD2(sendDcsmContentFocusRequest, bool(const Guid& to, ContentID contentID));
        MOCK_METHOD1(sendDcsmBroadcastRequestStopOfferContent, bool(ContentID contentID));
        MOCK_METHOD1(sendDcsmBroadcastForceStopOfferContent, bool(ContentID contentID));
        MOCK_METHOD4(sendDcsmCanvasSizeChange, bool(const Guid& to, ContentID contentID, SizeInfo sizeinfo, AnimationInformation));
        MOCK_METHOD5(sendDcsmContentStateChange, bool(const Guid& to, ContentID contentID, EDcsmState status, SizeInfo, AnimationInformation));

        MOCK_METHOD0(logConnectionInfo, void());
        MOCK_METHOD0(triggerLogMessageForPeriodicLog, void());

        virtual CommunicationSendDataSizes getSendDataSizes() const override;
        virtual void setSendDataSizes(const CommunicationSendDataSizes& sizes) override;

        void setResourceProviderServiceHandler(IResourceProviderServiceHandler* handler) override;
        void setResourceConsumerServiceHandler(IResourceConsumerServiceHandler* handler) override;
        void setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler) override;
        void setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler) override;
        void setDcsmProviderServiceHandler(IDcsmProviderServiceHandler* handler) override;
        void setDcsmConsumerServiceHandler(IDcsmConsumerServiceHandler* handler) override;

        CommunicationSendDataSizes m_sendDataSizes;
    };
}

#endif
