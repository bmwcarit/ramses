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
#include "Components/CategoryInfo.h"

namespace ramses_internal
{
    class CommunicationSystemMock : public ICommunicationSystem
    {
    public:
        CommunicationSystemMock();
        ~CommunicationSystemMock() override;

        MOCK_METHOD(bool, connectServices, (), (override));
        MOCK_METHOD(bool, disconnectServices, (), (override));

        MOCK_METHOD(IConnectionStatusUpdateNotifier&, getRamsesConnectionStatusUpdateNotifier, (), (override));
        MOCK_METHOD(IConnectionStatusUpdateNotifier&, getDcsmConnectionStatusUpdateNotifier, (), (override));

        // resource
        MOCK_METHOD(bool, sendRequestResources, (const Guid& to, const ResourceContentHashVector& resources), (override));
        MOCK_METHOD(bool, sendResourcesNotAvailable, (const Guid& to, const ResourceContentHashVector& resources), (override));
        MOCK_METHOD(bool, sendResources, (const Guid& to, const ManagedResourceVector& resources), (override));

        // scene
        MOCK_METHOD(bool, broadcastNewScenesAvailable, (const SceneInfoVector& newScenes), (override));
        MOCK_METHOD(bool, broadcastScenesBecameUnavailable, (const SceneInfoVector& unavailableScenes), (override));
        MOCK_METHOD(bool, sendScenesAvailable, (const Guid& to, const SceneInfoVector& availableScenes), (override));

        MOCK_METHOD(bool, sendSubscribeScene, (const Guid& to, const SceneId& sceneId), (override));
        MOCK_METHOD(bool, sendUnsubscribeScene, (const Guid& to, const SceneId& sceneId), (override));

        MOCK_METHOD(bool, sendSceneNotAvailable, (const Guid& to, const SceneId& sceneId), (override));
        MOCK_METHOD(bool, sendInitializeScene, (const Guid& to, const SceneInfo& sceneInfo), (override));
        MOCK_METHOD(uint64_t, sendSceneActionList, (const Guid& to, const SceneId& sceneId, const SceneActionCollection& actions, const uint64_t& actionListCounter), (override));

        MOCK_METHOD(bool, sendRendererEvent, (const Guid& to, const SceneId& sceneId, const std::vector<Byte>& data), (override));

        MOCK_METHOD(bool, sendDcsmBroadcastOfferContent, (ContentID contentID, Category), (override));
        MOCK_METHOD(bool, sendDcsmOfferContent, (const Guid& to, ContentID contentID, Category), (override));
        MOCK_METHOD(bool, sendDcsmContentDescription, (const Guid& to, ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor), (override));
        MOCK_METHOD(bool, sendDcsmContentReady, (const Guid& to, ContentID contentID), (override));
        MOCK_METHOD(bool, sendDcsmContentEnableFocusRequest, (const Guid& to, ContentID contentID, int32_t focusRequest), (override));
        MOCK_METHOD(bool, sendDcsmContentDisableFocusRequest, (const Guid& to, ContentID contentID, int32_t focusRequest), (override));
        MOCK_METHOD(bool, sendDcsmBroadcastRequestStopOfferContent, (ContentID contentID), (override));
        MOCK_METHOD(bool, sendDcsmBroadcastForceStopOfferContent, (ContentID contentID), (override));
        MOCK_METHOD(bool, sendDcsmUpdateContentMetadata, (const Guid& to, ContentID contentID, const DcsmMetadata& metadata), (override));
        MOCK_METHOD(bool, sendDcsmCanvasSizeChange, (const Guid& to, ContentID contentID, const CategoryInfo& categoryInfo, AnimationInformation), (override));
        MOCK_METHOD(bool, sendDcsmContentStateChange, (const Guid& to, ContentID contentID, EDcsmState status, const CategoryInfo&, AnimationInformation), (override));

        MOCK_METHOD(void, logConnectionInfo, (), (override));
        MOCK_METHOD(void, triggerLogMessageForPeriodicLog, (), (override));

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
