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
                LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalConsumerAvailability: local RegisterContent content " << ci.value.content << ", category " << ci.value.category << ", from " << ci.value.providerID);
                addConsumerEvent_RegisterContent(ci.value.content, ci.value.category, ci.value.providerID);
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
                    if (ci.value.unregisterRequested)
                    {
                        if (ci.value.providerID == m_myID)
                        {
                            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalConsumerAvailability: Queue unregister state and remove locally provided content " << ci.key << " because was requested and consumer now gone");
                            addProviderEvent_ContentStatusChange(ci.value.content, EDcsmStatus::Unregistered, {0, 0}, m_myID);
                        }
                        else
                        {
                            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalConsumerAvailability: Send unregister state and remove remotely provided content " << ci.key << " because was requested and consumer now gone");
                            m_communicationSystem.sendDcsmContentStatusChange(ci.value.providerID, ci.value.content, EDcsmStatus::Unregistered, {0, 0});
                        }
                        contentToRemove.push_back(ci.key);
                    }
                    else if (ci.value.consumedStatus != EDcsmStatus::Registered)
                    {
                        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalConsumerAvailability: Send contentStatusChange to " << ci.value.providerID << " for locally consumed content " << ci.value.content <<
                                 ", status " << ramses_internal::EnumToString(ci.value.consumedStatus) << " -> Registered");

                        if (ci.value.providerID == m_myID)
                        {
                            addProviderEvent_ContentStatusChange(ci.value.content, EDcsmStatus::Registered, {0, 0}, m_myID);
                        }
                        else
                        {
                            m_communicationSystem.sendDcsmContentStatusChange(ci.value.providerID, ci.value.content, EDcsmStatus::Registered, {0, 0});
                        }
                        ci.value.consumedStatus = EDcsmStatus::Registered;
                        ci.value.consumerID = Guid(false);
                    }
                }
            }

            // remove content that provider wanted to be gone
            for (const auto& cid : contentToRemove)
                m_contentRegistry.remove(cid);

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

        if (!m_localProviderAvailable)
        {
            m_providerEvents.clear();

            // send force unregister for all contents registered locally
            std::vector<ContentID> contentToRemove;
            for (const auto& ci : m_contentRegistry)
            {
                if (ci.value.providerID == m_myID)
                {
                    LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalProviderAvailability: ForceUnregisterContent for local content " << ci.value.content);

                    m_communicationSystem.sendDcsmBroadcastRequestUnregisterContent(ci.value.content);
                    if (m_localConsumerAvailable)
                        addConsumerEvent_ForceUnregisterContent(ci.value.content, m_myID);

                    contentToRemove.push_back(ci.key);
                }
            }

            // remove from tracked registered
            for (const auto& cid : contentToRemove)
                m_contentRegistry.remove(cid);
        }
        else
        {
            m_providerEvents.clear();
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
                     << ", send RegisterContent anyway");
        }
        m_connectedParticipants.put(newParticipant);

        // send own content to new participant
        for (auto& ci : m_contentRegistry)
            if (ci.value.providerID == m_myID)
                m_communicationSystem.sendDcsmRegisterContent(newParticipant, ci.value.content, ci.value.category);
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

        // unregister locally
        if (m_localConsumerAvailable)
        {
            // force unregister content from disappeared participant from local consumer
            for (const auto& ci : m_contentRegistry)
                if (ci.value.providerID == from)
                {
                    LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::participantHasDisconnected: ForceUnregisterContent locally " << ci.value.content);
                    addConsumerEvent_ForceUnregisterContent(ci.value.content, ci.value.providerID);
                }
        }

        if (m_localProviderAvailable)
        {
            // handle content locally provided and consumed by disconnecting remote
            std::vector<ContentID> localContentToRemove;
            for (auto& ci : m_contentRegistry)
            {
                if (ci.value.providerID == m_myID && ci.value.consumerID == from)
                {
                    if (ci.value.unregisterRequested)
                    {
                        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::participantHasDisconnected: Queue unregister state and remove locally provided content " << ci.key << " because was requested and consumer now gone");
                        addProviderEvent_ContentStatusChange(ci.value.content, EDcsmStatus::Unregistered, {0, 0}, m_myID);
                        localContentToRemove.push_back(ci.key);
                    }
                    else if (ci.value.consumedStatus != EDcsmStatus::Registered)
                    {
                        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::participantHasDisconnected: Queue contentStatusChange for locally provided and remotely consumed content " << ci.value.content <<
                                 ", status " << ramses_internal::EnumToString(ci.value.consumedStatus) << " -> Registered");

                        addProviderEvent_ContentStatusChange(ci.value.content, EDcsmStatus::Registered, {0, 0}, m_myID);
                        ci.value.consumedStatus = EDcsmStatus::Registered;
                        ci.value.consumerID = Guid(false);
                    }
                }
            }

            // remove locally provided content
            for (const auto cid : localContentToRemove)
                m_contentRegistry.remove(cid);
        }

        // remove everything from disconnecting provider
        std::vector<ContentID> contentToRemove;
        for (const auto& ci : m_contentRegistry)
        {
            if (ci.value.providerID == from)
                contentToRemove.push_back(ci.key);
        }
        for (const auto cid : contentToRemove)
        {
            m_contentRegistry.remove(cid);
        }

        m_connectedParticipants.remove(from);
    }

    bool DcsmComponent::sendRegisterContent(ContentID contentID, Category category)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRegisterContent: ContentID " << contentID << ", Category " << category);

        PlatformGuard guard(m_frameworkLock);
        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRegisterContent: called without local provider active");
            return false;
        }

        if (const ContentInfo* ci = m_contentRegistry.get(contentID))
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRegisterContent: tried to register content " << contentID << " already provided by " << ci->providerID);
            return false;
        }

        if (m_localConsumerAvailable)
        {
            addConsumerEvent_RegisterContent(contentID, category, m_myID);
        }
        m_communicationSystem.sendDcsmBroadcastRegisterContent(contentID, category);

        m_contentRegistry.put(contentID, ContentInfo{contentID, category, m_myID, false, Guid(false), EDcsmStatus::Registered});
        return true;
    }

    bool DcsmComponent::sendContentAvailable(const Guid& to, ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentAvailable: to " << to << ", ContentID " << contentID <<
                 ", ETechnicalContentType " << ramses_internal::EnumToString(technicalContentType) << ", TechnicalContentDescriptor " << technicalContentDescriptor);

        PlatformGuard guard(m_frameworkLock);
        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentAvailable: called without local provider active");
            return false;
        }
        const char* methodName = "sendContentAvailable";
        if (!isAllowedToSendTo(methodName, to) ||
            !isValidETechnicalContentType(methodName, technicalContentType) ||
            !isLocallyProvidingContent(methodName, contentID))
            return false;

        if (m_myID == to)
        {
            if (m_localConsumerAvailable)
            {
                addConsumerEvent_ContentAvailable(contentID, technicalContentType, technicalContentDescriptor, m_myID);
            }
            else
            {
                LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentAvailable: sent to non-existing local consumer");
                return false;
            }
        }
        else
        {
            m_communicationSystem.sendDcsmContentAvailable(to, contentID, technicalContentType, technicalContentDescriptor);
        }
        return true;
    }

    bool DcsmComponent::sendCategoryContentSwitchRequest(const Guid& to, ContentID contentID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendCategoryContentSwitchRequest: to " << to << ", ContentID " << contentID);

        PlatformGuard guard(m_frameworkLock);
        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendCategoryContentSwitchRequest: called without local provider active");
            return false;
        }
        const char* methodName = "sendCategoryContentSwitchRequest";
        if (!isAllowedToSendTo(methodName, to) ||
            !isLocallyProvidingContent(methodName, contentID))
            return false;

        if (m_myID == to)
        {
            if (m_localConsumerAvailable)
            {
                addConsumerEvent_CategoryContentSwitchRequest(contentID, m_myID);
            }
            else
            {
                LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendCategoryContentSwitchRequest: sent to non-existing local consumer");
                return false;
            }
        }
        else
        {
            m_communicationSystem.sendDcsmCategoryContentSwitchRequest(to, contentID);
        }
        return true;
    }

    bool DcsmComponent::sendRequestUnregisterContent(ContentID contentID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRequestUnregisterContent: ContentID " << contentID);

        PlatformGuard guard(m_frameworkLock);
        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRequestUnregisterContent: called without local provider active");
            return false;
        }

        const char* methodName = "sendRequestUnregisterContent";
        if (!isLocallyProvidingContent(methodName, contentID))
            return false;

        ContentInfo& ci = m_contentRegistry[contentID];
        if (ci.unregisterRequested)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRequestUnregisterContent: called more than once for content " << contentID);
            return false;
        }

        if (m_localConsumerAvailable)
        {
            addConsumerEvent_RequestUnregisterContent(contentID, m_myID);
        }
        m_communicationSystem.sendDcsmBroadcastRequestUnregisterContent(contentID);

        ci.unregisterRequested = true;
        if (ci.consumerID.isInvalid())
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRequestUnregisterContent: remove content " << contentID <<
                     " because no active consumer, state " << ramses_internal::EnumToString(ci.consumedStatus));
            assert(ci.consumedStatus == EDcsmStatus::Registered);
            m_contentRegistry.remove(contentID);
        }
        else
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRequestUnregisterContent: keep content " << contentID <<
                     " because consumed by " << ci.consumerID << ", state "  << ramses_internal::EnumToString(ci.consumedStatus));
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
        if (!isAllowedToSendTo(methodName, to) ||
            !isContentKnown(methodName, contentID))
            return false;

        ContentInfo& ci = m_contentRegistry[contentID];
        if (ci.providerID != to)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendCanvasSizeChange: cannot control content " << contentID << " from remote " << ci.providerID << " by sending to " << to);
            return false;
        }

        if (m_myID == to)
        {
            if (m_localProviderAvailable)
            {
                addProviderEvent_CanvasSizeChange(contentID, sizeInfo, ai, m_myID);
            }
            else
            {
                // TODO(tobias): check if can happen with providerid check before and autoremove if provider gone
                LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendCanvasSizeChange: sent to non-existing local provider");
                return false;
            }
        }
        else
        {
            m_communicationSystem.sendDcsmCanvasSizeChange(to, contentID, sizeInfo, ai);
        }
        return true;
    }

    bool DcsmComponent::sendContentStatusChange(ContentID contentID, EDcsmStatus status, AnimationInformation ai)
    {
        PlatformGuard guard(m_frameworkLock);

        const Guid& to = getContentProviderID(contentID);
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStatusChange: to " << to << ", ContentID " << contentID <<
                 ", EDcsmStatus " << ramses_internal::EnumToString(status) << ", ai [" << ai.startTimeStamp << ", " << ai.finishedTimeStamp << "]");

        if (!m_localConsumerAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStatusChange: called without local consumer active");
            return false;
        }
        const char* methodName = "sendContentStatusChange";
        if (!isAllowedToSendTo(methodName, to) ||
            !isValidEDcsmStatus(methodName, status) ||
            !isContentKnown(methodName, contentID))
            return false;

        // TODO(tobias): limit only one consumer can control content at a time, track active consumer + state

        ContentInfo& ci = m_contentRegistry[contentID];
        if (ci.providerID != to)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStatusChange: cannot control content " << contentID << " from remote " << ci.providerID << " by sending to " << to);
            return false;
        }
        if (!ci.consumerID.isInvalid() && ci.consumerID != m_myID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStatusChange: cannot control content " << contentID << " consumed by " << ci.consumerID);
            return false;
        }

        if (m_myID == to)
        {
            if (m_localProviderAvailable)
            {
                addProviderEvent_ContentStatusChange(contentID, status, ai, m_myID);
            }
            else
            {
                // TODO(tobias): check if can happen with providerid check before and autoremove if provider gone
                LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStatusChange: sent to non-existing local provider");
                return false;
            }
        }
        else
        {
            m_communicationSystem.sendDcsmContentStatusChange(to, contentID, status, ai);
        }

        ci.consumedStatus = status;
        if (status == EDcsmStatus::Registered)
        {
            ci.consumerID = Guid(false);
        }
        else
        {
            ci.consumerID = m_myID;
        }

        if (ci.unregisterRequested &&
            (ci.consumedStatus == EDcsmStatus::Unregistered || ci.consumedStatus == EDcsmStatus::Registered))
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStatusChange: remove content " << contentID <<
                     " because unregister requested and in state " << ramses_internal::EnumToString(ci.consumedStatus));
            m_contentRegistry.remove(contentID);
        }
        else
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStatusChange: keep content " << contentID <<
                     " because unregister requested " << ci.unregisterRequested << " and state " << ramses_internal::EnumToString(ci.consumedStatus));
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

    void DcsmComponent::addProviderEvent_ContentStatusChange(ContentID contentID, EDcsmStatus status, AnimationInformation ai, const Guid& consumerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::ContentStatusChange;
        event.contentID = contentID;
        event.status    = status;
        event.animation = ai;
        event.from      = consumerID;
        m_providerEvents.push_back(event);
    }

    void DcsmComponent::addConsumerEvent_RegisterContent(ContentID contentID, Category category, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::RegisterContent;
        event.contentID = contentID;
        event.category  = category;
        event.from      = providerID;
        m_consumerEvents.push_back(event);
    }

    void DcsmComponent::addConsumerEvent_ContentAvailable(ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType     = EDcsmCommandType::ContentAvailable;
        event.contentID   = contentID;
        event.contentType = technicalContentType;
        event.descriptor  = technicalContentDescriptor;
        event.from        = providerID;
        m_consumerEvents.push_back(event);
    }

    void DcsmComponent::addConsumerEvent_CategoryContentSwitchRequest(ContentID contentID, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::CategoryContentSwitchRequest;
        event.contentID = contentID;
        event.from      = providerID;
        m_consumerEvents.push_back(event);
    }

    void DcsmComponent::addConsumerEvent_RequestUnregisterContent(ContentID contentID, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::UnregisterContent;
        event.contentID = contentID;
        event.from      = providerID;
        m_consumerEvents.push_back(event);
    }

    void DcsmComponent::addConsumerEvent_ForceUnregisterContent(ContentID contentID, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::ForceUnregisterContent;
        event.contentID = contentID;
        event.from      = providerID;
        m_consumerEvents.push_back(event);
    }

    const char* DcsmComponent::EnumToString(EDcsmCommandType cmd) const
    {
        switch (cmd)
        {
        case EDcsmCommandType::RegisterContent:
            return "EDcsmCommandType::RegisterContent";
        case EDcsmCommandType::ContentAvailable:
            return "EDcsmCommandType::ContentAvailable";
        case EDcsmCommandType::ContentStatusChange:
            return "EDcsmCommandType::ContentStatusChange";
        case EDcsmCommandType::CanvasSizeChange:
            return "EDcsmCommandType::CanvasSizeChange";
        case EDcsmCommandType::CategoryContentSwitchRequest:
            return "EDcsmCommandType::CategoryContentSwitchRequest";
        case EDcsmCommandType::UnregisterContent:
            return "EDcsmCommandType::UnregisterContent";
        case EDcsmCommandType::ForceUnregisterContent:
            return "EDcsmCommandType::ForceUnregisterContent";
        }
        return "<Invalid EDcsmCommandType>";
    }

    void DcsmComponent::handleCanvasSizeChange(ContentID contentID, SizeInfo sizeInfo, AnimationInformation ai, const Guid& consumerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleCanvasSizeChange: from " << consumerID << ", ContentID " << contentID <<
                 ", SizeInfo " << sizeInfo.width << "x" << sizeInfo.height << ", ai [" << ai.startTimeStamp << ", " << ai.finishedTimeStamp << "]");

        const char* methodName = "handleCanvasSizeChange";
        if (!isAllowedToReceiveFrom(methodName, consumerID))
            return;

        // TODO(tobias): add error return channel and use on all false exits here

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

        addProviderEvent_CanvasSizeChange(contentID, sizeInfo, ai, consumerID);
    }

    void DcsmComponent::handleContentStatusChange(ContentID contentID, EDcsmStatus status, AnimationInformation ai, const Guid& consumerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStatusChange: from " << consumerID << ", ContentID " << contentID <<
                 ", EDcsmStatus " << ramses_internal::EnumToString(status) << ", ai [" << ai.startTimeStamp << ", " << ai.finishedTimeStamp << "]");

        const char* methodName = "handleContentStatusChange";
        if (!isAllowedToReceiveFrom(methodName, consumerID) ||
            !isValidEDcsmStatus(methodName, status))
            return;

        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStatusChange: sent to non-existing local provider");
            return;
        }
        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStatusChange: received from " << consumerID << " for unknown content " << contentID);
            return;
        }
        if (ci->providerID != m_myID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStatusChange: received from " << consumerID << " for content " << contentID << " from remote " << ci->providerID);
            return;
        }
        if (!ci->consumerID.isInvalid() && ci->consumerID != consumerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStatusChange: cannot control content " << contentID << " consumed by " << ci->consumerID);
            return;
        }

        addProviderEvent_ContentStatusChange(contentID, status, ai, consumerID);

        ci->consumedStatus = status;
        if (status == EDcsmStatus::Registered)
        {
            ci->consumerID = Guid(false);
        }
        else
        {
            ci->consumerID = consumerID;
        }


        if (ci->unregisterRequested &&
            (ci->consumedStatus == EDcsmStatus::Unregistered || ci->consumedStatus == EDcsmStatus::Registered))
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStatusChange: remove content " << contentID <<
                     " because unregister requested and in state " << ramses_internal::EnumToString(ci->consumedStatus));
            m_contentRegistry.remove(contentID);
        }
        else
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStatusChange: keep content " << contentID <<
                     " because unregister requested " << ci->unregisterRequested << " and state " << ramses_internal::EnumToString(ci->consumedStatus));
        }
    }

    void DcsmComponent::handleRegisterContent(ContentID contentID, Category category, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRegisterContent: from " << providerID << ", ContentID " << contentID << ", Category " << category << ", hasLocalConsumer " << m_localConsumerAvailable);

        const char* methodName = "handleRegisterContent";
        if (!isAllowedToReceiveFrom(methodName, providerID))
            return;

        if (const ContentInfo* ci = m_contentRegistry.get(contentID))
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRegisterContent: provider " << ci->providerID << " already registered content " << contentID);
            return;
        }

        if (m_localConsumerAvailable)
        {
            addConsumerEvent_RegisterContent(contentID, category, providerID);
        }

        m_contentRegistry.put(contentID, ContentInfo{contentID, category, providerID, false, Guid(false), EDcsmStatus::Registered});
    }

    void DcsmComponent::handleContentAvailable(ContentID contentID, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentAvailable: from " << providerID << ", ContentID " << contentID <<
                 ", ETechnicalContentType " << ramses_internal::EnumToString(technicalContentType) << ", TechnicalContentDescriptor " << technicalContentDescriptor);

        const char* methodName = "handleContentAvailable";
        if (!isAllowedToReceiveFrom(methodName, providerID) ||
            !isValidETechnicalContentType(methodName, technicalContentType))
            return;

        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentAvailable: received from " << providerID << " for unknown content " << contentID);
            return;
        }
        if (ci->providerID != providerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentAvailable: received from " << providerID << " for content " << contentID << " but registered by remote " << ci->providerID);
            return;
        }
        if (!m_localConsumerAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentAvailable: sent to non-existing local consumer");
            return;
        }

        addConsumerEvent_ContentAvailable(contentID, technicalContentType, technicalContentDescriptor, providerID);
    }

    void DcsmComponent::handleCategoryContentSwitchRequest(ContentID contentID, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleCategoryContentSwitchRequest: from " << providerID << ", ContentID " << contentID);

        const char* methodName = "handleCategoryContentSwitchRequest";
        if (!isAllowedToReceiveFrom(methodName, providerID))
            return;

        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleCategoryContentSwitchRequest: received from " << providerID << " for unknown content " << contentID);
            return;
        }
        if (ci->providerID != providerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleCategoryContentSwitchRequest: received from " << providerID << " for content " << contentID << " but registered by remote " << ci->providerID);
            return;
        }
        if (!m_localConsumerAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleCategoryContentSwitchRequest: sent to non-existing local consumer");
            return;
        }

        addConsumerEvent_CategoryContentSwitchRequest(contentID, providerID);
    }

    void DcsmComponent::handleRequestUnregisterContent(ContentID contentID, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRequestUnregisterContent: from " << providerID << ", ContentID " << contentID << ", hasLocalConsumer " << m_localConsumerAvailable);

        const char* methodName = "handleRequestUnregisterContent";
        if (!isAllowedToReceiveFrom(methodName, providerID))
            return;

        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRequestUnregisterContent: received from " << providerID << " for unknown content " << contentID);
            return;
        }
        if (ci->providerID != providerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRequestUnregisterContent: received from " << providerID << " for content " << contentID << " but registered by remote " << ci->providerID);
            return;
        }
        if (ci->unregisterRequested)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRequestUnregisterContent: called more than once for content " << contentID << " from " << providerID);
            return;
        }

        if (m_localConsumerAvailable)
        {
            addConsumerEvent_RequestUnregisterContent(contentID, providerID);
        }

        ci->unregisterRequested = true;
        if (ci->consumerID.isInvalid())
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRequestUnregisterContent: remove content " << contentID <<
                     " because no active consumer, state " << ramses_internal::EnumToString(ci->consumedStatus));
            assert(ci->consumedStatus == EDcsmStatus::Registered);
            m_contentRegistry.remove(contentID);
        }
        else
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleRequestUnregisterContent: keep content " << contentID <<
                     " because consumed by " << ci->consumerID << ", state "  << ramses_internal::EnumToString(ci->consumedStatus));
        }
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
                handler.canvasSizeChange(ramses::ContentID(ev.contentID.getValue()),
                                         ramses::SizeInfo{ev.size.width, ev.size.height},
                                         ramses::AnimationInformation{ev.animation.startTimeStamp, ev.animation.finishedTimeStamp},
                                         ev.from);
                break;

            case EDcsmCommandType::ContentStatusChange:
                handler.contentStatusChange(ramses::ContentID(ev.contentID.getValue()),
                                            ramses::EDcsmStatus(ev.status),
                                            ramses::AnimationInformation{ev.animation.startTimeStamp, ev.animation.finishedTimeStamp},
                                            ev.from);
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
            case EDcsmCommandType::RegisterContent:
                handler.registerContent(ramses::ContentID(ev.contentID.getValue()),
                                        ramses::Category(ev.category.getValue()));
                break;

            case EDcsmCommandType::ContentAvailable:
                handler.contentAvailable(ramses::ContentID(ev.contentID.getValue()),
                                         ramses::ETechnicalContentType(ev.contentType),
                                         ramses::TechnicalContentDescriptor(ev.descriptor.getValue()));
                break;

            case EDcsmCommandType::CategoryContentSwitchRequest:
                handler.categoryContentSwitchRequest(ramses::ContentID(ev.contentID.getValue()));
                break;

            case EDcsmCommandType::UnregisterContent:
                handler.requestUnregisterContent(ramses::ContentID(ev.contentID.getValue()));
                break;

            case EDcsmCommandType::ForceUnregisterContent:
                handler.forceUnregisterContent(ramses::ContentID(ev.contentID.getValue()));
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
        if (ci->providerID != m_myID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": Local provider cannot control contentID " << content << " provided by " << ci->providerID);
            return false;
        }
        return true;
    }

    bool DcsmComponent::isContentKnown(const char* callerMethod, ContentID content) const
    {
        const ContentInfo* ci = m_contentRegistry.get(content);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": Unknown contentID " << content);
            return false;
        }
        return true;
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

    bool DcsmComponent::isValidEDcsmStatus(const char* callerMethod, EDcsmStatus val) const
    {
        switch (val)
        {
        case EDcsmStatus::Unregistered: return true;
        case EDcsmStatus::Registered: return true;
        case EDcsmStatus::ContentRequested: return true;
        case EDcsmStatus::Shown: return true;
        };
        LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": invalid technicalContentType " << static_cast<std::underlying_type_t<EDcsmStatus>>(val));
        return false;
    }
}
