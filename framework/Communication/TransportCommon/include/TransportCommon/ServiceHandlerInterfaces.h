//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SERVICEHANDLERINTERFACES_H
#define RAMSES_SERVICEHANDLERINTERFACES_H

#include "Transfer/ResourceTypes.h"
#include "Resource/ResourceInfo.h"
#include "SceneAPI/SceneId.h"
#include "Collections/ArrayView.h"
#include "Components/DcsmTypes.h"

namespace ramses_internal
{
    class Guid;
    class SceneActionCollection;

    class IResourceConsumerServiceHandler
    {
    public:
        virtual ~IResourceConsumerServiceHandler() {}

        virtual void handleSendResource(const ByteArrayView& data, const Guid& providerID) = 0;
        virtual void handleResourcesNotAvailable(const ResourceContentHashVector& resources, const Guid& providerID) = 0;
    };

    class IResourceProviderServiceHandler
    {
    public:
        virtual ~IResourceProviderServiceHandler() {}

        virtual void handleRequestResources(const ResourceContentHashVector& resources, UInt32 chunkSize, const Guid& requesterId) = 0;
    };

    class ISceneProviderServiceHandler
    {
    public:
        virtual ~ISceneProviderServiceHandler() {}

        virtual void handleSubscribeScene(const SceneId& sceneId, const Guid& consumerID) = 0;
        virtual void handleUnsubscribeScene(const SceneId& sceneId, const Guid& consumerID) = 0;
    };

    class ISceneRendererServiceHandler
    {
    public:
        virtual ~ISceneRendererServiceHandler() {}

        virtual void handleNewScenesAvailable(const SceneInfoVector& newScenes, const Guid& providerID, EScenePublicationMode mode) = 0;
        virtual void handleScenesBecameUnavailable(const SceneInfoVector& unavailableScenes, const Guid& providerID) = 0;

        virtual void handleSceneNotAvailable(const SceneId& sceneId, const Guid& providerID) = 0;

        virtual void handleInitializeScene(const SceneInfo& sceneInfo, const Guid& providerID) = 0;
        virtual void handleSceneActionList(const SceneId& sceneId, SceneActionCollection&& actions, const uint64_t& counter, const Guid& providerID) = 0;
    };

    class IDcsmProviderServiceHandler
    {
    public:
        virtual ~IDcsmProviderServiceHandler() {};
        virtual void handleCanvasSizeChange(ContentID contentID, SizeInfo sizeinfo, AnimationInformation, const Guid& consumerID) = 0;
        virtual void handleContentStateChange(ContentID contentID, EDcsmState status, SizeInfo, AnimationInformation, const Guid& consumerID) = 0;
    };

    class IDcsmConsumerServiceHandler
    {
    public:
        virtual ~IDcsmConsumerServiceHandler() {};
        virtual void handleOfferContent(ContentID contentID, Category, const Guid& providerID) = 0;
        virtual void handleContentReady(ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID) = 0;
        virtual void handleContentFocusRequest(ContentID contentID, const Guid& providerID) = 0;
        virtual void handleRequestStopOfferContent(ContentID contentID, const Guid& providerID) = 0;
        virtual void handleForceStopOfferContent(ContentID contentID, const Guid& providerID) = 0;
    };
}

#endif
