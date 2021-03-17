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
#include "Components/DcsmMetadata.h"
#include "TransportCommon/IConnectionStatusListener.h"
#include "TransportCommon/ServiceHandlerInterfaces.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "ramses-framework-api/DcsmApiTypes.h"
#include "Collections/HashMap.h"
#include "Collections/Guid.h"
#include "Utils/IPeriodicLogSupplier.h"
#include "Collections/HashSet.h"
#include <chrono>
#include <condition_variable>
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

    class DcsmComponent final : public IDcsmComponent, public IConnectionStatusListener, public IDcsmProviderServiceHandler, public IDcsmConsumerServiceHandler, public IPeriodicLogSupplier
    {
    public:
        DcsmComponent(const Guid& myID, ICommunicationSystem& communicationSystem, IConnectionStatusUpdateNotifier& connectionStatusUpdateNotifier, PlatformLock& frameworkLock);
        virtual ~DcsmComponent() override;

        bool connect();
        bool disconnect();

        virtual bool setLocalConsumerAvailability(bool available) override;
        virtual bool setLocalProviderAvailability(bool available) override;

        virtual void newParticipantHasConnected(const Guid& guid) override;
        virtual void participantHasDisconnected(const Guid& guid) override;

        // Local consumer send methods
        virtual bool sendCanvasSizeChange(ContentID contentID, const CategoryInfo& categoryInfo, AnimationInformation ai) override;
        virtual bool sendContentStateChange(ContentID contentID, EDcsmState status, const CategoryInfo& categoryInfo, AnimationInformation ai) override;

        // Local provider send methods
        virtual bool sendOfferContent(ContentID contentID, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName, bool localOnly) override;
        virtual bool sendContentDescription(ContentID contentID, TechnicalContentDescriptor technicalContentDescriptor) override;
        virtual bool sendContentReady(ContentID contentID) override;
        virtual bool sendContentEnableFocusRequest(ContentID contentID, int32_t) override;
        virtual bool sendContentDisableFocusRequest(ContentID contentID, int32_t) override;
        virtual bool sendRequestStopOfferContent(ContentID contentID) override;
        virtual bool sendUpdateContentMetadata(ContentID contentID, const DcsmMetadata& metadata) override;

        // IDcsmProviderServiceHandler implementation
        virtual void handleCanvasSizeChange(ContentID contentID, const CategoryInfo& categoryInfo, AnimationInformation, const Guid& consumerID) override;
        virtual void handleContentStateChange(ContentID contentID, EDcsmState status, const CategoryInfo& categoryInfo, AnimationInformation, const Guid& consumerID) override;

        // IDcsmConsumerServiceHandler implementation
        virtual void handleOfferContent(ContentID contentID, Category, ETechnicalContentType technicalContentType, const std::string& friendlyName, const Guid& providerID) override;
        virtual void handleContentDescription(ContentID contentID, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID) override;
        virtual void handleContentReady(ContentID contentID, const Guid& providerID) override;
        virtual void handleContentEnableFocusRequest(ContentID contentID, int32_t focusRequest, const Guid& providerID) override;
        virtual void handleContentDisableFocusRequest(ContentID contentID, int32_t focusRequest, const Guid& providerID) override;
        virtual void handleRequestStopOfferContent(ContentID contentID, const Guid& providerID) override;
        virtual void handleForceStopOfferContent(ContentID contentID, const Guid& providerID) override;
        virtual void handleUpdateContentMetadata(ContentID contentID, DcsmMetadata metadata, const Guid& providerID) override;

        virtual bool dispatchProviderEvents(IDcsmProviderEventHandler& handler) override;
        virtual bool dispatchConsumerEvents(ramses::IDcsmConsumerEventHandler& handler) override;

        virtual bool dispatchProviderEvents(IDcsmProviderEventHandler& handler, std::chrono::milliseconds timeout) override;
        virtual bool dispatchConsumerEvents(ramses::IDcsmConsumerEventHandler& handler, std::chrono::milliseconds timeout) override;

        // for logging
        void writeStateForLog(StringOutputStream& sos);
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
            std::string friendlyName;
            ContentState state;
            Guid providerID;
            Guid consumerID;
            DcsmMetadata metadata;
            std::vector<int32_t> m_currentFocusRequests;
            bool localOnly;
            ETechnicalContentType contentType;
            TechnicalContentDescriptor contentDescriptor;
        };

        enum class EDcsmCommandType {
            OfferContent,
            ContentDescription,
            ContentReady,
            ContentStateChange,
            CanvasSizeChange,
            ContentEnableFocusRequest,
            ContentDisableFocusRequest,
            StopOfferContentRequest,
            ForceStopOfferContent,
            UpdateContentMetadata,
        };

        struct DcsmEvent
        {
            EDcsmCommandType           cmdType;
            ContentID                  contentID;
            Category                   category;
            TechnicalContentDescriptor descriptor;
            ETechnicalContentType      contentType = ETechnicalContentType::RamsesSceneID;
            EDcsmState                 state       = EDcsmState::Offered;
            CategoryInfo               categoryInfo;
            AnimationInformation       animation  {0, 0};
            DcsmMetadata               metadata;
            int32_t                    focusRequest = 0;
            Guid                       from;
        };

        void addProviderEvent_CanvasSizeChange(ContentID contentID, CategoryInfo categoryInfo, AnimationInformation, const Guid& consumerID);
        void addProviderEvent_ContentStateChange(ContentID contentID, EDcsmState status, CategoryInfo categoryInfo,AnimationInformation, const Guid& consumerID);
        void addConsumerEvent_OfferContent(ContentID contentID, Category, ETechnicalContentType technicalContentType, const Guid& providerID);
        void addConsumerEvent_ContentDescription(ContentID contentID, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID);
        void addConsumerEvent_ContentReady(ContentID contentID, const Guid& providerID);
        void addConsumerEvent_ContentEnableFocusRequest(ContentID contentID, int32_t focusRequest, const Guid& providerID);
        void addConsumerEvent_ContentDisableFocusRequest(ContentID contentID, int32_t focusRequest, const Guid& providerID);
        void addConsumerEvent_RequestStopOfferContent(ContentID contentID, const Guid& providerID);
        void addConsumerEvent_ForceStopOfferContent(ContentID contentID, const Guid& providerID);
        void addConsumerEvent_UpdateContentMetadata(ContentID contentID,  DcsmMetadata metadata, const Guid& providerID);

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

        bool m_connected = false;
        bool m_localConsumerAvailable = false;
        bool m_localProviderAvailable = false;

        HashSet<Guid> m_connectedParticipants;
        HashMap<ContentID, ContentInfo> m_contentRegistry;

        std::vector<DcsmEvent> m_providerEvents;
        std::condition_variable_any m_providerEventsSignal;
        std::vector<DcsmEvent> m_consumerEvents;
        std::condition_variable_any m_consumerEventsSignal;
    };
}

#endif
