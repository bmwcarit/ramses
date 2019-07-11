//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/DcsmComponent.h"
#include "Components/IDcsmProviderEventHandler.h"
#include "TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "TransportCommon/ICommunicationSystem.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "Utils/LogMacros.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"

namespace ramses_internal
{
    DcsmComponent::DcsmComponent(const Guid& myID, ICommunicationSystem& communicationSystem, IConnectionStatusUpdateNotifier& connectionStatusUpdateNotifier, PlatformLock& frameworkLock)
        : m_myID(myID)
        , m_communicationSystem(communicationSystem)
        , m_connectionStatusUpdateNotifier(connectionStatusUpdateNotifier)
        , m_frameworkLock(frameworkLock)
    {
        assert(!m_myID.isInvalid());

        m_connectionStatusUpdateNotifier.registerForConnectionUpdates(this);
        m_communicationSystem.setDcsmProviderServiceHandler(this);
        m_communicationSystem.setDcsmConsumerServiceHandler(this);
    }

    DcsmComponent::~DcsmComponent()
    {
        m_connectionStatusUpdateNotifier.unregisterForConnectionUpdates(this);
    }

    bool DcsmComponent::setLocalConsumerAvailability(bool available)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalConsumerAvailability: availability " << available << ", was " << m_localConsumerAvailable);

        PlatformGuard guard(m_frameworkLock);
        if (available == m_localConsumerAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalConsumerAvailability: called without state change, current state " << m_localConsumerAvailable);
            return false;
        }
        m_localConsumerAvailable = available;

        if (m_localConsumerAvailable)
        {
            m_consumerEvents.clear();

            // newly available: create events for all known contents
            for (auto& ci : m_contentRegistry)
            {
                if (ci.value.state != ContentState::Unknown)
                {
                    LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalConsumerAvailability: local OfferContent content " << ci.value.content << ", category " << ci.value.category <<
                             ", from " << ci.value.providerID << ", state " << EnumToString(ci.value.state));
                    addConsumerEvent_OfferContent(ci.value.content, ci.value.category, ci.value.providerID);
                }
            }
        }
        else
        {
            // consumer became unavilable: send update for all locally consumed contents
            std::vector<ContentID> contentToRemove;
            for (auto& ci : m_contentRegistry)
            {
                if (ci.value.consumerID == m_myID)
                {
                    if (ci.value.state == ContentState::StopOfferRequested)
                    {
                        // provider wanted to remove and now local consumer gone -> inform provider and remove locally
                        if (ci.value.providerID == m_myID)
                        {
                            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalConsumerAvailability: Queue StopOffer state and remove locally provided content " << ci.key << " because was requested and consumer now gone");
                            addProviderEvent_ContentStateChange(ci.value.content, EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{0, 0}, m_myID);
                        }
                        else
                        {
                            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalConsumerAvailability: Send AcceptStopOffer and remove remotely provided content " << ci.key << " because was requested and consumer now gone");
                            m_communicationSystem.sendDcsmContentStateChange(ci.value.providerID, ci.value.content, EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{0, 0});
                        }
                        ci.value.state = ContentState::Unknown;
                    }
                    else if (ci.value.state != ContentState::Offered && ci.value.state != ContentState::Unknown)
                    {
                        // content was in use by local consumer, inform provider that not actively in use anymore
                        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalConsumerAvailability: Send contentStateChange to " << ci.value.providerID << " for locally consumed content " << ci.value.content <<
                                 ", status " << EnumToString(ci.value.state) << " -> Offered");
                        if (ci.value.providerID == m_myID)
                        {
                            addProviderEvent_ContentStateChange(ci.value.content, EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{0, 0}, m_myID);
                        }
                        else
                        {
                            m_communicationSystem.sendDcsmContentStateChange(ci.value.providerID, ci.value.content, EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{0, 0});
                        }
                        ci.value.state = ContentState::Offered;
                        ci.value.consumerID = Guid(false);
                    }
                }
            }
            m_consumerEvents.clear();
        }

        return true;
    }

    bool DcsmComponent::setLocalProviderAvailability(bool available)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalProviderAvailability: availability " << available << ", was " << m_localProviderAvailable);

        PlatformGuard guard(m_frameworkLock);
        if (available == m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalProviderAvailability: called without state change, current state " << m_localProviderAvailable);
            return false;
        }
        m_localProviderAvailable = available;

        m_providerEvents.clear();
        if (!m_localProviderAvailable)
        {
            // send force stop offer for all contents offered locally
            for (auto& ci : m_contentRegistry)
            {
                if (ci.value.providerID == m_myID)
                {
                    LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalProviderAvailability: ForceStopOfferContent for local content " << ci.value.content << ", consumer " <<
                             ci.value.consumerID << ", state " << EnumToString(ci.value.state));

                    m_communicationSystem.sendDcsmBroadcastForceStopOfferContent(ci.value.content);
                    if (m_localConsumerAvailable)
                        addConsumerEvent_ForceStopOfferContent(ci.value.content, m_myID);

                    ci.value.state = ContentState::Unknown;
                }
            }
        }

        return true;
    }

    void DcsmComponent::newParticipantHasConnected(const Guid& newParticipant)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::newParticipantHasConnected: " << newParticipant);

        if (newParticipant == m_myID)
        {
            LOG_ERROR(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::newParticipantHasConnected: Ignore remote " << newParticipant << ", has same id as local");
            return;
        }

        if (m_connectedParticipants.hasElement(newParticipant))
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::newParticipantHasConnected: duplicate connect from " << newParticipant
                     << ", send offer anyway");
        }
        m_connectedParticipants.put(newParticipant);

        // send own content to new participant
        for (auto& ci : m_contentRegistry)
            if (ci.value.providerID == m_myID && ci.value.state != ContentState::Unknown)
                m_communicationSystem.sendDcsmOfferContent(newParticipant, ci.value.content, ci.value.category);
    }

    void DcsmComponent::participantHasDisconnected(const Guid& from)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::participantHasDisconnected: " << from);

        if (from == m_myID)
        {
            LOG_ERROR(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::participantHasDisconnected: Ignore remote " << from << ", has same id as local");
            return;
        }

        if (!m_connectedParticipants.hasElement(from))
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::participantHasDisconnected: ignore from unknown participant " << from);
            return;
        }

        // stop offer locally
        if (m_localConsumerAvailable)
        {
            // force stop offer content from disappeared participant for local consumer
            for (const auto& ci : m_contentRegistry)
                if (ci.value.providerID == from)
                {
                    LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::participantHasDisconnected: ForceStopOffer to local consumer " << ci.value.content << ", state " << EnumToString(ci.value.state));
                    addConsumerEvent_ForceStopOfferContent(ci.value.content, ci.value.providerID);
                }
        }

        if (m_localProviderAvailable)
        {
            // handle content locally provided and consumed by disconnected remote
            for (auto& ci : m_contentRegistry)
            {
                if (ci.value.providerID == m_myID && ci.value.consumerID == from)
                {
                    if (ci.value.state == ContentState::StopOfferRequested)
                    {
                        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::participantHasDisconnected: Queue AcceptStopOffer and remove locally provided content " << ci.key << " because was requested and consumer now gone");
                        addProviderEvent_ContentStateChange(ci.value.content, EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{0, 0}, m_myID);
                        ci.value.state = ContentState::Unknown;
                    }
                    else if (ci.value.state != ContentState::Offered && ci.value.state != ContentState::Unknown)
                    {
                        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::participantHasDisconnected: Queue contentStateChange for locally provided and remotely consumed content " << ci.value.content <<
                                 ", status " << EnumToString(ci.value.state) << " -> Offered");

                        addProviderEvent_ContentStateChange(ci.value.content, EDcsmState::Offered, SizeInfo{0, 0}, AnimationInformation{0, 0}, m_myID);
                        ci.value.state = ContentState::Offered;
                        ci.value.consumerID = Guid(false);
                    }
                }
            }
        }

        // set everything from disconnected provider to unknown
        for (auto& ci : m_contentRegistry)
        {
            if (ci.value.providerID == from)
                ci.value.state = ContentState::Unknown;
        }

        m_connectedParticipants.remove(from);
    }

    bool DcsmComponent::sendOfferContent(ContentID contentID, Category category)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendOfferContent: ContentID " << contentID << ", Category " << category);

        PlatformGuard guard(m_frameworkLock);
        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendOfferContent: called without local provider active");
            return false;
        }

        if (const ContentInfo* ci = m_contentRegistry.get(contentID))
        {
            if (ci->state == ContentState::StopOfferRequested &&
                ci->providerID == m_myID)
            {
                LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendOfferContent: ContentID " << contentID << " StopOfferRequested -> Offered");
            }
            else if (ci->state != ContentState::Unknown)
            {
                LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendOfferContent: tried to offer content " << contentID << " already provided by " << ci->providerID);
                return false;
            }
            else if (ci->providerID != m_myID)
            {
                LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendOfferContent: offering content " << contentID << " previously provided by " << ci->providerID);
            }
        }

        if (m_localConsumerAvailable)
        {
            addConsumerEvent_OfferContent(contentID, category, m_myID);
        }
        m_communicationSystem.sendDcsmBroadcastOfferContent(contentID, category);

        m_contentRegistry.put(contentID, ContentInfo{contentID, category, ContentState::Offered, m_myID, Guid(false)});
        return true;
    }

    bool DcsmComponent::sendContentReady(ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor)
    {
        PlatformGuard guard(m_frameworkLock);

        const Guid& to = getContentConsumerID(contentID);
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentReady: to " << to << ", ContentID " << contentID <<
                 ", ETechnicalContentType " << ramses_internal::EnumToString(technicalContentType) << ", TechnicalContentDescriptor " << technicalContentDescriptor);

        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentReady: called without local provider active");
            return false;
        }
        const char* methodName = "sendContentReady";
        if (!isAllowedToSendTo(methodName, to))
            return false;
        if (!isValidETechnicalContentType(methodName, technicalContentType))
            return false;
        if (!isLocallyProvidingContent(methodName, contentID))
            return false;

        ContentInfo& ci = m_contentRegistry[contentID];
        if (ci.state != ContentState::ReadyRequested)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentReady: called on content " << contentID << " in invalid state " << EnumToString(ci.state));
            return false;
        }

        if (m_myID == to)
        {
            assert(m_localConsumerAvailable && "Logic error on consumer disable");
            addConsumerEvent_ContentReady(contentID, technicalContentType, technicalContentDescriptor, m_myID);
        }
        else
        {
            m_communicationSystem.sendDcsmContentReady(to, contentID, technicalContentType, technicalContentDescriptor);
        }

        ci.state = ContentState::Ready;

        return true;
    }

    bool DcsmComponent::sendContentFocusRequest(ContentID contentID)
    {
        PlatformGuard guard(m_frameworkLock);

        const Guid& to = getContentConsumerID(contentID);
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentFocusRequest: to " << to << ", ContentID " << contentID);

        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentFocusRequest: called without local provider active");
            return false;
        }
        const char* methodName = "sendContentFocusRequest";
        if (!isAllowedToSendTo(methodName, to))
            return false;
        if (!isLocallyProvidingContent(methodName, contentID))
            return false;

        ContentInfo& ci = m_contentRegistry[contentID];
        if (ci.state != ContentState::Assigned &&
            ci.state != ContentState::ReadyRequested &&
            ci.state != ContentState::Ready &&
            ci.state != ContentState::Shown)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentReady: called on content " << contentID << " in invalid state " << EnumToString(ci.state));
            return false;
        }

        if (m_myID == to)
        {
            assert(m_localConsumerAvailable && "Logic error on consumer disable");
            addConsumerEvent_ContentFocusRequest(contentID, m_myID);
        }
        else
        {
            m_communicationSystem.sendDcsmContentFocusRequest(to, contentID);
        }
        return true;
    }

    bool DcsmComponent::sendRequestStopOfferContent(ContentID contentID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRequestStopOfferContent: ContentID " << contentID);

        PlatformGuard guard(m_frameworkLock);
        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRequestStopOfferContent: called without local provider active");
            return false;
        }

        const char* methodName = "sendRequestStopOfferContent";
        if (!isLocallyProvidingContent(methodName, contentID))
            return false;

        ContentInfo& ci = m_contentRegistry[contentID];
        if (ci.state == ContentState::StopOfferRequested)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRequestStopOfferContent: called more than once for content " << contentID);
            return false;
        }

        if (m_localConsumerAvailable)
        {
            addConsumerEvent_RequestStopOfferContent(contentID, m_myID);
        }
        m_communicationSystem.sendDcsmBroadcastRequestStopOfferContent(contentID);

        if (ci.state == ContentState::Offered)
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRequestStopOfferContent: set content " << contentID <<
                     " state Unknown because no active consumer");
            assert(ci.consumerID == Guid(false));
            ci.state = ContentState::Unknown;
            addProviderEvent_ContentStateChange(contentID, EDcsmState::AcceptStopOffer, SizeInfo{0, 0}, AnimationInformation{0, 0}, Guid(false));
        }
        else
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRequestStopOfferContent: set content " << contentID <<
                     " state " << EnumToString(ci.state) << " -> StopOfferRequested, consumed by " << ci.consumerID);
            ci.state = ContentState::StopOfferRequested;
        }

        return true;
    }

    bool DcsmComponent::sendCanvasSizeChange(ContentID contentID, SizeInfo sizeInfo, AnimationInformation ai)
    {
        PlatformGuard guard(m_frameworkLock);

        const Guid& to = getContentProviderID(contentID);
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendCanvasSizeChange: to " << to << ", ContentID " << contentID <<
                 ", SizeInfo " << sizeInfo.width << "x" << sizeInfo.height << ", ai [" << ai.startTimeStamp << ", " << ai.finishedTimeStamp << "]");

        if (!m_localConsumerAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendCanvasSizeChange: called without local consumer active");
            return false;
        }
        const char* methodName = "sendCanvasSizeChange";
        if (!isAllowedToSendTo(methodName, to))
            return false;
        if (!isValidContent(methodName, contentID))
            return false;

        const ContentInfo& ci = *m_contentRegistry.get(contentID);
        if (ci.consumerID != m_myID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendCanvasSizeChange: cannot control content " << contentID << " consumed by " << ci.consumerID);
            return false;
        }
        assert((ci.state == ContentState::Assigned ||
                ci.state == ContentState::ReadyRequested ||
                ci.state == ContentState::Ready ||
                ci.state == ContentState::Shown ||
                ci.state == ContentState::StopOfferRequested)
                && "Logic error in consumer tracking");

        if (m_myID == to)
        {
            assert(m_localProviderAvailable && "Logic error in provider disable");
            addProviderEvent_CanvasSizeChange(contentID, sizeInfo, ai, m_myID);
        }
        else
        {
            m_communicationSystem.sendDcsmCanvasSizeChange(to, contentID, sizeInfo, ai);
        }
        return true;
    }

    bool DcsmComponent::sendContentStateChange(ContentID contentID, EDcsmState state, SizeInfo sizeInfo, AnimationInformation ai)
    {
        PlatformGuard guard(m_frameworkLock);

        const Guid& to = getContentProviderID(contentID);
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStateChange: to " << to << ", ContentID " << contentID <<
                 ", EDcsmStatus " << ramses_internal::EnumToString(state) << ", ai [" << ai.startTimeStamp << ", " << ai.finishedTimeStamp << "]");

        if (!m_localConsumerAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStateChange: called without local consumer active");
            return false;
        }
        const char* methodName = "sendContentStateChange";
        if (!isAllowedToSendTo(methodName, to))
            return false;
        if (!isValidEDcsmState(methodName, state))
            return false;
        if (!isValidContent(methodName, contentID))
            return false;

        ContentInfo& ci = *m_contentRegistry.get(contentID);
        if (ci.state != ContentState::Offered &&
            ci.consumerID != m_myID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStateChange: cannot control content " << contentID << " consumed by " << ci.consumerID << " in state " << EnumToString(ci.state));
            return false;
        }

        if (ci.state == ContentState::Offered && state == EDcsmState::Assigned && sizeInfo == SizeInfo{0, 0})
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStateChange: needs valid SizeInfo for Offered -> Assigned transition");
            return false;
        }
        else if ((ci.state != ContentState::Offered || state != EDcsmState::Assigned) && sizeInfo != SizeInfo{0, 0})
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStateChange: needs zero SizeInfo for " << EnumToString(ci.state) << " -> " << ramses_internal::EnumToString(state) << " transition");
            return false;
        }

        ContentState newState = ContentState::Unknown;
        if (!isValidStateTransition(methodName, ci, state, newState))
            return false;
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStateChange: state change " << EnumToString(ci.state) << " -> " << ramses_internal::EnumToString(state) << " -> " << EnumToString(newState));
        ci.state = newState;

        if (ci.state == ContentState::Offered)
            ci.consumerID = Guid(false);
        if (ci.state == ContentState::Assigned)
            ci.consumerID = m_myID;

        if (m_myID == to)
        {
            assert(m_localProviderAvailable && "Logic error in provider disable");
            addProviderEvent_ContentStateChange(contentID, state, sizeInfo, ai, m_myID);
        }
        else
        {
            m_communicationSystem.sendDcsmContentStateChange(to, contentID, state, sizeInfo, ai);
        }
        return true;
    }

    void DcsmComponent::addProviderEvent_CanvasSizeChange(ContentID contentID, SizeInfo sizeInfo, AnimationInformation ai, const Guid& consumerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::CanvasSizeChange;
        event.contentID = contentID;
        event.size      = sizeInfo;
        event.animation = ai;
        event.from      = consumerID;
        m_providerEvents.push_back(event);
    }

    void DcsmComponent::addProviderEvent_ContentStateChange(ContentID contentID, EDcsmState status, SizeInfo sizeInfo, AnimationInformation ai, const Guid& consumerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::ContentStateChange;
        event.contentID = contentID;
        event.state     = status;
        event.size      = sizeInfo;
        event.animation = ai;
        event.from      = consumerID;
        m_providerEvents.push_back(event);
    }

    void DcsmComponent::addConsumerEvent_OfferContent(ContentID contentID, Category category, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::OfferContent;
        event.contentID = contentID;
        event.category  = category;
        event.from      = providerID;
        m_consumerEvents.push_back(event);
    }

    void DcsmComponent::addConsumerEvent_ContentReady(ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType     = EDcsmCommandType::ContentReady;
        event.contentID   = contentID;
        event.contentType = technicalContentType;
        event.descriptor  = technicalContentDescriptor;
        event.from        = providerID;
        m_consumerEvents.push_back(event);
    }

    void DcsmComponent::addConsumerEvent_ContentFocusRequest(ContentID contentID, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::ContentFocusRequest;
        event.contentID = contentID;
        event.from      = providerID;
        m_consumerEvents.push_back(event);
    }

    void DcsmComponent::addConsumerEvent_RequestStopOfferContent(ContentID contentID, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::StopOfferContentRequest;
        event.contentID = contentID;
        event.from      = providerID;
        m_consumerEvents.push_back(event);
    }

    void DcsmComponent::addConsumerEvent_ForceStopOfferContent(ContentID contentID, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::ForceStopOfferContent;
        event.contentID = contentID;
        event.from      = providerID;
        m_consumerEvents.push_back(event);
    }

    const char* DcsmComponent::EnumToString(EDcsmCommandType cmd) const
    {
        switch (cmd)
        {
        case EDcsmCommandType::OfferContent:
            return "EDcsmCommandType::OfferContent";
        case EDcsmCommandType::ContentReady:
            return "EDcsmCommandType::ContentReady";
        case EDcsmCommandType::ContentStateChange:
            return "EDcsmCommandType::ContentStateChange";
        case EDcsmCommandType::CanvasSizeChange:
            return "EDcsmCommandType::CanvasSizeChange";
        case EDcsmCommandType::ContentFocusRequest:
            return "EDcsmCommandType::ContentFocusRequest";
        case EDcsmCommandType::StopOfferContentRequest:
            return "EDcsmCommandType::StopOfferContentRequest";
        case EDcsmCommandType::ForceStopOfferContent:
            return "EDcsmCommandType::ForceStopOfferContent";
        }
        return "<Invalid EDcsmCommandType>";
    }

    const char* DcsmComponent::EnumToString(ContentState val) const
    {
        switch(val)
        {
        case ContentState::Unknown:
            return "ContentState::Unknown";
        case ContentState::Offered:
            return "ContentState::Offered";
        case ContentState::Assigned:
            return "ContentState::Assigned";
        case ContentState::ReadyRequested:
            return "ContentState::ReadyRequested";
        case ContentState::Ready:
            return "ContentState::Ready";
        case ContentState::Shown:
            return "ContentState::Shown";
        case ContentState::StopOfferRequested:
            return "ContentState::StopOfferRequested";
        }
        return "<Invalid ContentState>";
    }

    // TODO(tobias): add error return channel and use on all false return in handle functions

    void DcsmComponent::handleCanvasSizeChange(ContentID contentID, SizeInfo sizeInfo, AnimationInformation ai, const Guid& consumerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleCanvasSizeChange: from " << consumerID << ", ContentID " << contentID <<
                 ", SizeInfo " << sizeInfo.width << "x" << sizeInfo.height << ", ai [" << ai.startTimeStamp << ", " << ai.finishedTimeStamp << "]");

        const char* methodName = "handleCanvasSizeChange";
        if (!isAllowedToReceiveFrom(methodName, consumerID))
            return;

        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleCanvasSizeChange: sent to non-existing local provider");
            return;
        }
        const ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleCanvasSizeChange: received from " << consumerID << " for unknown content " << contentID);
            return;
        }
        if (ci->providerID != m_myID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleCanvasSizeChange: received from " << consumerID << " for content " << contentID << " from remote " << ci->providerID);
            return;
        }
        if (!ci->consumerID.isInvalid() && ci->consumerID != consumerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleCanvasSizeChange: cannot control content " << contentID << " consumed by " << ci->consumerID);
            return;
        }
        if (ci->state != ContentState::Assigned &&
            ci->state != ContentState::ReadyRequested &&
            ci->state != ContentState::Ready &&
            ci->state != ContentState::Shown &&
            ci->state != ContentState::StopOfferRequested)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleCanvasSizeChange: invalid for content " << contentID << " in state " << EnumToString(ci->state));
            return;
        }

        addProviderEvent_CanvasSizeChange(contentID, sizeInfo, ai, consumerID);
    }

    void DcsmComponent::handleContentStateChange(ContentID contentID, EDcsmState state, SizeInfo si, AnimationInformation ai, const Guid& consumerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStateChange: from " << consumerID << ", ContentID " << contentID <<
                 ", EDcsmStatus " << ramses_internal::EnumToString(state) << ", ai [" << ai.startTimeStamp << ", " << ai.finishedTimeStamp << "]");

        const char* methodName = "handleContentStateChange";
        if (!isAllowedToReceiveFrom(methodName, consumerID))
            return;
        if (!isValidEDcsmState(methodName, state))
            return;

        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStateChange: sent to non-existing local provider");
            return;
        }
        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStateChange: received from " << consumerID << " for unknown content " << contentID);
            return;
        }
        if (ci->providerID != m_myID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStateChange: received from " << consumerID << " for content " << contentID << " from remote " << ci->providerID);
            return;
        }
        if (ci->state != ContentState::Offered &&
            ci->consumerID != consumerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStateChange: cannot control content " << contentID << " consumed by " << ci->consumerID << " in state " << EnumToString(ci->state));
            return;
        }

        ContentState newState = ContentState::Unknown;
        if (!isValidStateTransition(methodName, *ci, state, newState))
            return;
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStateChange: state change " << EnumToString(ci->state) << " -> " << ramses_internal::EnumToString(state) << " -> " << EnumToString(newState));
        ci->state = newState;

        if (ci->state == ContentState::Offered)
            ci->consumerID = Guid(false);
        if (ci->state == ContentState::Assigned)
            ci->consumerID = consumerID;

        addProviderEvent_ContentStateChange(contentID, state, si, ai, consumerID);
    }

    void DcsmComponent::handleOfferContent(ContentID contentID, Category category, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleOfferContent: from " << providerID << ", ContentID " << contentID << ", Category " << category << ", hasLocalConsumer " << m_localConsumerAvailable);

        const char* methodName = "handleOfferContent";
        if (!isAllowedToReceiveFrom(methodName, providerID))
            return;

        if (const ContentInfo* ci = m_contentRegistry.get(contentID))
        {
            if (ci->state == ContentState::StopOfferRequested &&
                ci->providerID == providerID)
            {
                LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleOfferContent: ContentID " << contentID << " StopOfferRequested -> Offered");
            }
            else if (ci->state != ContentState::Unknown)
            {
                LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleOfferContent: " << providerID << " tried to offer content " << contentID << " already provided by " << ci->providerID);
                return;
            }
            else if (ci->providerID != m_myID)
            {
                LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleOfferContent: " << providerID << " offered content " << contentID << " previously provided by " << ci->providerID);
            }
        }

        if (m_localConsumerAvailable)
        {
            addConsumerEvent_OfferContent(contentID, category, providerID);
        }

        m_contentRegistry.put(contentID, ContentInfo{contentID, category, ContentState::Offered, providerID, Guid(false)});
    }

    void DcsmComponent::handleContentReady(ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentReady: from " << providerID << ", ContentID " << contentID <<
                 ", ETechnicalContentType " << ramses_internal::EnumToString(technicalContentType) << ", TechnicalContentDescriptor " << technicalContentDescriptor);

        const char* methodName = "handleContentReady";
        if (!isAllowedToReceiveFrom(methodName, providerID))
            return;
        if (!isValidETechnicalContentType(methodName, technicalContentType))
            return;

        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentReady: received from " << providerID << " for unknown content " << contentID);
            return;
        }
        if (ci->state == ContentState::Unknown)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentReady: received from " << providerID << " not proving contentID " << contentID);
            return;
        }
        if (ci->providerID != providerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentReady: received from " << providerID << " for content " << contentID << " but registered by remote " << ci->providerID);
            return;
        }
        if (ci->state != ContentState::ReadyRequested)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentReady: received from " << providerID << " not allowed in state " << EnumToString(ci->state));
            return;
        }
        if (!m_localConsumerAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentReady: sent to non-existing local consumer");
            return;
        }

        ci->state = ContentState::Ready;
        addConsumerEvent_ContentReady(contentID, technicalContentType, technicalContentDescriptor, providerID);
    }

    void DcsmComponent::handleContentFocusRequest(ContentID contentID, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentFocusRequest: from " << providerID << ", ContentID " << contentID);

        const char* methodName = "handleContentFocusRequest";
        if (!isAllowedToReceiveFrom(methodName, providerID))
            return;

        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentFocusRequest: received from " << providerID << " for unknown content " << contentID);
            return;
        }
        if (ci->providerID != providerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentFocusRequest: received from " << providerID << " for content " << contentID << " but registered by remote " << ci->providerID);
            return;
        }
        if (ci->state != ContentState::Assigned &&
            ci->state != ContentState::ReadyRequested &&
            ci->state != ContentState::Ready &&
            ci->state != ContentState::Shown)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentReady: received from " << providerID << " for content " << contentID << " in invalid state " << EnumToString(ci->state));
            return;
        }
        if (!m_localConsumerAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentFocusRequest: sent to non-existing local consumer");
            return;
        }

        addConsumerEvent_ContentFocusRequest(contentID, providerID);
    }

    void DcsmComponent::handleRequestStopOfferContent(ContentID contentID, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRequestStopOfferContent: from " << providerID << ", ContentID " << contentID << ", hasLocalConsumer " << m_localConsumerAvailable);

        const char* methodName = "handleRequestStopOfferContent";
        if (!isAllowedToReceiveFrom(methodName, providerID))
            return;

        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRequestStopOfferContent: received from " << providerID << " for unknown content " << contentID);
            return;
        }
        if (ci->state == ContentState::Unknown)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRequestStopOfferContent: received from " << providerID << " not proving contentID " << contentID);
            return;
        }
        if (ci->providerID != providerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRequestStopOfferContent: received from " << providerID << " for content " << contentID << " but registered by remote " << ci->providerID);
            return;
        }
        if (ci->state == ContentState::StopOfferRequested)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRequestStopOfferContent: called more than once for content " << contentID << " from " << providerID);
            return;
        }

        if (m_localConsumerAvailable)
        {
            addConsumerEvent_RequestStopOfferContent(contentID, providerID);
        }

        if (ci->state == ContentState::Offered)
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRequestStopOfferContent: set content " << contentID <<
                     " state Unknown because no active consumer");
            assert(ci->consumerID == Guid(false));
            ci->state = ContentState::Unknown;
        }
        else
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRequestStopOfferContent: set content " << contentID <<
                     " state " << EnumToString(ci->state) << " -> StopOfferRequested, consumed by " << ci->consumerID);
            ci->state = ContentState::StopOfferRequested;
        }
    }

    void DcsmComponent::handleForceStopOfferContent(ContentID contentID, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleForceStopOfferContent: from " << providerID << ", ContentID " << contentID);

        const char* methodName = "handleForceStopOfferContent";
        if (!isAllowedToReceiveFrom(methodName, providerID))
            return;

        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleForceStopOfferContent: received from " << providerID << " for unknown content " << contentID);
            return;
        }
        if (ci->state == ContentState::Unknown)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleForceStopOfferContent: received from " << providerID << " not proving contentID " << contentID);
            return;
        }
        if (ci->providerID != providerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleForceStopOfferContent: received from " << providerID << " for content " << contentID << " but registered by remote " << ci->providerID);
            return;
        }

        if (m_localConsumerAvailable)
        {
            addConsumerEvent_ForceStopOfferContent(contentID, providerID);
        }

        ci->state = ContentState::Unknown;
    }

    bool DcsmComponent::dispatchProviderEvents(IDcsmProviderEventHandler& handler)
    {
        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::dispatchProviderEvents: called without active local provider");
            return false;
        }

        std::vector<DcsmEvent> events;
        {
            PlatformGuard guard(m_frameworkLock);
            m_providerEvents.swap(events);
        }

        for (const auto& ev : events)
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::dispatchProviderEvents: " << EnumToString(ev.cmdType));

            switch (ev.cmdType)
            {
            case EDcsmCommandType::CanvasSizeChange:
                handler.contentSizeChange(ramses::ContentID(ev.contentID.getValue()),
                                         ramses::SizeInfo{ev.size.width, ev.size.height},
                                         ramses::AnimationInformation{ev.animation.startTimeStamp, ev.animation.finishedTimeStamp});
                break;

            case EDcsmCommandType::ContentStateChange:
                handler.contentStateChange(ramses::ContentID(ev.contentID.getValue()),
                                           ev.state,
                                           ramses::SizeInfo{ev.size.width, ev.size.height},
                                           ramses::AnimationInformation{ev.animation.startTimeStamp, ev.animation.finishedTimeStamp});
                break;

            default:
                assert(false && "Wrong DcsmCommandType");
            }
        }
        return true;
    }

    bool DcsmComponent::dispatchConsumerEvents(ramses::IDcsmConsumerEventHandler& handler)
    {
        if (!m_localConsumerAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::dispatchConsumerEvents: called without active local consumer");
            return false;
        }

        std::vector<DcsmEvent> events;
        {
            PlatformGuard guard(m_frameworkLock);
            m_consumerEvents.swap(events);
        }

        for (const auto& ev : events)
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::dispatchConsumerEvents: " << EnumToString(ev.cmdType));

            switch (ev.cmdType)
            {
            case EDcsmCommandType::OfferContent:
                handler.contentOffered(ramses::ContentID(ev.contentID.getValue()),
                                        ramses::Category(ev.category.getValue()));
                break;

            case EDcsmCommandType::ContentReady:
                handler.contentReady(ramses::ContentID(ev.contentID.getValue()),
                                         ramses::ETechnicalContentType(ev.contentType),
                                         ramses::TechnicalContentDescriptor(ev.descriptor.getValue()));
                break;

            case EDcsmCommandType::ContentFocusRequest:
                handler.contentFocusRequest(ramses::ContentID(ev.contentID.getValue()));
                break;

            case EDcsmCommandType::StopOfferContentRequest:
                handler.contentStopOfferRequest(ramses::ContentID(ev.contentID.getValue()));
                break;

            case EDcsmCommandType::ForceStopOfferContent:
                handler.forceContentOfferStopped(ramses::ContentID(ev.contentID.getValue()));
                break;

            default:
                assert(false && "Wrong DcsmCommandType");
            }
        }
        return true;
    }

    Guid DcsmComponent::getContentProviderID(ContentID content) const
    {
        const ContentInfo* ci = m_contentRegistry.get(content);
        if (!ci)
            return Guid(false);
        return ci->providerID;
    }

    Guid DcsmComponent::getContentConsumerID(ContentID content) const
    {
        const ContentInfo* ci = m_contentRegistry.get(content);
        if (!ci)
            return Guid(false);
        return ci->consumerID;
    }

    DcsmComponent::ContentState DcsmComponent::getContentState(ContentID content) const
    {
        const ContentInfo* ci = m_contentRegistry.get(content);
        if (!ci)
            return ContentState::Unknown;
        return ci->state;
    }

    bool DcsmComponent::isAllowedToSendTo(const char* callerMethod, const Guid& id) const
    {
        if (id.isInvalid())
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": Tried to send to invalid participant id");
            return false;
        }
        if (id == m_myID)
        {
            return true;
        }
        if (!m_connectedParticipants.hasElement(id))
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": Tried to send to disconnected participant " << id);
            return false;
        }
        return true;
    }

    bool DcsmComponent::isAllowedToReceiveFrom(const char* callerMethod, const Guid& id) const
    {
        if (id.isInvalid())
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": Receive from invalid participant id");
            return false;
        }
        if (id == m_myID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": Received over network from self");
            return false;
        }
        if (!m_connectedParticipants.hasElement(id))
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": Received from disconnected participant " << id);
            return false;
        }
        return true;
    }

    bool DcsmComponent::isLocallyProvidingContent(const char* callerMethod, ContentID content) const
    {
        const ContentInfo* ci = m_contentRegistry.get(content);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": Unknown contentID " << content);
            return false;
        }
        if (ci->state == ContentState::Unknown)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": Local provider not providing contentID " << content);
            return false;
        }
        if (ci->providerID != m_myID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": Local provider cannot control contentID " << content << " provided by " << ci->providerID);
            return false;
        }
        return true;
    }

    bool DcsmComponent::isValidContent(const char* callerMethod, ContentID content) const
    {
        const ContentInfo* ci = m_contentRegistry.get(content);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": Unknown contentID " << content);
            return false;
        }
        if (ci->state == ContentState::Unknown)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": contentID " << content << " has no provider, previously provided by " << ci->providerID);
            return false;
        }
        return true;
    }

    bool DcsmComponent::isValidStateTransition(const char* callerMethod, const ContentInfo& ci, EDcsmState transition, ContentState& newState) const
    {
        newState = ci.state;
        if (ci.state == ContentState::Offered)
        {
            if (transition == EDcsmState::Assigned)
            {
                newState = ContentState::Assigned;
                return true;
            }
        }
        else if (ci.state == ContentState::Assigned)
        {
            if (transition == EDcsmState::Offered)
            {
                newState = ContentState::Offered;
                return true;
            }
            if (transition == EDcsmState::Ready)
            {
                newState = ContentState::ReadyRequested;
                return true;
            }
        }
        else if (ci.state == ContentState::ReadyRequested)
        {
            if (transition == EDcsmState::Offered)
            {
                newState = ContentState::Offered;
                return true;
            }
            if (transition == EDcsmState::Assigned)
            {
                newState = ContentState::Assigned;
                return true;
            }
        }
        else if (ci.state == ContentState::Ready)
        {
            if (transition == EDcsmState::Offered)
            {
                newState = ContentState::Offered;
                return true;
            }
            if (transition == EDcsmState::Assigned)
            {
                newState = ContentState::Assigned;
                return true;
            }
            if (transition == EDcsmState::Shown)
            {
                newState = ContentState::Shown;
                return true;
            }
        }
        else if (ci.state == ContentState::Shown)
        {
            if (transition == EDcsmState::Offered)
            {
                newState = ContentState::Offered;
                return true;
            }
            if (transition == EDcsmState::Assigned)
            {
                newState = ContentState::Assigned;
                return true;
            }
            if (transition == EDcsmState::Ready)
            {
                newState = ContentState::Ready;
                return true;
            }
        }
        else if (ci.state == ContentState::StopOfferRequested)
        {
            if (transition == EDcsmState::AcceptStopOffer)
            {
                newState = ContentState::Unknown;
                return true;
            }
        }
        assert(ci.state == newState);
        LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": invalid state transiton from " << EnumToString(ci.state) << " with " << ramses_internal::EnumToString(transition));
        return false;
    }

    bool DcsmComponent::isValidETechnicalContentType(const char* callerMethod, ETechnicalContentType val) const
    {
        switch (val)
        {
        case ETechnicalContentType::RamsesSceneID: return true;
        };
        LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": invalid technicalContentType " << static_cast<std::underlying_type_t<ETechnicalContentType>>(val));
        return false;
    }

    bool DcsmComponent::isValidEDcsmState(const char* callerMethod, EDcsmState val) const
    {
        switch (val)
        {
        case EDcsmState::Offered: return true;
        case EDcsmState::Assigned: return true;
        case EDcsmState::Ready: return true;
        case EDcsmState::Shown: return true;
        case EDcsmState::AcceptStopOffer: return true;
        };
        LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": invalid technicalContentType " << static_cast<std::underlying_type_t<EDcsmState>>(val));
        return false;
    }

    void DcsmComponent::logInfo()
    {
        PlatformGuard guard(m_frameworkLock);
        LOG_INFO_F(CONTEXT_DCSM, ([&](StringOutputStream& sos) {
            sos << "DcsmComponent(" << m_myID << ")::logInfo:\n"
                << "  LocalProvider " << m_localProviderAvailable << " (events pending " << m_consumerEvents.size() << ")\n"
                << "  LocalConsumer " << m_localConsumerAvailable << " (events pending " << m_providerEvents.size() << ")\n"
                << "  Connected participants:\n";
            for (const auto& p : m_connectedParticipants)
                sos << "  - " << p << "\n";
            sos << "  Known content:\n";
            for (const auto& p : m_contentRegistry)
            {
                const auto& ci = p.value;
                sos << "  - ContentID " << ci.content << "\n"
                    << "    Category  " << ci.category << "\n"
                    << "    State     " << EnumToString(ci.state) << "\n"
                    << "    Provider  " << ci.providerID << "\n"
                    << "    Consumer  " << ci.consumerID << "\n";
            }
        }));
    }

    void DcsmComponent::triggerLogMessageForPeriodicLog()
    {
        PlatformGuard guard(m_frameworkLock);
        LOG_INFO_F(CONTEXT_DCSM, ([&](StringOutputStream& sos) {
            sos << "Dcsm(" << m_myID << ") LP:" << m_localProviderAvailable << " LC:" << m_localConsumerAvailable
                << " CP[";
            for (const auto& p : m_connectedParticipants)
                sos << p << ";";
            sos << "] C[";
            for (const auto& p : m_contentRegistry)
            {
                const auto& ci = p.value;
                sos << ci.content << "," << ci.category << "," << EnumToString(ci.state);
                if (!ci.providerID.isInvalid())
                    sos << "," << ci.providerID;
                else
                    sos << ",-";
                if (!ci.consumerID.isInvalid())
                    sos << "," << ci.consumerID;
                else
                    sos << ",-";
                sos << ";";
            }
            sos << "]";
        }));
    }
}
