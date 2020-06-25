//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SERVICEHANDLERMOCKS_H
#define RAMSES_SERVICEHANDLERMOCKS_H

#include <gmock/gmock.h>
#include "framework_common_gmock_header.h"
#include "TransportCommon/ServiceHandlerInterfaces.h"
#include "SceneReferencing/SceneReferenceEvent.h"
#include "TransportCommon/ServiceHandlerInterfaces.h"
#include "Components/ISceneProviderEventConsumer.h"
#include "Components/CategoryInfo.h"

namespace ramses_internal
{
    class ResourceConsumerServiceHandlerMock : public IResourceConsumerServiceHandler
    {
    public:
        ResourceConsumerServiceHandlerMock();
        virtual ~ResourceConsumerServiceHandlerMock() override;

        MOCK_METHOD(void, handleSendResource, (const absl::Span<const Byte>& data, const Guid& providerID), (override));
        MOCK_METHOD(void, handleResourcesNotAvailable, (const ResourceContentHashVector& resources, const Guid& providerID), (override));
    };

    class ResourceProviderServiceHandlerMock : public IResourceProviderServiceHandler
    {
    public:
        ResourceProviderServiceHandlerMock();
        virtual ~ResourceProviderServiceHandlerMock() override;

        MOCK_METHOD(void, handleRequestResources, (const ResourceContentHashVector& resources, UInt32 chunkSize, const Guid& requesterId), (override));
    };

    class SceneProviderServiceHandlerMock : public ISceneProviderServiceHandler
    {
    public:
        SceneProviderServiceHandlerMock();
        virtual ~SceneProviderServiceHandlerMock() override;

        MOCK_METHOD(void, handleSubscribeScene, (const SceneId& sceneId, const Guid& consumerID), (override));
        MOCK_METHOD(void, handleUnsubscribeScene, (const SceneId& sceneId, const Guid& consumerID), (override));
        MOCK_METHOD(void, handleRendererEvent, (const SceneId& sceneId, std::vector<Byte> data, const Guid& rendererID), (override));
    };

    class SceneRendererServiceHandlerMock : public ISceneRendererServiceHandler
    {
    public:
        SceneRendererServiceHandlerMock();
        virtual ~SceneRendererServiceHandlerMock() override;

        MOCK_METHOD(void, handleNewScenesAvailable, (const SceneInfoVector& newScenes, const Guid& providerID, EScenePublicationMode publicationMode), (override));
        MOCK_METHOD(void, handleScenesBecameUnavailable, (const SceneInfoVector& unavailableScenes, const Guid& providerID), (override));

        MOCK_METHOD(void, handleSceneNotAvailable, (const SceneId& sceneId, const Guid& providerID), (override));

        MOCK_METHOD(void, handleInitializeScene, (const SceneInfo& sceneInfo, const Guid& providerID), (override));
        MOCK_METHOD(void, handleSceneActionList_rvr, (const SceneId& sceneId, const SceneActionCollection& actions, const uint64_t& counter, const Guid& providerID));
        virtual void handleSceneActionList(const SceneId& sceneId, SceneActionCollection&& actions, const uint64_t& counter, const Guid& providerID) override
        {
            handleSceneActionList_rvr(sceneId, actions, counter, providerID);
        }
    };

    class DcsmProviderServiceHandlerMock : public IDcsmProviderServiceHandler
    {
    public:
        DcsmProviderServiceHandlerMock();
        virtual ~DcsmProviderServiceHandlerMock() override;

        MOCK_METHOD(void, handleCanvasSizeChange, (ContentID contentID, const CategoryInfo& categoryInfo, AnimationInformation, const Guid& consumerID), (override));
        MOCK_METHOD(void, handleContentStateChange, (ContentID contentID, EDcsmState status, const CategoryInfo& categoryInfo, AnimationInformation, const Guid& consumerID), (override));
    };

    class DcsmConsumerServiceHandlerMock : public IDcsmConsumerServiceHandler
    {
    public:
        DcsmConsumerServiceHandlerMock();
        virtual ~DcsmConsumerServiceHandlerMock() override;

        MOCK_METHOD(void, handleOfferContent, (ContentID contentID, Category, const Guid& providerID), (override));
        MOCK_METHOD(void, handleContentDescription, (ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID), (override));
        MOCK_METHOD(void, handleContentReady, (ContentID contentID, const Guid& providerID), (override));
        MOCK_METHOD(void, handleContentEnableFocusRequest, (ContentID contentID, int32_t focusRequest, const Guid& providerID), (override));
        MOCK_METHOD(void, handleContentDisableFocusRequest, (ContentID contentID, int32_t focusRequest, const Guid& providerID), (override));
        MOCK_METHOD(void, handleRequestStopOfferContent, (ContentID contentID, const Guid& providerID), (override));
        MOCK_METHOD(void, handleForceStopOfferContent, (ContentID contentID, const Guid& providerID), (override));
        MOCK_METHOD(void, handleUpdateContentMetadata, (ContentID contentID, DcsmMetadata metadata, const Guid& providerID), (override));
    };
}

#endif
