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
#include "Transfer/ResourceTypes.h"
#include "Components/ManagedResource.h"
#include "Utils/IPeriodicLogSupplier.h"
#include "Components/DcsmTypes.h"

namespace ramses_internal
{
    class Guid;
    class IConnectionStatusUpdateNotifier;
    class SceneActionCollection;

    struct CommunicationSendDataSizes
    {
        UInt32 sceneActionNumber;
        UInt32 sceneActionDataArray;
        UInt32 resourceDataArray;
        UInt32 resourceInfoNumber;
        UInt32 resourceMetaDataNumber;
        UInt32 sceneInfoNumber;
    };

    class ICommunicationSystem : public IPeriodicLogSupplier
    {
    public:
        virtual ~ICommunicationSystem() override {}

        // connection management
        virtual bool connectServices() = 0;
        virtual bool disconnectServices() = 0;

        virtual IConnectionStatusUpdateNotifier& getRamsesConnectionStatusUpdateNotifier() = 0;
        virtual IConnectionStatusUpdateNotifier& getDcsmConnectionStatusUpdateNotifier() = 0;


        // resource
        virtual bool sendRequestResources(const Guid& to, const ResourceContentHashVector& resources) = 0;
        virtual bool sendResourcesNotAvailable(const Guid& to, const ResourceContentHashVector& resources) = 0;
        virtual bool sendResources(const Guid& to, const ManagedResourceVector& resources) = 0;

        // scene
        virtual bool broadcastNewScenesAvailable(const SceneInfoVector& newScenes) = 0;
        virtual bool broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes) = 0;
        virtual bool sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes) = 0;

        virtual bool sendSubscribeScene(const Guid& to, const SceneId& sceneId) = 0;
        virtual bool sendUnsubscribeScene(const Guid& to, const SceneId& sceneId) = 0;
        virtual bool sendSceneNotAvailable(const Guid& to, const SceneId& sceneId) = 0;

        virtual bool sendInitializeScene(const Guid& to, const SceneInfo& sceneInfo) = 0;
        virtual uint64_t sendSceneActionList(const Guid& to, const SceneId& sceneId, const SceneActionCollection& actions, const uint64_t& actionListCounter) = 0;

        // dcsm provider -> consumer
        virtual bool sendDcsmBroadcastOfferContent(ContentID contentID, Category) = 0;
        virtual bool sendDcsmOfferContent(const Guid& to, ContentID contentID, Category) = 0;
        virtual bool sendDcsmContentReady(const Guid& to, ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor) = 0;
        virtual bool sendDcsmContentFocusRequest(const Guid& to, ContentID contentID) = 0;
        virtual bool sendDcsmBroadcastRequestStopOfferContent(ContentID contentID) = 0;
        virtual bool sendDcsmBroadcastForceStopOfferContent(ContentID contentID) = 0;

        // dcsm consumer -> provider
        virtual bool sendDcsmCanvasSizeChange(const Guid& to, ContentID contentID, SizeInfo sizeinfo, AnimationInformation) = 0;
        virtual bool sendDcsmContentStateChange(const Guid& to, ContentID contentID, EDcsmState status, SizeInfo, AnimationInformation) = 0;


        // message limits configuration
        virtual CommunicationSendDataSizes getSendDataSizes() const = 0;
        virtual void setSendDataSizes(const CommunicationSendDataSizes& sizes) = 0;

        // set service handlers
        virtual void setResourceProviderServiceHandler(IResourceProviderServiceHandler* handler) = 0;
        virtual void setResourceConsumerServiceHandler(IResourceConsumerServiceHandler* handler) = 0;
        virtual void setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler) = 0;
        virtual void setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler) = 0;

        virtual void setDcsmProviderServiceHandler(IDcsmProviderServiceHandler* handler) = 0;
        virtual void setDcsmConsumerServiceHandler(IDcsmConsumerServiceHandler* handler) = 0;


        // log triggers
        virtual void logConnectionInfo() = 0;
        virtual void triggerLogMessageForPeriodicLog() override = 0;
    };
}

#endif
