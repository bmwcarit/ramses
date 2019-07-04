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

namespace ramses_internal
{
    class ResourceConsumerServiceHandlerMock : public IResourceConsumerServiceHandler
    {
    public:
        ResourceConsumerServiceHandlerMock();
        ~ResourceConsumerServiceHandlerMock() override;

        MOCK_METHOD2(handleSendResource, void(const ByteArrayView& data, const Guid& providerID));
        MOCK_METHOD2(handleResourcesNotAvailable, void(const ResourceContentHashVector& resources, const Guid& providerID));
    };

    class ResourceProviderServiceHandlerMock : public IResourceProviderServiceHandler
    {
    public:
        ResourceProviderServiceHandlerMock();
        ~ResourceProviderServiceHandlerMock() override;

        MOCK_METHOD3(handleRequestResources, void(const ResourceContentHashVector& resources, UInt32 chunkSize, const Guid& requesterId));
    };

    class SceneProviderServiceHandlerMock : public ISceneProviderServiceHandler
    {
    public:
        SceneProviderServiceHandlerMock();
        ~SceneProviderServiceHandlerMock() override;

        MOCK_METHOD2(handleSubscribeScene, void(const SceneId& sceneId, const Guid& consumerID));
        MOCK_METHOD2(handleUnsubscribeScene, void(const SceneId& sceneId, const Guid& consumerID));
    };

    class SceneRendererServiceHandlerMock : public ISceneRendererServiceHandler
    {
    public:
        SceneRendererServiceHandlerMock();
        ~SceneRendererServiceHandlerMock() override;

        MOCK_METHOD3(handleNewScenesAvailable, void(const SceneInfoVector& newScenes, const Guid& providerID, EScenePublicationMode publicationMode));
        MOCK_METHOD2(handleScenesBecameUnavailable, void(const SceneInfoVector& unavailableScenes, const Guid& providerID));

        MOCK_METHOD2(handleSceneNotAvailable, void(const SceneId& sceneId, const Guid& providerID));

        MOCK_METHOD2(handleInitializeScene, void(const SceneInfo& sceneInfo, const Guid& providerID));
        MOCK_METHOD4(handleSceneActionList_rvr, void(const SceneId& sceneId, const SceneActionCollection& actions, const uint64_t& counter, const Guid& providerID));
        virtual void handleSceneActionList(const SceneId& sceneId, SceneActionCollection&& actions, const uint64_t& counter, const Guid& providerID) override
        {
            handleSceneActionList_rvr(sceneId, actions, counter, providerID);
        }
    };

    class DcsmProviderServiceHandlerMock : public IDcsmProviderServiceHandler
    {
    public:
        DcsmProviderServiceHandlerMock();
        ~DcsmProviderServiceHandlerMock() override;

        MOCK_METHOD4(handleCanvasSizeChange, void(ContentID contentID, SizeInfo sizeinfo, AnimationInformation, const Guid& consumerID));
        MOCK_METHOD5(handleContentStateChange, void(ContentID contentID, EDcsmState status, SizeInfo, AnimationInformation, const Guid& consumerID));
    };

    class DcsmConsumerServiceHandlerMock : public IDcsmConsumerServiceHandler
    {
    public:
        DcsmConsumerServiceHandlerMock();
        ~DcsmConsumerServiceHandlerMock() override;

        MOCK_METHOD3(handleOfferContent, void(ContentID contentID, Category, const Guid& providerID));
        MOCK_METHOD4(handleContentReady, void(ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID));
        MOCK_METHOD2(handleContentFocusRequest, void(ContentID contentID, const Guid& providerID));
        MOCK_METHOD2(handleRequestStopOfferContent, void(ContentID contentID, const Guid& providerID));
        MOCK_METHOD2(handleForceStopOfferContent, void(ContentID contentID, const Guid& providerID));
    };
}

#endif
