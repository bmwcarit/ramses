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
#include <unordered_map>


namespace ramses
{
    class IDcsmConsumerEventHandler;
}

namespace ramses_internal
{
    // TODO(tobias): add content state transitions test (register, unregister done): canvas change maybe?

    class ICommunicationSystem;
    class IConnectionStatusUpdateNotifier;
    class IDcsmProviderEventHandler;
    class IDcsmConsumerEventHandler;

    class DcsmComponent final : public IDcsmComponent, public IConnectionStatusListener, public IDcsmProviderServiceHandler, public IDcsmConsumerServiceHandler
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
        virtual bool sendContentStatusChange(ContentID contentID, EDcsmStatus status, AnimationInformation ai) override;

        // Local provider send methods
        virtual bool sendRegisterContent(ContentID contentID, Category) override;
        virtual bool sendContentAvailable(const Guid& to, ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor) override;
        virtual bool sendCategoryContentSwitchRequest(const Guid& to, ContentID contentID) override;
        virtual bool sendRequestUnregisterContent(ContentID contentID) override;

        // IDcsmProviderServiceHandler implementation
        virtual void handleCanvasSizeChange(ContentID contentID, SizeInfo sizeinfo, AnimationInformation, const Guid& consumerID) override;
        virtual void handleContentStatusChange(ContentID contentID, EDcsmStatus status, AnimationInformation, const Guid& consumerID) override;

        // IDcsmConsumerServiceHandler implementation
        virtual void handleRegisterContent(ContentID contentID, Category, const Guid& providerID) override;
        virtual void handleContentAvailable(ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID) override;
        virtual void handleCategoryContentSwitchRequest(ContentID contentID, const Guid& providerID) override;
        virtual void handleRequestUnregisterContent(ContentID contentID, const Guid& providerID) override;

        virtual bool dispatchProviderEvents(IDcsmProviderEventHandler& handler) override;
        virtual bool dispatchConsumerEvents(ramses::IDcsmConsumerEventHandler& handler) override;

        // For testing only
        Guid getContentProviderID(ContentID content) const;

    private:
        struct ContentInfo
        {
            ContentID content;
            Category category;
            Guid providerID;
            bool unregisterRequested;
            Guid consumerID;
            EDcsmStatus consumedStatus;
        };

        enum class EDcsmCommandType {
            RegisterContent,
            ContentAvailable,
            ContentStatusChange,
            CanvasSizeChange,
            CategoryContentSwitchRequest,
            UnregisterContent,
            ForceUnregisterContent,
        };

        struct DcsmEvent
        {
            EDcsmCommandType           cmdType;
            ContentID                  contentID;
            Category                   category;
            TechnicalContentDescriptor descriptor;
            ETechnicalContentType      contentType;
            EDcsmStatus                status;
            SizeInfo                   size;
            AnimationInformation       animation;
            Guid                       from;
        };

        void addProviderEvent_CanvasSizeChange(ContentID contentID, SizeInfo sizeinfo, AnimationInformation, const Guid& consumerID);
        void addProviderEvent_ContentStatusChange(ContentID contentID, EDcsmStatus status, AnimationInformation, const Guid& consumerID);
        void addConsumerEvent_RegisterContent(ContentID contentID, Category, const Guid& providerID);
        void addConsumerEvent_ContentAvailable(ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID);
        void addConsumerEvent_CategoryContentSwitchRequest(ContentID contentID, const Guid& providerID);
        void addConsumerEvent_RequestUnregisterContent(ContentID contentID, const Guid& providerID);
        void addConsumerEvent_ForceUnregisterContent(ContentID contentID, const Guid& providerID);

        const char* EnumToString(EDcsmCommandType cmd) const;
        bool isValidETechnicalContentType(const char* callerMethod, ETechnicalContentType val) const;
        bool isValidEDcsmStatus(const char* callerMethod, EDcsmStatus val) const;
        bool isAllowedToSendTo(const char* callerMethod, const Guid& id) const;
        bool isAllowedToReceiveFrom(const char* callerMethod, const Guid& id) const;
        bool isLocallyProvidingContent(const char* callerMethod, ContentID content) const;
        bool isContentKnown(const char* callerMethod, ContentID content) const;

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
