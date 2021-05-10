//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/DcsmConnectionSystem.h"
#include "TransportCommon/ServiceHandlerInterfaces.h"
#include "Components/CategoryInfo.h"


namespace ramses_internal
{
    std::unique_ptr<DcsmConnectionSystem> DcsmConnectionSystem::Construct(const std::shared_ptr<ISomeIPDcsmStack>& stack, UInt32 communicationUserID, const ParticipantIdentifier& namedPid,
                                                                          UInt32 protocolVersion, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection,
                                                                          std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout,
                                                                          std::function<std::chrono::steady_clock::time_point(void)> steadyClockNow)
    {
        if (!CheckConstructorArguments(stack, communicationUserID, namedPid, protocolVersion, keepAliveInterval, keepAliveTimeout, CONTEXT_DCSM, "DCSM"))
            return nullptr;
        return std::unique_ptr<DcsmConnectionSystem>(new DcsmConnectionSystem(stack, communicationUserID, namedPid, protocolVersion, frameworkLock, statisticCollection,
                                                                              keepAliveInterval, keepAliveTimeout, std::move(steadyClockNow)));
    }

    DcsmConnectionSystem::DcsmConnectionSystem(const std::shared_ptr<ISomeIPDcsmStack>& stack, UInt32 communicationUserID, const ParticipantIdentifier& namedPid,
                                               UInt32 protocolVersion, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection,
                                               std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout,
                                               std::function<std::chrono::steady_clock::time_point(void)> steadyClockNow)
        : ConnectionSystemBase(stack,
                               communicationUserID,
                               namedPid,
                               protocolVersion,
                               frameworkLock,
                               statisticCollection,
                               keepAliveInterval, keepAliveTimeout,
                               std::move(steadyClockNow),
                               CONTEXT_DCSM,
                               "DCSM")
        , m_stack(stack)
    {
    }

    DcsmConnectionSystem::~DcsmConnectionSystem() = default;

    void DcsmConnectionSystem::setDcsmProviderServiceHandler(IDcsmProviderServiceHandler* handler)
    {
        m_dcsmProviderHandler = handler;
    }

    void DcsmConnectionSystem::setDcsmConsumerServiceHandler(IDcsmConsumerServiceHandler* handler)
    {
        m_dcsmConsumerHandler = handler;
    }

    // public send functions
    bool DcsmConnectionSystem::sendBroadcastOfferContent(ContentID content, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName, uint32_t sortOrder)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmConnectionSystem(" << m_communicationUserID << ")::sendBroadcastOfferContent: content " << content << ", category " << category << ", technicalContentType " << technicalContentType << ", name " << friendlyName << ", sortOrder " << sortOrder);
        return sendBroadcast("sendBroadcastOfferContent", [&](DcsmInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendOfferContent(iid, hdr, content, category, technicalContentType, friendlyName, sortOrder);
        });
    }

    bool DcsmConnectionSystem::sendOfferContent(const Guid& to, ContentID content, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName, uint32_t sortOrder)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmConnectionSystem(" << m_communicationUserID << ")::sendOfferContent: to " << to << ", content " << content << ", category " << category << ", technicalContentType " << technicalContentType << ", name " << friendlyName << ", sortOrder " << sortOrder);
        return sendUnicast("sendOfferContent", to, [&](DcsmInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendOfferContent(iid, hdr, content, category, technicalContentType, friendlyName, sortOrder);
        });
    }

    bool DcsmConnectionSystem::sendBroadcastRequestStopOfferContent(ContentID content, bool forceStopOffer)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmConnectionSystem(" << m_communicationUserID << ")::sendBroadcastRequestStopOfferContent: content " << content << ", forceStopOffer " << forceStopOffer);
        return sendBroadcast("sendBroadcastRequestStopOfferContent", [&](DcsmInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendRequestStopOfferContent(iid, hdr, content, forceStopOffer);
        });
    }

    bool DcsmConnectionSystem::sendCanvasSizeChange(const Guid& to, ContentID content, const CategoryInfo& categoryInfo, uint16_t dpi, const AnimationInformation& animation)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmConnectionSystem(" << m_communicationUserID << ")::sendCanvasSizeChange: to " << to << ", content " << content << ", " <<
                 ", dpi " << dpi << ", " << categoryInfo << ", animation [" << animation.startTimeStamp << "; " << animation.finishedTimeStamp << "]");
        return sendUnicast("sendCanvasSizeChange", to, [&](DcsmInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendCanvasSizeChange(iid, hdr, content, categoryInfo, dpi, animation);
        });
    }

    bool DcsmConnectionSystem::sendContentDescription(const Guid& to, ContentID content, TechnicalContentDescriptor technicalContentDescriptor)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmConnectionSystem(" << m_communicationUserID << ")::sendContentDescription: to " << to << ", content " << content <<
                 ", technicalContentDescriptor " << technicalContentDescriptor.getValue());
        return sendUnicast("sendContentDescription", to, [&](DcsmInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendContentDescription(iid, hdr, content, technicalContentDescriptor);
        });
    }

    bool DcsmConnectionSystem::sendContentReady(const Guid& to, ContentID content)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmConnectionSystem(" << m_communicationUserID << ")::sendContentReady: to " << to << ", content " << content);
        return sendUnicast("sendContentReady", to, [&](DcsmInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendContentReady(iid, hdr, content);
        });
    }

    bool DcsmConnectionSystem::sendContentStateChange(const Guid& to, ContentID content, EDcsmState state, const CategoryInfo& categoryInfo, const AnimationInformation& animation)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmConnectionSystem(" << m_communicationUserID << ")::sendContentStateChange: to " << to << ", content " << content << ", state " << state <<
                 ", " << categoryInfo << ", animation [" << animation.startTimeStamp << "; " << animation.finishedTimeStamp << "]");
        return sendUnicast("sendContentStateChange", to, [&](DcsmInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendContentStateChange(iid, hdr, content, state, categoryInfo, animation);
        });
    }

    bool DcsmConnectionSystem::sendContentStatus(const Guid& to, ContentID content, uint64_t messageID, std::vector<ramses_internal::Byte> const& message)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmConnectionSystem(" << m_communicationUserID << ")::sendContentStatus: to " << to << ", content " << content << ", message id  " << messageID <<
            ", data with size" << message.size() << "]");
        return sendUnicast("sendDcsmContentStatus", to, [&](DcsmInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendContentStatus(iid, hdr, content, messageID, message);
            });
    }

    bool DcsmConnectionSystem::sendContentEnableFocusRequest(const Guid& to, ContentID content, int32_t focusRequest)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmConnectionSystem(" << m_communicationUserID << ")::sendContentEnableFocusRequest: to " << to << ", content " << content << ", focusRequest " << focusRequest);
        return sendUnicast("sendContentEnableFocusRequest", to, [&](DcsmInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendContentEnableFocusRequest(iid, hdr, content, focusRequest);
        });
    }

    bool DcsmConnectionSystem::sendContentDisableFocusRequest(const Guid& to, ContentID content, int32_t focusRequest)
    {
        LOG_INFO(CONTEXT_DCSM, "DcsmConnectionSystem(" << m_communicationUserID << ")::sendContentDisableFocusRequest: to " << to << ", content " << content << ", focusRequest " << focusRequest);
        return sendUnicast("sendContentDisableFocusRequest", to, [&](DcsmInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendContentDisableFocusRequest(iid, hdr, content, focusRequest);
        });
    }

    bool DcsmConnectionSystem::sendBroadcastUpdateContentMetadata(ContentID contentID, const DcsmMetadata& metadata)
    {
        const std::vector<Byte> metadataBlob = metadata.toBinary();
        LOG_INFO(CONTEXT_DCSM, "DcsmConnectionSystem(" << m_communicationUserID << ")::sendBroadcastUpdateContentMetadata: content " << contentID << ", metadataSize " << metadataBlob.size());

        return sendBroadcast("sendBroadcastUpdateContentMetadata", [&](DcsmInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendUpdateContentMetadata(iid, hdr, contentID, metadataBlob);
        });
    }

    bool DcsmConnectionSystem::sendUpdateContentMetadata(const Guid& to, ContentID contentID, const DcsmMetadata& metadata)
    {
        const std::vector<Byte> metadataBlob = metadata.toBinary();
        LOG_INFO(CONTEXT_DCSM, "DcsmConnectionSystem(" << m_communicationUserID << ")::sendUpdateContentMetadata: to " << to << ", content " << contentID << ", metadataSize " << metadataBlob.size());

        return sendUnicast("sendUpdateContentMetadata", to, [&](DcsmInstanceId iid, SomeIPMsgHeader hdr) {
            return m_stack->sendUpdateContentMetadata(iid, hdr, contentID, metadataBlob);
        });
    }


    // handlers for generic ISomeIPDcsmCallbacks callbacks
    void DcsmConnectionSystem::handleOfferContent(const SomeIPMsgHeader& header, ContentID content, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName, uint32_t /*sortOrder*/)
    {
        assert(m_dcsmConsumerHandler);
        if (const Guid* pid = processReceivedMessageHeader(header, "handleOfferContent"))
            m_dcsmConsumerHandler->handleOfferContent(content, category, technicalContentType, friendlyName, *pid);
    }

    void DcsmConnectionSystem::handleRequestStopOfferContent(const SomeIPMsgHeader& header, ContentID content, bool forceStopOffer)
    {
        assert(m_dcsmConsumerHandler);
        if (const Guid* pid = processReceivedMessageHeader(header, "handleRequestStopOfferContent"))
        {
            if (forceStopOffer)
                m_dcsmConsumerHandler->handleForceStopOfferContent(content, *pid);
            else
                m_dcsmConsumerHandler->handleRequestStopOfferContent(content, *pid);
        }
    }

    void DcsmConnectionSystem::handleUpdateContentMetadata(const SomeIPMsgHeader& header, ContentID content, absl::Span<const Byte> metadata)
    {
        assert(m_dcsmConsumerHandler);
        if (const Guid* pid = processReceivedMessageHeader(header, "handleUpdateContentMetadata"))
            m_dcsmConsumerHandler->handleUpdateContentMetadata(content, DcsmMetadata(metadata), *pid);
    }

    void DcsmConnectionSystem::handleCanvasSizeChange(const SomeIPMsgHeader& header, ContentID content, const CategoryInfo& categoryInfo, uint16_t /*dpi*/, const AnimationInformation& animation)
    {
        assert(m_dcsmProviderHandler);
        if (const Guid* pid = processReceivedMessageHeader(header, "handleCanvasSizeChange"))
            m_dcsmProviderHandler->handleCanvasSizeChange(content, categoryInfo, animation, *pid);
    }

    void DcsmConnectionSystem::handleContentDescription(const SomeIPMsgHeader& header, ContentID content, TechnicalContentDescriptor technicalContentDescriptor)
    {
        assert(m_dcsmConsumerHandler);
        if (const Guid* pid = processReceivedMessageHeader(header, "handleContentDescription"))
            m_dcsmConsumerHandler->handleContentDescription(content, technicalContentDescriptor, *pid);
    }

    void DcsmConnectionSystem::handleContentReady(const SomeIPMsgHeader& header, ContentID content)
    {
        assert(m_dcsmConsumerHandler);
        if (const Guid* pid = processReceivedMessageHeader(header, "handleContentReady"))
            m_dcsmConsumerHandler->handleContentReady(content, *pid);
    }

    void DcsmConnectionSystem::handleContentStateChange(const SomeIPMsgHeader& header, ContentID content, EDcsmState state, const CategoryInfo& categoryInfo, const AnimationInformation& animation)
    {
        assert(m_dcsmProviderHandler);
        if (const Guid* pid = processReceivedMessageHeader(header, "handleContentStateChange"))
            m_dcsmProviderHandler->handleContentStateChange(content, state, categoryInfo, animation, *pid);
    }

    void DcsmConnectionSystem::handleContentEnableFocusRequest(const SomeIPMsgHeader& header, ContentID content, int32_t focusRequest)
    {
        assert(m_dcsmConsumerHandler);
        if (const Guid* pid = processReceivedMessageHeader(header, "handleContentEnableFocusRequest"))
            m_dcsmConsumerHandler->handleContentEnableFocusRequest(content, focusRequest, *pid);
    }

    void DcsmConnectionSystem::handleContentDisableFocusRequest(const SomeIPMsgHeader& header, ContentID content, int32_t focusRequest)
    {
        assert(m_dcsmConsumerHandler);
        if (const Guid* pid = processReceivedMessageHeader(header, "handleContentDisableFocusRequest"))
            m_dcsmConsumerHandler->handleContentDisableFocusRequest(content, focusRequest, *pid);
    }

    void DcsmConnectionSystem::handleResponse(const SomeIPMsgHeader& header, uint64_t /*originalMessageId*/, uint64_t /*originalSessionId*/, uint64_t /*responseCode*/)
    {
        if (const Guid* pid = processReceivedMessageHeader(header, "handleResponse"))
            LOG_WARN(CONTEXT_DCSM, "DcsmConnectionSystem(" << m_communicationUserID << ")::handleResponse: unexpected message from " << *pid);
    }

    void DcsmConnectionSystem::handleContentStatus(const SomeIPMsgHeader& header, ContentID content, uint64_t messageID, absl::Span<const Byte> message)
    {
        assert(m_dcsmProviderHandler);
        if (const Guid* pid = processReceivedMessageHeader(header, "handleContentStatus"))
            m_dcsmProviderHandler->handleContentStatus(content, messageID, message, *pid);
    }
}
