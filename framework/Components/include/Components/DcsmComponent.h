//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMPONENT_DCSMCOMPONENT_H
#define RAMSES_COMPONENT_DCSMCOMPONENT_H

#include "Components/DcsmTypes.h"
#include "Components/IDcsmComponent.h"
#include "TransportCommon/IConnectionStatusListener.h"
#include "TransportCommon/ServiceHandlerInterfaces.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "ramses-framework-api/DcsmApiTypes.h"
#include "Collections/HashMap.h"
#include "Utils/IPeriodicLogSupplier.h"
#include <unordered_map>


namespace ramses
{
    class IDcsmConsumerEventHandler;
}

namespace ramses_internal
{
    class ICommunicationSystem;
    class IConnectionStatusUpdateNotifier;
    class IDcsmProviderEventHandler;
    class IDcsmConsumerEventHandler;

    class DcsmComponent final : public IDcsmComponent, public IConnectionStatusListener, public IDcsmProviderServiceHandler, public IDcsmConsumerServiceHandler, public IPeriodicLogSupplier
    {
    public:
        DcsmComponent(const Guid& myID, ICommunicationSystem& communicationSystem, IConnectionStatusUpdateNotifier& connectionStatusUpdateNotifier, PlatformLock& frameworkLock);
        virtual ~DcsmComponent() override;

        virtual bool setLocalConsumerAvailability(bool available) override;
        virtual bool setLocalProviderAvailability(bool available) override;

        virtual void newParticipantHasConnected(const Guid& guid) override;
        virtual void participantHasDisconnected(const Guid& guid) override;

        // Local consumer send methods
        virtual bool sendCanvasSizeChange(ContentID contentID, SizeInfo sizeInfo, AnimationInformation ai) override;
        virtual bool sendContentStateChange(ContentID contentID, EDcsmState status, SizeInfo sizeInfo, AnimationInformation ai) override;

        // Local provider send methods
        virtual bool sendOfferContent(ContentID contentID, Category) override;
        virtual bool sendContentReady(ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor) override;
        virtual bool sendContentFocusRequest(ContentID contentID) override;
        virtual bool sendRequestStopOfferContent(ContentID contentID) override;

        // IDcsmProviderServiceHandler implementation
        virtual void handleCanvasSizeChange(ContentID contentID, SizeInfo sizeinfo, AnimationInformation, const Guid& consumerID) override;
        virtual void handleContentStateChange(ContentID contentID, EDcsmState status, SizeInfo, AnimationInformation, const Guid& consumerID) override;

        // IDcsmConsumerServiceHandler implementation
        virtual void handleOfferContent(ContentID contentID, Category, const Guid& providerID) override;
        virtual void handleContentReady(ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID) override;
        virtual void handleContentFocusRequest(ContentID contentID, const Guid& providerID) override;
        virtual void handleRequestStopOfferContent(ContentID contentID, const Guid& providerID) override;
        virtual void handleForceStopOfferContent(ContentID contentID, const Guid& providerID) override;

        virtual bool dispatchProviderEvents(IDcsmProviderEventHandler& handler) override;
        virtual bool dispatchConsumerEvents(ramses::IDcsmConsumerEventHandler& handler) override;

        // for logging
        void logInfo();
        virtual void triggerLogMessageForPeriodicLog() override;

        // For testing only
        Guid getContentProviderID(ContentID content) const;
        Guid getContentConsumerID(ContentID content) const;

        enum class ContentState
        {
            Unknown,
            Offered,
            Assigned,
            ReadyRequested,
            Ready,
            Shown,
            StopOfferRequested,
        };
        ContentState getContentState(ContentID content) const;

    private:
        struct ContentInfo
        {
            ContentID content;
            Category category;
            ContentState state;
            Guid providerID;
            Guid consumerID;
            // TODO(tobias): add more infos (tech etc) for periodic/ramsh logging
        };

        enum class EDcsmCommandType {
            OfferContent,
            ContentReady,
            ContentStateChange,
            CanvasSizeChange,
            ContentFocusRequest,
            StopOfferContentRequest,
            ForceStopOfferContent,
        };

        struct DcsmEvent
        {
            EDcsmCommandType           cmdType;
            ContentID                  contentID;
            Category                   category;
            TechnicalContentDescriptor descriptor;
            ETechnicalContentType      contentType;
            EDcsmState                 state;
            SizeInfo                   size;
            AnimationInformation       animation;
            Guid                       from;
        };

        void addProviderEvent_CanvasSizeChange(ContentID contentID, SizeInfo sizeinfo, AnimationInformation, const Guid& consumerID);
        void addProviderEvent_ContentStateChange(ContentID contentID, EDcsmState status, SizeInfo sizeInfo,AnimationInformation, const Guid& consumerID);
        void addConsumerEvent_OfferContent(ContentID contentID, Category, const Guid& providerID);
        void addConsumerEvent_ContentReady(ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID);
        void addConsumerEvent_ContentFocusRequest(ContentID contentID, const Guid& providerID);
        void addConsumerEvent_RequestStopOfferContent(ContentID contentID, const Guid& providerID);
        void addConsumerEvent_ForceStopOfferContent(ContentID contentID, const Guid& providerID);

        const char* EnumToString(EDcsmCommandType cmd) const;
        const char* EnumToString(ContentState val) const;

        bool isValidETechnicalContentType(const char* callerMethod, ETechnicalContentType val) const;
        bool isValidEDcsmState(const char* callerMethod, EDcsmState val) const;
        bool isAllowedToSendTo(const char* callerMethod, const Guid& id) const;
        bool isAllowedToReceiveFrom(const char* callerMethod, const Guid& id) const;
        bool isLocallyProvidingContent(const char* callerMethod, ContentID content) const;
        bool isValidContent(const char* callerMethod, ContentID content) const;
        bool isValidStateTransition(const char* callerMethod, const ContentInfo& ci, EDcsmState transition, ContentState& newState) const;

        const Guid m_myID;
        ICommunicationSystem& m_communicationSystem;
        IConnectionStatusUpdateNotifier& m_connectionStatusUpdateNotifier;
        PlatformLock& m_frameworkLock;

        bool m_localConsumerAvailable = false;
        bool m_localProviderAvailable = false;

        HashSet<Guid> m_connectedParticipants;
        HashMap<ContentID, ContentInfo> m_contentRegistry;

        std::vector<DcsmEvent> m_providerEvents;
        std::vector<DcsmEvent> m_consumerEvents;
    };
}

#endif
