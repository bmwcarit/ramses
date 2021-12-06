//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMCONNECTIONSYSTEM_H
#define RAMSES_DCSMCONNECTIONSYSTEM_H

#include "TransportCommon/ConnectionSystemBase.h"
#include "TransportCommon/ISomeIPDcsmStack.h"

namespace ramses_internal
{
    class IDcsmProviderServiceHandler;
    class IDcsmConsumerServiceHandler;
    class DcsmMetadata;

    class DcsmConnectionSystem : public ConnectionSystemBase<ISomeIPDcsmStackCallbacks>
    {
    public:
        static std::unique_ptr<DcsmConnectionSystem> Construct(const std::shared_ptr<ISomeIPDcsmStack>& stack, UInt32 communicationUserID, const ParticipantIdentifier& namedPid,
                                                               UInt32 protocolVersion, PlatformLock& frameworkLock,
                                                               std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout,
                                                               std::function<std::chrono::steady_clock::time_point(void)> steadyClockNow);

        ~DcsmConnectionSystem() override;

        bool sendBroadcastOfferContent(ContentID content, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName, uint32_t sortOrder);
        bool sendOfferContent(const Guid& to, ContentID content, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName, uint32_t sortOrder);
        bool sendBroadcastRequestStopOfferContent(ContentID content, bool forceStopOffer);

        bool sendCanvasSizeChange(const Guid& to, ContentID content, const CategoryInfo& categoryInfo, uint16_t dpi, const AnimationInformation& animation);
        bool sendContentStateChange(const Guid& to, ContentID content, EDcsmState state, const CategoryInfo& categoryInfo, const AnimationInformation& animation);
        bool sendContentStatus(const Guid& to, ContentID contentID, uint64_t messageID, std::vector<ramses_internal::Byte> const& message);

        bool sendContentDescription(const Guid& to, ContentID content, TechnicalContentDescriptor technicalContentDescriptor);
        bool sendContentReady(const Guid& to, ContentID content);
        bool sendContentEnableFocusRequest(const Guid& to, ContentID content, int32_t focusRequest);
        bool sendContentDisableFocusRequest(const Guid& to, ContentID content, int32_t focusRequest);

        bool sendBroadcastUpdateContentMetadata(ContentID contentID, const DcsmMetadata& metadata);
        bool sendUpdateContentMetadata(const Guid& to, ContentID contentID, const DcsmMetadata& metadata);

        void setDcsmProviderServiceHandler(IDcsmProviderServiceHandler* handler);
        void setDcsmConsumerServiceHandler(IDcsmConsumerServiceHandler* handler);

    private:
        DcsmConnectionSystem(const std::shared_ptr<ISomeIPDcsmStack>& stack, UInt32 communicationUserID, const ParticipantIdentifier& namedPid,
                             UInt32 protocolVersion, PlatformLock& frameworkLock,
                             std::chrono::milliseconds keepAliveInterval, std::chrono::milliseconds keepAliveTimeout,
                             std::function<std::chrono::steady_clock::time_point(void)> steadyClockNow);

        // from ISomeIPDcsmCallbacks
        virtual void handleOfferContent(const SomeIPMsgHeader& header, ContentID content, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName, uint32_t sortOrder) override;
        virtual void handleRequestStopOfferContent(const SomeIPMsgHeader& header, ContentID content, bool forceStopOffer) override;
        virtual void handleUpdateContentMetadata(const SomeIPMsgHeader& header, ContentID content, absl::Span<const Byte> metadata) override;

        virtual void handleCanvasSizeChange(const SomeIPMsgHeader& header, ContentID content, const CategoryInfo& categoryInfo, uint16_t dpi, const AnimationInformation& animation) override;
        virtual void handleContentDescription(const SomeIPMsgHeader& header, ContentID content, TechnicalContentDescriptor technicalContentDescriptor) override;
        virtual void handleContentReady(const SomeIPMsgHeader& header, ContentID content) override;
        virtual void handleContentStateChange(const SomeIPMsgHeader& header, ContentID content, EDcsmState state, const CategoryInfo& categoryInfo, const AnimationInformation& animation) override;
        virtual void handleContentEnableFocusRequest(const SomeIPMsgHeader& header, ContentID content, int32_t focusRequest) override;
        virtual void handleContentDisableFocusRequest(const SomeIPMsgHeader& header, ContentID content, int32_t focusRequest) override;
        virtual void handleResponse(const SomeIPMsgHeader& header, uint64_t originalMessageId, uint64_t originalSessionId, uint64_t responseCode) override;
        virtual void handleContentStatus(const SomeIPMsgHeader& header, ContentID content, uint64_t messageID, absl::Span<const Byte> message) override;

        std::shared_ptr<ISomeIPDcsmStack> m_stack;
        IDcsmProviderServiceHandler* m_dcsmProviderHandler = nullptr;
        IDcsmConsumerServiceHandler* m_dcsmConsumerHandler = nullptr;
    };
}

#endif
