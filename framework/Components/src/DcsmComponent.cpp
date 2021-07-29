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
#include "Utils/LogMacros.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"
#include "ramses-framework-api/DcsmMetadataUpdate.h"
#include "DcsmMetadataUpdateImpl.h"
#include "CategoryInfoUpdateImpl.h"
#include <chrono>

// ensure internal and api types match
static_assert(std::is_same<ramses::ContentID::BaseType, ramses_internal::ContentID::BaseType>::value, "ContentID type mismatch");
static_assert(std::is_same<ramses::Category::BaseType, ramses_internal::Category::BaseType>::value, "Category type mismatch");
static_assert(std::is_same<ramses::TechnicalContentDescriptor::BaseType, ramses_internal::TechnicalContentDescriptor::BaseType>::value, "TechnicalContentDescriptor type mismatch");

static_assert(ramses::ContentID::Invalid().getValue() == ramses_internal::ContentID::Invalid().getValue(), "ContentID default mismatch");
static_assert(ramses::Category::Invalid().getValue() == ramses_internal::Category::Invalid().getValue(), "Category default mismatch");
static_assert(ramses::TechnicalContentDescriptor::Invalid().getValue() == ramses_internal::TechnicalContentDescriptor::Invalid().getValue(), "TechnicalContentDescriptor default mismatch");


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

    bool DcsmComponent::connect()
    {
        PlatformGuard guard(m_frameworkLock);
        if (m_connected)
        {
            LOG_ERROR(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::connect: Already connected");
            return false;
        }

        m_connected = true;
        return true;
    }

    bool DcsmComponent::disconnect()
    {
        PlatformGuard guard(m_frameworkLock);
        if (!m_connected)
        {
            LOG_ERROR(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::disconnect: Not connected");
            return false;
        }

        // network broadcast force stop offer for all contents offered locally
        for (auto& ci : m_contentRegistry)
        {
            if (ci.value.state == ContentState::Unknown) // obsolete, unoffered contents - don't touch
                continue;

            if (ci.value.providerID != m_myID || ci.value.localOnly)
                continue;

            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::disconnect: network ForceStopOfferContent for local content " << ci.value.content << ", consumer " <<
                ci.value.consumerID << ", state " << EnumToString(ci.value.state));

            m_communicationSystem.sendDcsmBroadcastForceStopOfferContent(ci.value.content);

            if (!ci.value.consumerID.isValid() || ci.value.consumerID == m_myID) // no remote consumer, nothing else to do
                continue;

            ci.value.consumerID = Guid();
            if (ci.value.state == ContentState::StopOfferRequested)
            {
                ci.value.state = ContentState::Unknown;
                ci.value.contentDescriptor = TechnicalContentDescriptor::Invalid();
                ci.value.metadata = DcsmMetadata{};
                ci.value.m_currentFocusRequests.clear();
                addProviderEvent_ContentStateChange(ci.value.content, EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{ 0, 0 }, m_myID);
            }
            else
            {
                ci.value.state = ContentState::Offered;
                addProviderEvent_ContentStateChange(ci.value.content, EDcsmState::Offered, CategoryInfo{}, AnimationInformation{ 0, 0 }, m_myID);
            }
        }

        m_connected = false;
        return true;
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
                    addConsumerEvent_OfferContent(ci.value.content, ci.value.category, ci.value.contentType, ci.value.providerID);
                }
            }
        }
        else
        {
            // consumer became unavailable: send update for all locally consumed contents
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
                            addProviderEvent_ContentStateChange(ci.value.content, EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{0, 0}, m_myID);
                        }
                        else
                        {
                            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalConsumerAvailability: Send AcceptStopOffer and remove remotely provided content " << ci.key << " because was requested and consumer now gone");
                            m_communicationSystem.sendDcsmContentStateChange(ci.value.providerID, ci.value.content, EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{0, 0});
                        }
                        ci.value.state = ContentState::Unknown;
                        ci.value.contentDescriptor = TechnicalContentDescriptor::Invalid();
                        ci.value.metadata = DcsmMetadata{};
                        ci.value.m_currentFocusRequests.clear();
                    }
                    else if (ci.value.state != ContentState::Offered && ci.value.state != ContentState::Unknown)
                    {
                        // content was in use by local consumer, inform provider that not actively in use anymore
                        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalConsumerAvailability: Send contentStateChange to " << ci.value.providerID << " for locally consumed content " << ci.value.content <<
                                 ", status " << EnumToString(ci.value.state) << " -> Offered");
                        if (ci.value.providerID == m_myID)
                        {
                            addProviderEvent_ContentStateChange(ci.value.content, EDcsmState::Offered, CategoryInfo{}, AnimationInformation{0, 0}, m_myID);
                        }
                        else
                        {
                            m_communicationSystem.sendDcsmContentStateChange(ci.value.providerID, ci.value.content, EDcsmState::Offered, CategoryInfo{}, AnimationInformation{0, 0});
                        }
                        ci.value.state = ContentState::Offered;
                        ci.value.consumerID = Guid();
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
                if (ci.value.providerID == m_myID && ci.value.state != ContentState::Unknown)
                {
                    LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::setLocalProviderAvailability: ForceStopOfferContent for local content " << ci.value.content << ", consumer " <<
                             ci.value.consumerID << ", state " << EnumToString(ci.value.state));

                    if (m_connected && !ci.value.localOnly)
                        m_communicationSystem.sendDcsmBroadcastForceStopOfferContent(ci.value.content);
                    if (m_localConsumerAvailable)
                        addConsumerEvent_ForceStopOfferContent(ci.value.content, m_myID);

                    ci.value.state = ContentState::Unknown;
                    ci.value.contentDescriptor = TechnicalContentDescriptor::Invalid();
                    ci.value.metadata = DcsmMetadata{};
                    ci.value.m_currentFocusRequests.clear();
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

        if (m_connectedParticipants.contains(newParticipant))
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::newParticipantHasConnected: duplicate connect from " << newParticipant
                     << ", send offer anyway");
        }
        m_connectedParticipants.put(newParticipant);

        // send own content to new participant
        for (auto& ci : m_contentRegistry)
            if (ci.value.providerID == m_myID && ci.value.state != ContentState::Unknown && !ci.value.localOnly)
                m_communicationSystem.sendDcsmOfferContent(newParticipant, ci.value.content, ci.value.category, ci.value.contentType, ci.value.friendlyName);
    }

    void DcsmComponent::participantHasDisconnected(const Guid& from)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::participantHasDisconnected: " << from);

        if (from == m_myID)
        {
            LOG_ERROR(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::participantHasDisconnected: Ignore remote " << from << ", has same id as local");
            return;
        }

        if (!m_connectedParticipants.contains(from))
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::participantHasDisconnected: ignore from unknown participant " << from);
            return;
        }

        // stop offer locally
        if (m_localConsumerAvailable)
        {
            // force stop offer content from disappeared participant for local consumer
            for (const auto& ci : m_contentRegistry)
                if (ci.value.providerID == from && ci.value.state != ContentState::Unknown)
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
                        addProviderEvent_ContentStateChange(ci.value.content, EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{0, 0}, m_myID);
                        ci.value.state = ContentState::Unknown;
                        ci.value.contentDescriptor = TechnicalContentDescriptor::Invalid();
                        ci.value.metadata = DcsmMetadata{};
                        ci.value.m_currentFocusRequests.clear();
                    }
                    else if (ci.value.state != ContentState::Offered && ci.value.state != ContentState::Unknown)
                    {
                        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::participantHasDisconnected: Queue contentStateChange for locally provided and remotely consumed content " << ci.value.content <<
                                 ", status " << EnumToString(ci.value.state) << " -> Offered");

                        addProviderEvent_ContentStateChange(ci.value.content, EDcsmState::Offered, CategoryInfo{}, AnimationInformation{0, 0}, m_myID);
                        ci.value.state = ContentState::Offered;
                        ci.value.consumerID = Guid();
                    }
                }
            }
        }

        // set everything from disconnected provider to unknown
        for (auto& ci : m_contentRegistry)
        {
            if (ci.value.providerID == from)
            {
                ci.value.state = ContentState::Unknown;
                ci.value.contentDescriptor = TechnicalContentDescriptor::Invalid();
                ci.value.metadata = DcsmMetadata{};
                ci.value.m_currentFocusRequests.clear();
            }
        }

        m_connectedParticipants.remove(from);
    }

    bool DcsmComponent::sendOfferContent(ContentID contentID, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName, bool localOnly)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendOfferContent: ContentID " << contentID << ", Category " << category
            << ", ETechnicalContentType " << technicalContentType << ", Name '" << friendlyName << "'" << (localOnly ? " for local" : " for remote"));

        PlatformGuard guard(m_frameworkLock);
        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendOfferContent: called without local provider active");
            return false;
        }

        if (!contentID.isValid())
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendOfferContent: called with invalid ContentID");
            return false;
        }

        if (!category.isValid())
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendOfferContent: called with invalid Category");
            return false;
        }

        if (!isValidETechnicalContentType("sendOfferContent", technicalContentType))
            return false;

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
            addConsumerEvent_OfferContent(contentID, category, technicalContentType, m_myID);
        if (m_connected && !localOnly)
            m_communicationSystem.sendDcsmBroadcastOfferContent(contentID, category, technicalContentType, friendlyName);

        m_contentRegistry.put(contentID, ContentInfo{contentID, category, friendlyName, ContentState::Offered,
                              m_myID, Guid(), DcsmMetadata(), {}, localOnly,
                              technicalContentType, TechnicalContentDescriptor::Invalid()});
        return true;
    }

    bool DcsmComponent::sendContentDescription(ContentID contentID, TechnicalContentDescriptor technicalContentDescriptor)
    {
        PlatformGuard guard(m_frameworkLock);

        const Guid& to = getContentConsumerID(contentID);
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentDescription: to " << to << ", ContentID " << contentID <<
            ", TechnicalContentDescriptor " << technicalContentDescriptor);

        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentDescription: called without local provider active");
            return false;
        }

        const char* methodName = "sendContentDescription";
        if (!isLocallyProvidingContent(methodName, contentID))
            return false;

        ContentInfo& ci = m_contentRegistry[contentID];
        if (ci.state != ContentState::Offered &&
            ci.state != ContentState::Assigned &&
            ci.state != ContentState::ReadyRequested)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentDescription: called on content " << contentID << " in invalid state " << EnumToString(ci.state));
            return false;
        }

        if (!technicalContentDescriptor.isValid())
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentDescription: called with invalid TechnicalContentDescriptor");
            return false;
        }

        if (ci.contentDescriptor.isValid())
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentDescription: switch content " << contentID << " technicalContentDescriptor " << ci.contentDescriptor << " -> " << technicalContentDescriptor);

        ci.contentDescriptor = technicalContentDescriptor;

        if (ci.state == ContentState::Offered)
        {
            LOG_INFO_P(CONTEXT_DCSM, "DcsmComponent({})::sendContentDescription: skip sending because not in assigned state. Will send TechnicalContentDescriptor {} to assigning consumers.",
                m_myID, technicalContentDescriptor);
            return true;
        }

        if (!isAllowedToSendTo(methodName, to))
            return false;

        if (m_myID == to)
        {
            assert(m_localConsumerAvailable && "Logic error on consumer disable");
            addConsumerEvent_ContentDescription(contentID, technicalContentDescriptor, m_myID);
        }
        else
        {
            m_communicationSystem.sendDcsmContentDescription(to, contentID, technicalContentDescriptor);
        }

        return true;
    }

    bool DcsmComponent::sendContentReady(ContentID contentID)
    {
        PlatformGuard guard(m_frameworkLock);

        const Guid& to = getContentConsumerID(contentID);
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentReady: to " << to << ", ContentID " << contentID);

        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentReady: called without local provider active");
            return false;
        }
        const char* methodName = "sendContentReady";
        if (!isAllowedToSendTo(methodName, to))
            return false;
        if (!isLocallyProvidingContent(methodName, contentID))
            return false;

        ContentInfo& ci = m_contentRegistry[contentID];
        if (ci.state != ContentState::ReadyRequested)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentReady: called on content " << contentID << " in invalid state " << EnumToString(ci.state));
            return false;
        }

        if (!ci.contentDescriptor.isValid())
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentReady: called on content " << contentID << " without calling sendContentDescription first");
            return false;
        }

        if (m_myID == to)
        {
            assert(m_localConsumerAvailable && "Logic error on consumer disable");
            addConsumerEvent_ContentReady(contentID, m_myID);
        }
        else
        {
            m_communicationSystem.sendDcsmContentReady(to, contentID);
        }

        ci.state = ContentState::Ready;

        return true;
    }

    bool DcsmComponent::sendContentEnableFocusRequest(ContentID contentID, int32_t request)
    {
        PlatformGuard guard(m_frameworkLock);

        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentEnableFocusRequest:  ContentID " << contentID << ", request " << request);

        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentEnableFocusRequest: called without local provider active");
            return false;
        }
        const char* methodName = "sendContentEnableFocusRequest";
        if (!isLocallyProvidingContent(methodName, contentID))
            return false;

        ContentInfo& ci = m_contentRegistry[contentID];
        if (ci.m_currentFocusRequests.end() != std::find(ci.m_currentFocusRequests.begin(), ci.m_currentFocusRequests.end(), request) )
        {
            LOG_ERROR(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentEnableFocusRequest: called for already active request:" << request);
            return false;
        }

        ci.m_currentFocusRequests.push_back(request);
        if (ci.state == ContentState::Assigned ||
            ci.state == ContentState::ReadyRequested ||
            ci.state == ContentState::Ready ||
            ci.state == ContentState::Shown ||
            ci.state == ContentState::StopOfferRequested)
        {
            const Guid& to = getContentConsumerID(contentID);
            assert(to.isValid());

            if (m_myID == to)
            {
                assert(m_localConsumerAvailable && "Logic error on consumer disable");
                addConsumerEvent_ContentEnableFocusRequest(contentID, request, m_myID);
            }
            else
                m_communicationSystem.sendDcsmContentEnableFocusRequest(to, contentID, request);
        }
        return true;
    }

    bool DcsmComponent::sendContentDisableFocusRequest(ContentID contentID, int32_t request)
    {
        PlatformGuard guard(m_frameworkLock);

        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentDisableFocusRequest: ContentID " << contentID << ", request " << request);

        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentDisableFocusRequest: called without local provider active");
            return false;
        }
        const char* methodName = "sendContentDisableFocusRequest";
        if (!isLocallyProvidingContent(methodName, contentID))
            return false;

        ContentInfo& ci = m_contentRegistry[contentID];
        const auto& iterator = std::find(ci.m_currentFocusRequests.begin(), ci.m_currentFocusRequests.end(), request);
        const bool found = iterator != ci.m_currentFocusRequests.end();
        if (!found)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentDisableFocusRequest: request was not active");
            return false;
        }
        ci.m_currentFocusRequests.erase(iterator);
        if (ci.state == ContentState::Assigned ||
            ci.state == ContentState::ReadyRequested ||
            ci.state == ContentState::Ready ||
            ci.state == ContentState::Shown ||
            ci.state == ContentState::StopOfferRequested)
        {
            const Guid& to = getContentConsumerID(contentID);
            assert(to.isValid());
            if (m_myID == to)
            {
                assert(m_localConsumerAvailable && "Logic error on consumer disable");
                addConsumerEvent_ContentDisableFocusRequest(contentID, request, m_myID);
            }
            else
            {
                m_communicationSystem.sendDcsmContentDisableFocusRequest(to, contentID, request);
            }
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
            addConsumerEvent_RequestStopOfferContent(contentID, m_myID);
        if (m_connected && !ci.localOnly)
            m_communicationSystem.sendDcsmBroadcastRequestStopOfferContent(contentID);

        if (ci.state == ContentState::Offered)
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRequestStopOfferContent: set content " << contentID <<
                     " state Unknown because no active consumer");
            assert(!ci.consumerID.isValid());
            ci.state = ContentState::Unknown;
            ci.contentDescriptor = TechnicalContentDescriptor::Invalid();
            ci.metadata = DcsmMetadata{};
            ci.m_currentFocusRequests.clear();
            addProviderEvent_ContentStateChange(contentID, EDcsmState::AcceptStopOffer, CategoryInfo{}, AnimationInformation{0, 0}, Guid());
        }
        else
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendRequestStopOfferContent: set content " << contentID <<
                     " state " << EnumToString(ci.state) << " -> StopOfferRequested, consumed by " << ci.consumerID);
            ci.state = ContentState::StopOfferRequested;
        }

        return true;
    }

    bool DcsmComponent::sendForceStopOfferContent(ContentID contentID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendForceStopOfferContent: ContentID " << contentID);

        PlatformGuard guard(m_frameworkLock);
        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendForceStopOfferContent: called without local provider active");
            return false;
        }

        const char* methodName = "sendForceStopOfferContent";
        if (!isLocallyProvidingContent(methodName, contentID))
            return false;

        ContentInfo& ci = m_contentRegistry[contentID];

        if (m_localConsumerAvailable)
            addConsumerEvent_ForceStopOfferContent(contentID, m_myID);
        if (m_connected && !ci.localOnly)
            m_communicationSystem.sendDcsmBroadcastForceStopOfferContent(contentID);

        ci.state = ContentState::Unknown;
        ci.contentDescriptor = TechnicalContentDescriptor::Invalid();
        ci.metadata = DcsmMetadata{};
        ci.m_currentFocusRequests.clear();
        ci.consumerID = Guid();

        return true;
    }

    bool DcsmComponent::sendUpdateContentMetadata(ContentID contentID, const DcsmMetadata& metadata)
    {
        PlatformGuard guard(m_frameworkLock);

        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendUpdateContentMetadata: metadata " << metadata);

        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendUpdateContentMetadata: called without local provider active");
            return false;
        }
        const char* methodName = "sendUpdateContentMetadata";
        if (!isLocallyProvidingContent(methodName, contentID))
            return false;

        // send out update only but store merged state for later consumers
        ContentInfo& ci = m_contentRegistry[contentID];
        ci.metadata.updateFromOther(metadata);

        if (ci.state == ContentState::Assigned ||
            ci.state == ContentState::ReadyRequested ||
            ci.state == ContentState::Ready ||
            ci.state == ContentState::Shown ||
            ci.state == ContentState::StopOfferRequested)
        {
            const Guid& to = getContentConsumerID(contentID);
            assert(to.isValid());

            if (m_myID == to)
            {
                assert(m_localConsumerAvailable && "Logic error on consumer disable");
                addConsumerEvent_UpdateContentMetadata(contentID, metadata, m_myID);
            }
            else
                m_communicationSystem.sendDcsmUpdateContentMetadata(to, contentID, metadata);
        }
        return true;
    }

    bool DcsmComponent::sendCanvasSizeChange(ContentID contentID, const CategoryInfo& categoryInfo, AnimationInformation ai)
    {
        PlatformGuard guard(m_frameworkLock);

        const Guid& to = getContentProviderID(contentID);
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendCanvasSizeChange: to " << to << ", ContentID " << contentID <<
                 ", ci " << categoryInfo << ", ai [" << ai.startTimeStamp << ", " << ai.finishedTimeStamp << "]");

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
            addProviderEvent_CanvasSizeChange(contentID, categoryInfo, ai, m_myID);
        }
        else
        {
            m_communicationSystem.sendDcsmCanvasSizeChange(to, contentID, categoryInfo, ai);
        }
        return true;
    }

    bool DcsmComponent::sendContentStateChange(ContentID contentID, EDcsmState state, const CategoryInfo& categoryInfo, AnimationInformation ai)
    {
        PlatformGuard guard(m_frameworkLock);

        const Guid& to = getContentProviderID(contentID);
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStateChange: to " << to << ", ContentID " << contentID <<
                 ", EDcsmStatus " << state << ", ci " << categoryInfo << ", ai [" << ai.startTimeStamp << ", " << ai.finishedTimeStamp << "]");

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

        if ((ci.state != ContentState::Offered || state != EDcsmState::Assigned) && categoryInfo != CategoryInfo{})
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::sendContentStateChange: unexpected non-zero CategoryInfo for " << EnumToString(ci.state) << " -> " << state << " transition");
            return false;
        }

        ContentState newState = ContentState::Unknown;
        if (!isValidStateTransition(methodName, ci, state, newState))
            return false;

        const bool mustSendCachedDataLocally = (ci.state == ContentState::Offered && newState == ContentState::Assigned &&
                                              m_myID == to);

        LOG_INFO_F(CONTEXT_DCSM, ([&](ramses_internal::StringOutputStream& sos) {
            sos << "DcsmComponent(" << m_myID << ")::sendContentStateChange: state change " << EnumToString(ci.state) << " -> " << state << " -> " << EnumToString(newState);
            if (mustSendCachedDataLocally)
            {
                if (!ci.metadata.empty())
                    sos << ", will send metadata to local consumer " << ci.metadata;
                if (!ci.m_currentFocusRequests.empty())
                {
                    sos << ", will send focusrequests";
                    for (auto& req : ci.m_currentFocusRequests)
                        sos << req << ",";
                }
                if (ci.contentDescriptor.isValid())
                    sos <<", will send content descriptor";
            }
        }));
        ci.state = newState;

        if (ci.state == ContentState::Offered)
            ci.consumerID = Guid();
        if (ci.state == ContentState::Assigned)
            ci.consumerID = m_myID;
        if (newState == ContentState::Unknown)
        {
            ci.contentDescriptor = TechnicalContentDescriptor::Invalid();
            ci.metadata = DcsmMetadata{};
            ci.m_currentFocusRequests.clear();
        }

        if (m_myID == to)
        {

            if (mustSendCachedDataLocally)
            {
                if (ci.contentDescriptor.isValid())
                    addConsumerEvent_ContentDescription(contentID, ci.contentDescriptor, m_myID);
                if (!ci.metadata.empty())
                    addConsumerEvent_UpdateContentMetadata(contentID, ci.metadata, m_myID);
                for (auto& req : ci.m_currentFocusRequests)
                    addConsumerEvent_ContentEnableFocusRequest( contentID, req, m_myID);
            }

            assert(m_localProviderAvailable && "Logic error in provider disable");
            addProviderEvent_ContentStateChange(contentID, state, categoryInfo, ai, m_myID);
        }
        else
        {
            m_communicationSystem.sendDcsmContentStateChange(to, contentID, state, categoryInfo, ai);
        }
        return true;
    }

    bool DcsmComponent::sendContentStatus(ContentID contentID, ramses::DcsmStatusMessageImpl const& message)
    {
        LOG_INFO_P(CONTEXT_DCSM, "DcsmComponent({})::sendContentStatus: ContentID {}, message {}",
            m_myID, contentID, message);
        PlatformGuard guard(m_frameworkLock);

        if (!m_localConsumerAvailable)
        {
            LOG_WARN_P(CONTEXT_DCSM, "DcsmComponent({})::sendContentStatus: called without local consumer active", m_myID);
            return false;
        }

        const char* methodName = "sendContentStatus";
        if (!isValidContent(methodName, contentID))
            return false;

        ContentInfo& ci = *m_contentRegistry.get(contentID);

        const Guid& to = getContentProviderID(contentID);
        if (!isAllowedToSendTo(methodName, to))
            return false;

        if (ci.state == ContentState::Offered || ci.state == ContentState::Unknown || ci.consumerID != m_myID)
        {
            LOG_WARN_P(CONTEXT_DCSM, "DcsmComponent({})::sendContentStatus: cannot send contentstatus for content {} consumed by {} in state {}", m_myID, contentID, ci.consumerID, EnumToString(ci.state));
            return false;
        }

        if (m_myID == to)
        {
            assert(m_localProviderAvailable && "Logic error in provider disable");
            auto msgCopy = std::make_unique<ramses::DcsmStatusMessageImpl>(static_cast<ramses::DcsmStatusMessageImpl::Type>(message.getType()), message.getData());
            addProviderEvent_ContentStatus(contentID, std::move(msgCopy), m_myID);
        }
        else
        {
            m_communicationSystem.sendDcsmContentStatus(to, contentID, static_cast<uint64_t>(message.getType()), message.getData());
        }
        return true;
    }

    void DcsmComponent::addProviderEvent_CanvasSizeChange(ContentID contentID, CategoryInfo categoryInfo, AnimationInformation ai, const Guid& consumerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::CanvasSizeChange;
        event.contentID = contentID;
        event.categoryInfo = categoryInfo;
        event.animation = ai;
        event.from      = consumerID;
        m_providerEvents.push_back(std::move(event));
        m_providerEventsSignal.notify_one();
    }

    void DcsmComponent::addProviderEvent_ContentStateChange(ContentID contentID, EDcsmState status, CategoryInfo categoryInfo, AnimationInformation ai, const Guid& consumerID)
    {
        DcsmEvent event;
        event.cmdType = EDcsmCommandType::ContentStateChange;
        event.contentID = contentID;
        event.state = status;
        event.categoryInfo = categoryInfo;
        event.animation = ai;
        event.from = consumerID;
        m_providerEvents.push_back(std::move(event));
        m_providerEventsSignal.notify_one();
    }

    void DcsmComponent::addProviderEvent_ContentStatus(ContentID contentID, std::unique_ptr<ramses::DcsmStatusMessageImpl>&& message, const Guid& consumerID)
    {
        DcsmEvent event;
        event.cmdType = EDcsmCommandType::ContentStatus;
        event.contentID = contentID;
        event.from = consumerID;
        event.contentStatus = std::move(message);
        m_providerEvents.push_back(std::move(event));
        m_providerEventsSignal.notify_one();
    }

    void DcsmComponent::addConsumerEvent_OfferContent(ContentID contentID, Category category, ETechnicalContentType technicalContentType, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType     = EDcsmCommandType::OfferContent;
        event.contentID   = contentID;
        event.category    = category;
        event.contentType = technicalContentType;
        event.from        = providerID;
        m_consumerEvents.push_back(std::move(event));
        m_consumerEventsSignal.notify_one();
    }

    void DcsmComponent::addConsumerEvent_ContentDescription(ContentID contentID, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType = EDcsmCommandType::ContentDescription;
        event.contentID = contentID;
        event.descriptor = technicalContentDescriptor;
        event.from = providerID;
        m_consumerEvents.push_back(std::move(event));
        m_consumerEventsSignal.notify_one();
    }

    void DcsmComponent::addConsumerEvent_ContentReady(ContentID contentID, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType     = EDcsmCommandType::ContentReady;
        event.contentID   = contentID;
        event.from        = providerID;
        m_consumerEvents.push_back(std::move(event));
        m_consumerEventsSignal.notify_one();
    }

    void DcsmComponent::addConsumerEvent_ContentEnableFocusRequest(ContentID contentID, int32_t focusRequest, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::ContentEnableFocusRequest;
        event.contentID = contentID;
        event.focusRequest = focusRequest;
        event.from      = providerID;
        m_consumerEvents.push_back(std::move(event));
        m_consumerEventsSignal.notify_one();
    }

    void DcsmComponent::addConsumerEvent_ContentDisableFocusRequest(ContentID contentID, int32_t focusRequest, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType = EDcsmCommandType::ContentDisableFocusRequest;
        event.contentID = contentID;
        event.focusRequest = focusRequest;
        event.from = providerID;
        m_consumerEvents.push_back(std::move(event));
        m_consumerEventsSignal.notify_one();
    }

    void DcsmComponent::addConsumerEvent_RequestStopOfferContent(ContentID contentID, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::StopOfferContentRequest;
        event.contentID = contentID;
        event.from      = providerID;
        m_consumerEvents.push_back(std::move(event));
        m_consumerEventsSignal.notify_one();
    }

    void DcsmComponent::addConsumerEvent_ForceStopOfferContent(ContentID contentID, const Guid& providerID)
    {
        // in case of force stop offer, content is gone no matter what. any potentially queued up previous events are obsolete.
        // delete them to not pointlessly trigger calls from user about obsolete content "session".
        auto it = std::remove_if(m_consumerEvents.begin(), m_consumerEvents.end(), [&](DcsmEvent const& ev) { return ev.contentID == contentID; });
        m_consumerEvents.erase(it, m_consumerEvents.end());

        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::ForceStopOfferContent;
        event.contentID = contentID;
        event.from      = providerID;
        m_consumerEvents.push_back(std::move(event));
        m_consumerEventsSignal.notify_one();
    }

    void DcsmComponent::addConsumerEvent_UpdateContentMetadata(ContentID contentID, DcsmMetadata metadata, const Guid& providerID)
    {
        DcsmEvent event;
        event.cmdType   = EDcsmCommandType::UpdateContentMetadata;
        event.contentID = contentID;
        event.metadata  = std::move(metadata);
        event.from      = providerID;
        m_consumerEvents.push_back(std::move(event));
        m_consumerEventsSignal.notify_one();
    }

    const char* DcsmComponent::EnumToString(EDcsmCommandType cmd) const
    {
        switch (cmd)
        {
        case EDcsmCommandType::OfferContent:
            return "OfferContent";
        case EDcsmCommandType::ContentDescription:
            return "ContentDescription";
        case EDcsmCommandType::ContentReady:
            return "ContentReady";
        case EDcsmCommandType::ContentStateChange:
            return "ContentStateChange";
        case EDcsmCommandType::CanvasSizeChange:
            return "CanvasSizeChange";
        case EDcsmCommandType::ContentEnableFocusRequest:
            return "ContentEnableFocusRequest";
        case EDcsmCommandType::ContentDisableFocusRequest:
            return "ContentDisableFocusRequest";
        case EDcsmCommandType::StopOfferContentRequest:
            return "StopOfferContentRequest";
        case EDcsmCommandType::ForceStopOfferContent:
            return "ForceStopOfferContent";
        case EDcsmCommandType::UpdateContentMetadata:
            return "UpdateContentMetadata";
        case EDcsmCommandType::ContentStatus:
            return "ContentStatus";
        }
        return "<Invalid EDcsmCommandType>";
    }

    const char* DcsmComponent::EnumToString(ContentState val) const
    {
        switch(val)
        {
        case ContentState::Unknown:
            return "Unknown";
        case ContentState::Offered:
            return "Offered";
        case ContentState::Assigned:
            return "Assigned";
        case ContentState::ReadyRequested:
            return "ReadyRequested";
        case ContentState::Ready:
            return "Ready";
        case ContentState::Shown:
            return "Shown";
        case ContentState::StopOfferRequested:
            return "StopOfferRequested";
        }
        return "<Invalid ContentState>";
    }

    // TODO(tobias): add error return channel and use on all false return in handle functions

    void DcsmComponent::handleCanvasSizeChange(ContentID contentID, const CategoryInfo& categoryInfo, AnimationInformation ai, const Guid& consumerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleCanvasSizeChange: from " << consumerID << ", ContentID " << contentID <<
                 ", ci " << categoryInfo << ", ai [" << ai.startTimeStamp << ", " << ai.finishedTimeStamp << "]");

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
        if (ci->localOnly && consumerID != m_myID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleCanvasSizeChange: ignoring message for " << contentID << " from " << consumerID << " because content is local only");
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

        addProviderEvent_CanvasSizeChange(contentID, categoryInfo, ai, consumerID);
    }

    void DcsmComponent::handleContentStateChange(ContentID contentID, EDcsmState state, const CategoryInfo& categoryInfo, AnimationInformation ai, const Guid& consumerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStateChange: from " << consumerID << ", ContentID " << contentID <<
                 ", EDcsmStatus " << state << ", ci " << categoryInfo << ", ai [" << ai.startTimeStamp << ", " << ai.finishedTimeStamp << "]");

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
        if (ci->localOnly && consumerID != m_myID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStateChange: ignoring message for " << contentID << " from " << consumerID << " because content is local only");
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

        const bool mustSendCachedData = (ci->state == ContentState::Offered && newState == ContentState::Assigned);
        LOG_INFO_F(CONTEXT_DCSM, ([&](ramses_internal::StringOutputStream& sos) {
            sos << "DcsmComponent(" << m_myID << ")::handleContentStateChange: state change " << EnumToString(ci->state) << " -> " << state << " -> " << EnumToString(newState);
            if (mustSendCachedData)
            {
                if (!ci->metadata.empty())
                    sos << ", will send metadata " << ci->metadata;
                if (!ci->m_currentFocusRequests.empty())
                {
                    sos << ", will send focusrequests";
                    for (auto& req : ci->m_currentFocusRequests)
                        sos << req <<",";
                }
                if (ci->contentDescriptor.isValid())
                    sos << ", will send content descriptor";
            }
        }));
        ci->state = newState;

        if (ci->state == ContentState::Offered)
            ci->consumerID = Guid();
        if (ci->state == ContentState::Assigned)
            ci->consumerID = consumerID;
        if (newState == ContentState::Unknown)
        {
            ci->contentDescriptor = TechnicalContentDescriptor::Invalid();
            ci->metadata = DcsmMetadata{};
            ci->m_currentFocusRequests.clear();
        }

        if (mustSendCachedData)
        {
            assert(m_connected);
            if(ci->contentDescriptor.isValid())
                m_communicationSystem.sendDcsmContentDescription(consumerID, contentID, ci->contentDescriptor);
            if (!ci->metadata.empty())
                m_communicationSystem.sendDcsmUpdateContentMetadata(consumerID, contentID, ci->metadata);
            for (auto& req : ci->m_currentFocusRequests)
                m_communicationSystem.sendDcsmContentEnableFocusRequest(consumerID, contentID, req);
        }

        addProviderEvent_ContentStateChange(contentID, state, categoryInfo, ai, consumerID);
    }

    void DcsmComponent::handleContentStatus(ContentID contentID, uint64_t messageID, absl::Span<const Byte> message, const Guid& consumerID)
    {
        if (!ramses::DcsmStatusMessageImpl::isValidType(messageID))
        {
            LOG_WARN_P(CONTEXT_DCSM, "DcsmComponent({})::handleContentStatus: from {}, ContentID {} messageID {} is unknown. Ignoring message of size:{}",
                m_myID, consumerID, contentID, messageID, message.size());
            return;
        }

        auto statusMsg = std::make_unique<ramses::DcsmStatusMessageImpl>(static_cast<ramses::DcsmStatusMessageImpl::Type>(messageID), message);
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStatus: from " << consumerID << ", ContentID " << contentID
            << ", messageID " << messageID << ", message " << *statusMsg << ", hasLocalProvider " << m_localProviderAvailable);

        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStatus: sent to non-existing local provider");
            return;
        }
        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStatus: received from " << consumerID << " for unknown content " << contentID);
            return;
        }
        if (ci->providerID != m_myID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentStatus: received from " << consumerID << " for content " << contentID << " from remote " << ci->providerID);
            return;
        }
        const char* methodName = "handleContentStatus";
        if (!isAllowedToReceiveFrom(methodName, consumerID))
            return;

        addProviderEvent_ContentStatus(contentID, std::move(statusMsg), consumerID);
    }

    void DcsmComponent::handleOfferContent(ContentID contentID, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleOfferContent: from " << providerID << ", ContentID " << contentID
            << ", Category " << category << ", ETechnicalContentType " << technicalContentType
            << ", Name '" << friendlyName << "', hasLocalConsumer " << m_localConsumerAvailable);

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
            else if (ci->providerID != providerID)
            {
                LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleOfferContent: " << providerID << " offered content " << contentID << " previously provided by " << ci->providerID);
            }
        }

        if (m_localConsumerAvailable)
        {
            addConsumerEvent_OfferContent(contentID, category, technicalContentType, providerID);
        }

        m_contentRegistry.put(contentID, ContentInfo{contentID, category, friendlyName, ContentState::Offered, providerID, Guid(), DcsmMetadata{}, {}, false, technicalContentType, TechnicalContentDescriptor::Invalid() });
    }

    void DcsmComponent::handleContentDescription(ContentID contentID, TechnicalContentDescriptor technicalContentDescriptor, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentDescription: from " << providerID << ", ContentID " << contentID <<
            ", TechnicalContentDescriptor " << technicalContentDescriptor);

        const char* methodName = "handleContentDescription";
        if (!isAllowedToReceiveFrom(methodName, providerID))
            return;

        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentDescription: received from " << providerID << " for unknown content " << contentID);
            return;
        }
        if (ci->state == ContentState::Unknown)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentDescription: received from " << providerID << " not proving contentID " << contentID);
            return;
        }
        if (ci->providerID != providerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentDescription: received from " << providerID << " for content " << contentID << " but registered by remote " << ci->providerID);
            return;
        }
        if (ci->state != ContentState::ReadyRequested && ci->state != ContentState::Assigned)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentDescription: received from " << providerID << " not allowed in state " << EnumToString(ci->state));
            return;
        }
        if (!m_localConsumerAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentDescription: sent to non-existing local consumer");
            return;
        }
        if (ci->contentDescriptor.isValid() && (ci->contentDescriptor != technicalContentDescriptor))
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentReady: received descriptor different from the ones previously sent" << ci->contentDescriptor);
            return;
        }

        ci->contentDescriptor = technicalContentDescriptor;
        addConsumerEvent_ContentDescription(contentID, technicalContentDescriptor, providerID);
    }

    void DcsmComponent::handleContentReady(ContentID contentID, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentReady: from " << providerID << ", ContentID " << contentID);

        const char* methodName = "handleContentReady";
        if (!isAllowedToReceiveFrom(methodName, providerID))
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
        addConsumerEvent_ContentReady(contentID, providerID);
    }

    void DcsmComponent::handleContentEnableFocusRequest(ContentID contentID, int32_t focusRequest, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentEnableFocusRequest: from " << providerID << ", ContentID " << contentID << " focusRequest " << focusRequest);

        const char* methodName = "handleContentEnableFocusRequest";
        if (!isAllowedToReceiveFrom(methodName, providerID))
            return;

        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentEnableFocusRequest: received from " << providerID << " for unknown content " << contentID);
            return;
        }
        if (ci->providerID != providerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentEnableFocusRequest: received from " << providerID << " for content " << contentID << " but registered by remote " << ci->providerID);
            return;
        }
        if (ci->state != ContentState::Assigned &&
            ci->state != ContentState::ReadyRequested &&
            ci->state != ContentState::Ready &&
            ci->state != ContentState::Shown &&
            ci->state != ContentState::StopOfferRequested)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentEnableFocusRequest: received from " << providerID << " for content " << contentID << " in invalid state " << EnumToString(ci->state));
            return;
        }
        if (!m_localConsumerAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentEnableFocusRequest: sent to non-existing local consumer");
            return;
        }

        addConsumerEvent_ContentEnableFocusRequest(contentID, focusRequest, providerID);
    }

    void DcsmComponent::handleContentDisableFocusRequest(ContentID contentID, int32_t focusRequest, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentDisableFocusRequest: from " << providerID << ", ContentID " << contentID << " focusRequest " << focusRequest);

        const char* methodName = "handleContentFocusRequest";
        if (!isAllowedToReceiveFrom(methodName, providerID))
            return;

        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentDisableFocusRequest: received from " << providerID << " for unknown content " << contentID);
            return;
        }
        if (ci->providerID != providerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentDisableFocusRequest: received from " << providerID << " for content " << contentID << " but registered by remote " << ci->providerID);
            return;
        }
        if (ci->state != ContentState::Assigned &&
            ci->state != ContentState::ReadyRequested &&
            ci->state != ContentState::Ready &&
            ci->state != ContentState::Shown &&
            ci->state != ContentState::StopOfferRequested)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentDisableFocusRequest: received from " << providerID << " for content " << contentID << " in invalid state " << EnumToString(ci->state));
            return;
        }
        if (!m_localConsumerAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleContentDisableFocusRequest: sent to non-existing local consumer");
            return;
        }

        addConsumerEvent_ContentDisableFocusRequest(contentID, focusRequest, providerID);
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
            assert(!ci->consumerID.isValid());
            ci->state = ContentState::Unknown;
            ci->contentDescriptor = TechnicalContentDescriptor::Invalid();
            ci->metadata = DcsmMetadata{};
            ci->m_currentFocusRequests.clear();
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
        ci->contentDescriptor = TechnicalContentDescriptor::Invalid();
        ci->metadata = DcsmMetadata{};
        ci->m_currentFocusRequests.clear();
    }

    void DcsmComponent::handleUpdateContentMetadata(ContentID contentID, DcsmMetadata metadata, const Guid& providerID)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleUpdateContentMetadata: from " << providerID << ", ContentID " << contentID << ", metadata " << metadata);

        const char* methodName = "handleUpdateContentMetadata";
        if (!isAllowedToReceiveFrom(methodName, providerID))
            return;

        ContentInfo* ci = m_contentRegistry.get(contentID);
        if (!ci)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleUpdateContentMetadata: received from " << providerID << " for unknown content " << contentID);
            return;
        }
        if (ci->providerID != providerID)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleUpdateContentMetadata: received from " << providerID << " for content " << contentID << " but registered by remote " << ci->providerID);
            return;
        }
        if (ci->state != ContentState::Assigned &&
            ci->state != ContentState::ReadyRequested &&
            ci->state != ContentState::Ready &&
            ci->state != ContentState::Shown &&
            ci->state != ContentState::StopOfferRequested)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::handleUpdateContentMetadata: invalid for content " << contentID << " in state " << EnumToString(ci->state));
            return;
        }

        assert(m_localConsumerAvailable && "Logic error in consumer disable");
        addConsumerEvent_UpdateContentMetadata(contentID, std::move(metadata), providerID);
    }

    bool DcsmComponent::dispatchProviderEvents(IDcsmProviderEventHandler& handler)
    {
        return DcsmComponent::dispatchProviderEvents(handler, std::chrono::milliseconds{0});
    }

    bool DcsmComponent::dispatchConsumerEvents(ramses::IDcsmConsumerEventHandler& handler)
    {
        return DcsmComponent::dispatchConsumerEvents(handler, std::chrono::milliseconds{0});
    }

    bool DcsmComponent::dispatchProviderEvents(IDcsmProviderEventHandler& handler, std::chrono::milliseconds timeout)
    {
        if (!m_localProviderAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::dispatchProviderEvents: called without active local provider");
            return false;
        }

        std::vector<DcsmEvent> events;
        {
            PlatformGuard guard(m_frameworkLock);
            if (timeout != std::chrono::milliseconds{0})
                m_providerEventsSignal.wait_for(m_frameworkLock, timeout, [&]() { return !m_providerEvents.empty(); });
            m_providerEvents.swap(events);
        }

        for (auto& ev : events)
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::dispatchProviderEvents: " << EnumToString(ev.cmdType));

            switch (ev.cmdType)
            {
            case EDcsmCommandType::CanvasSizeChange:
                {
                    ramses::CategoryInfoUpdate update;
                    update.impl.setCategoryInfo(ev.categoryInfo);
                    handler.contentSizeChange(ramses::ContentID(ev.contentID.getValue()),
                                         update,
                                         ramses::AnimationInformation{ev.animation.startTimeStamp, ev.animation.finishedTimeStamp});
                }
                break;

            case EDcsmCommandType::ContentStateChange:
                {
                ramses::CategoryInfoUpdate update;
                update.impl.setCategoryInfo(ev.categoryInfo);
                handler.contentStateChange(ramses::ContentID(ev.contentID.getValue()), ev.state, update,
                                           ramses::AnimationInformation{ev.animation.startTimeStamp, ev.animation.finishedTimeStamp});
                }
                break;
            case EDcsmCommandType::ContentStatus:
                {
                    assert(ev.contentStatus);
                    handler.contentStatus(ramses::ContentID(ev.contentID.getValue()), std::move(ev.contentStatus));
                }
                break;
            default:
                assert(false && "Wrong DcsmCommandType");
            }
        }
        return true;
    }

    bool DcsmComponent::dispatchConsumerEvents(ramses::IDcsmConsumerEventHandler& handler, std::chrono::milliseconds timeout)
    {
        if (!m_localConsumerAvailable)
        {
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::dispatchConsumerEvents: called without active local consumer");
            return false;
        }

        std::vector<DcsmEvent> events;
        {
            PlatformGuard guard(m_frameworkLock);
            if (timeout != std::chrono::milliseconds{0})
                m_consumerEventsSignal.wait_for(m_frameworkLock, timeout, [&]() { return !m_consumerEvents.empty(); });
            m_consumerEvents.swap(events);
        }

        for (const auto& ev : events)
        {
            LOG_INFO(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::dispatchConsumerEvents: " << EnumToString(ev.cmdType));

            switch (ev.cmdType)
            {
            case EDcsmCommandType::OfferContent:
                handler.contentOffered(ramses::ContentID(ev.contentID.getValue()),
                                        ramses::Category(ev.category.getValue()),
                                        ramses::ETechnicalContentType(ev.contentType));
                    break;

            case EDcsmCommandType::ContentDescription:
                handler.contentDescription(ramses::ContentID(ev.contentID.getValue()),
                                            ramses::TechnicalContentDescriptor(ev.descriptor.getValue()));
                break;

            case EDcsmCommandType::ContentReady:
                handler.contentReady(ramses::ContentID(ev.contentID.getValue()));
                break;

            case EDcsmCommandType::ContentEnableFocusRequest:
                handler.contentEnableFocusRequest(ramses::ContentID(ev.contentID.getValue()), ev.focusRequest);
                break;

            case EDcsmCommandType::ContentDisableFocusRequest:
                handler.contentDisableFocusRequest(ramses::ContentID(ev.contentID.getValue()), ev.focusRequest);
                break;

            case EDcsmCommandType::StopOfferContentRequest:
                handler.contentStopOfferRequest(ramses::ContentID(ev.contentID.getValue()));
                break;

            case EDcsmCommandType::ForceStopOfferContent:
                handler.forceContentOfferStopped(ramses::ContentID(ev.contentID.getValue()));
                break;

            case EDcsmCommandType::UpdateContentMetadata:
                {
                    ramses::DcsmMetadataUpdate metadataUpdate(*new ramses::DcsmMetadataUpdateImpl);
                    metadataUpdate.impl.setMetadata(std::move(ev.metadata));
                    handler.contentMetadataUpdated(ramses::ContentID(ev.contentID.getValue()), metadataUpdate);
                    break;
                }

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
            return Guid();
        return ci->providerID;
    }

    Guid DcsmComponent::getContentConsumerID(ContentID content) const
    {
        const ContentInfo* ci = m_contentRegistry.get(content);
        if (!ci)
            return Guid();
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
        if (!m_connectedParticipants.contains(id))
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
        if (!m_connectedParticipants.contains(id))
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
        LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": invalid state transiton from " << EnumToString(ci.state) << " with " << transition);
        return false;
    }

    bool DcsmComponent::isValidETechnicalContentType(const char* callerMethod, ETechnicalContentType val) const
    {
        switch (val)
        {
        case ETechnicalContentType::RamsesSceneID: return true;
        case ETechnicalContentType::WaylandIviSurfaceID: return true;
        default:
            LOG_WARN(CONTEXT_DCSM, "DcsmComponent(" << m_myID << ")::" << callerMethod << ": invalid technicalContentType " << static_cast<std::underlying_type_t<ETechnicalContentType>>(val));
            return false;
        };
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

    void DcsmComponent::writeStateForLog(StringOutputStream& sos)
    {
        PlatformGuard guard(m_frameworkLock);
        sos << "DcsmComponent(" << m_myID << ") state:\n"
            << "  LocalProvider " << m_localProviderAvailable << " (events pending " << m_consumerEvents.size() << ")\n"
            << "  LocalConsumer " << m_localConsumerAvailable << " (events pending " << m_providerEvents.size() << ")\n"
            << "  Connected to network " << m_connected << "\n"
            << "  Connected participants:\n";
        for (const auto& p : m_connectedParticipants)
            sos << "  - " << p << "\n";
        sos << "  Known content:\n";
        for (const auto& p : m_contentRegistry)
        {
            const auto& ci = p.value;
            sos << "  - ContentID         " << ci.content << "\n"
                << "    Category          " << ci.category << "\n"
                << "    Name              " << ci.friendlyName << "\n"
                << "    State             " << EnumToString(ci.state) << "\n"
                << "    Provider          " << ci.providerID << "\n"
                << "    Consumer          " << ci.consumerID << "\n"
                << "    LocalOnly         " << ci.localOnly << "\n"
                << "    FocusRequests     ";
            for (const auto& focusRequest : ci.m_currentFocusRequests)
                sos << focusRequest << "," ;
            sos << "\n"
                << "    ContentType       " << ci.contentType << "\n"
                << "    ContentDescriptor " << ci.contentDescriptor << "\n";
        }
    }

    void DcsmComponent::triggerLogMessageForPeriodicLog()
    {
        // log format description:
        // {C|NC} LP <local provider state> LC <local consumer state> CP[<connected participants>] C[<contentId>,<category>,<state>,<provider|->,<consumer|->,<techType:descriptor|-:->,<localonly>,<active focus requests|->; ...]

        PlatformGuard guard(m_frameworkLock);
        LOG_INFO_PF(CONTEXT_PERIODIC, ([&](auto& out) {
                fmt::format_to(out, "Dcsm({}) {} LP:{} LC:{} CP[", m_myID, (m_connected ? "C" : "NC"), m_localProviderAvailable, m_localConsumerAvailable);
                for (const auto& p : m_connectedParticipants)
                    fmt::format_to(out, "{};", p);
                fmt::format_to(out, "] C[");
                for (const auto& p : m_contentRegistry)
                {
                    const auto& ci = p.value;
                    fmt::format_to(out, "{},{},'{}',{}", ci.content, ci.category, ci.friendlyName, EnumToString(ci.state));
                    if (!ci.providerID.isInvalid())
                        fmt::format_to(out, ",{}", ci.providerID);
                    else
                        fmt::format_to(out, ",-");
                    if (!ci.consumerID.isInvalid())
                        fmt::format_to(out, ",{}", ci.consumerID);
                    else
                        fmt::format_to(out, ",-");
                    if (ci.contentDescriptor.isValid())
                        fmt::format_to(out, ",{:s}:{}", ci.contentType, ci.contentDescriptor);
                    else
                        fmt::format_to(out, ",-");
                    if (ci.localOnly)
                        fmt::format_to(out, ",local");
                    if (!ci.m_currentFocusRequests.empty())
                    {
                        fmt::format_to(out, ",");
                        for (auto& focusRequest : ci.m_currentFocusRequests)
                            fmt::format_to(out, "{}|", focusRequest);
                    }
                    else
                        fmt::format_to(out, ",-");
                }
                fmt::format_to(out, "]");
            }));
    }
}
