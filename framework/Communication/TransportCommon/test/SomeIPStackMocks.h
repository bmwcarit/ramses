//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TEST_SOMEIPSTACKMOCKS_H
#define RAMSES_TEST_SOMEIPSTACKMOCKS_H

#include "TransportCommon/ISomeIPDcsmStack.h"
#include "TransportCommon/ISomeIPRamsesStack.h"
#include "DcsmGmockPrinter.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    // gmock printers for types in mocks
    inline void PrintTo(const DcsmInstanceId& id, ::std::ostream* os)
    {
        *os << "DcsmInstanceId:" << id.getValue();
    }

    inline void PrintTo(const RamsesInstanceId& id, ::std::ostream* os)
    {
        *os << "RamsesInstanceId:" << id.getValue();
    }

    inline void PrintTo(const SomeIPMsgHeader& hdr, ::std::ostream* os)
    {
        StringOutputStream sos;
        sos << hdr;
        *os << sos.release().stdRef();
    }

    class SomeIPDcsmStackCallbacksMock : public ISomeIPDcsmStackCallbacks
    {
    public:
        SomeIPDcsmStackCallbacksMock();
        virtual ~SomeIPDcsmStackCallbacksMock() override;

        MOCK_METHOD(void, handleServiceAvailable, (DcsmInstanceId iid), (override));
        MOCK_METHOD(void, handleServiceUnavailable, (DcsmInstanceId iid), (override));

        MOCK_METHOD(void, handleParticipantInfo, (const SomeIPMsgHeader& header, uint16_t protocolVersion, DcsmInstanceId senderInstanceId, uint64_t expectedReceiverPid, uint8_t clockType, uint64_t timestampNow), (override));
        MOCK_METHOD(void, handleKeepAlive, (const SomeIPMsgHeader& header, uint64_t timestampNow), (override));

        MOCK_METHOD(void, handleOfferContent, (const SomeIPMsgHeader& header, ContentID content, Category category, const std::string& friendlyName, uint32_t sortOrder), (override));
        MOCK_METHOD(void, handleRequestStopOfferContent, (const SomeIPMsgHeader& header, ContentID content, bool forceStopOffer), (override));
        MOCK_METHOD(void, handleUpdateContentMetadata, (const SomeIPMsgHeader& header, ContentID content, absl::Span<const Byte> metadata), (override));

        MOCK_METHOD(void, handleCanvasSizeChange, (const SomeIPMsgHeader& header, ContentID content, const CategoryInfo& categoryInfo, uint16_t dpi, const AnimationInformation& animation), (override));
        MOCK_METHOD(void, handleContentDescription, (const SomeIPMsgHeader& header, ContentID content, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor), (override));
        MOCK_METHOD(void, handleContentReady, (const SomeIPMsgHeader& header, ContentID content), (override));
        MOCK_METHOD(void, handleContentStateChange, (const SomeIPMsgHeader& header, ContentID content, EDcsmState state, const CategoryInfo& sizeInfo, const AnimationInformation& animation), (override));
        MOCK_METHOD(void, handleContentEnableFocusRequest, (const SomeIPMsgHeader& header, ContentID content, int32_t focusRequest), (override));
        MOCK_METHOD(void, handleContentDisableFocusRequest, (const SomeIPMsgHeader& header, ContentID content, int32_t focusRequest), (override));
        MOCK_METHOD(void, handleResponse, (const SomeIPMsgHeader& header, uint64_t originalMessageId, uint64_t originalSessionId, uint64_t responseCode), (override));
    };

    class SomeIPDcsmStackMock : public ISomeIPDcsmStack
    {
    public:
        SomeIPDcsmStackMock();
        virtual ~SomeIPDcsmStackMock() override;

        MOCK_METHOD(bool, connect, (), (override));
        MOCK_METHOD(bool, disconnect, (), (override));
        MOCK_METHOD(void, logConnectionState, (StringOutputStream& sos), (override));

        MOCK_METHOD(DcsmInstanceId, getServiceInstanceId, (), (const, override));
        MOCK_METHOD(void, setCallbacks, (ISomeIPDcsmStackCallbacks*), (override));

        MOCK_METHOD(bool, sendParticipantInfo, (DcsmInstanceId to, const SomeIPMsgHeader& header, uint16_t protocolVersion, DcsmInstanceId senderInstanceId, uint64_t expectedReceiverPid, uint8_t clockType, uint64_t timestampNow), (override));
        MOCK_METHOD(bool, sendKeepAlive, (DcsmInstanceId to, const SomeIPMsgHeader& header, uint64_t timestampNow), (override));

        MOCK_METHOD(bool, sendOfferContent, (DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, Category category, const std::string& friendlyName, uint32_t sortOrder), (override));
        MOCK_METHOD(bool, sendRequestStopOfferContent, (DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, bool forceStopOffer), (override));
        MOCK_METHOD(bool, sendUpdateContentMetadata, (DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, const std::vector<Byte>& data), (override));

        MOCK_METHOD(bool, sendCanvasSizeChange, (DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, const CategoryInfo& categoryInfo, uint16_t dpi, const AnimationInformation& animation), (override));
        MOCK_METHOD(bool, sendContentDescription, (DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, ETechnicalContentType technicalContentType, TechnicalContentDescriptor technicalContentDescriptor), (override));
        MOCK_METHOD(bool, sendContentReady, (DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content), (override));
        MOCK_METHOD(bool, sendContentStateChange, (DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, EDcsmState state, const CategoryInfo& sizeInfo, const AnimationInformation& animation), (override));
        MOCK_METHOD(bool, sendContentEnableFocusRequest, (DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, int32_t focusRequest), (override));
        MOCK_METHOD(bool, sendContentDisableFocusRequest, (DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, int32_t focusRequest), (override));
        MOCK_METHOD(bool, sendResponse, (DcsmInstanceId to, const SomeIPMsgHeader& header, uint64_t originalMessageId, uint64_t originalSessionId, uint64_t responseCode), (override));
    };

    class SomeIPRamsesStackCallbacksMock : public ISomeIPRamsesStackCallbacks
    {
    public:
        SomeIPRamsesStackCallbacksMock();
        virtual ~SomeIPRamsesStackCallbacksMock() override;

        MOCK_METHOD(void, handleServiceAvailable, (RamsesInstanceId iid), (override));
        MOCK_METHOD(void, handleServiceUnavailable, (RamsesInstanceId iid), (override));

        MOCK_METHOD(void, handleParticipantInfo, (const SomeIPMsgHeader& header, uint16_t protocolVersion, RamsesInstanceId senderInstanceId, uint64_t expectedReceiverPid, uint8_t clockType, uint64_t timestampNow), (override));
        MOCK_METHOD(void, handleKeepAlive, (const SomeIPMsgHeader& header, uint64_t timestampNow), (override));

        MOCK_METHOD(void, handleRendererEvent, (const SomeIPMsgHeader& header, SceneId sceneId, const std::vector<Byte>& data), (override));
        MOCK_METHOD(void, handleRequestResources, (const SomeIPMsgHeader& header, const ResourceContentHashVector& resources), (override));
        MOCK_METHOD(void, handleResourcesNotAvailable, (const SomeIPMsgHeader& header, const ResourceContentHashVector& resources), (override));
        MOCK_METHOD(void, handleResourceTransfer, (const SomeIPMsgHeader& header, absl::Span<const Byte> resourceData), (override));
        MOCK_METHOD(void, handleSceneUpdate, (const SomeIPMsgHeader& header, SceneId sceneId, absl::Span<const Byte> sceneUpdate), (override));
        MOCK_METHOD(void, handleSceneAvailabilityChange, (const SomeIPMsgHeader& header, const std::vector<SceneAvailabilityUpdate>& update), (override));
        MOCK_METHOD(void, handleSceneSubscriptionChange, (const SomeIPMsgHeader& header, const std::vector<SceneSubscriptionUpdate>& update), (override));
        MOCK_METHOD(void, handleInitializeScene, (const SomeIPMsgHeader& header, SceneId sceneId), (override));
    };

    class SomeIPRamsesStackMock : public ISomeIPRamsesStack
    {
    public:
        SomeIPRamsesStackMock();
        virtual ~SomeIPRamsesStackMock() override;

        MOCK_METHOD(bool, connect, (), (override));
        MOCK_METHOD(bool, disconnect, (), (override));
        MOCK_METHOD(void, logConnectionState, (StringOutputStream& sos), (override));

        MOCK_METHOD(RamsesInstanceId, getServiceInstanceId, (), (const, override));
        MOCK_METHOD(void, setCallbacks, (ISomeIPRamsesStackCallbacks*), (override));
        MOCK_METHOD(RamsesStackSendDataSizes, getSendDataSizes, (), (const, override));

        MOCK_METHOD(bool, sendParticipantInfo, (RamsesInstanceId to, const SomeIPMsgHeader& header, uint16_t protocolVersion, RamsesInstanceId senderInstanceId, uint64_t expectedReceiverPid, uint8_t clockType, uint64_t timestampNow), (override));
        MOCK_METHOD(bool, sendKeepAlive, (RamsesInstanceId to, const SomeIPMsgHeader& header, uint64_t timestampNow), (override));

        MOCK_METHOD(bool, sendRendererEvent, (RamsesInstanceId to, const SomeIPMsgHeader& header, SceneId sceneId, const std::vector<Byte>& data), (override));
        MOCK_METHOD(bool, sendRequestResources, (RamsesInstanceId to, const SomeIPMsgHeader& header, absl::Span<const ResourceContentHash> resources), (override));
        MOCK_METHOD(bool, sendResourcesNotAvailable, (RamsesInstanceId to, const SomeIPMsgHeader& header, absl::Span<const ResourceContentHash> resources), (override));
        MOCK_METHOD(bool, sendResourceTransfer, (RamsesInstanceId to, const SomeIPMsgHeader& header, const std::vector<Byte>& resourceData), (override));
        MOCK_METHOD(bool, sendSceneUpdate, (RamsesInstanceId to, const SomeIPMsgHeader& header, SceneId sceneId, const std::vector<Byte>& sceneUpdate), (override));
        MOCK_METHOD(bool, sendSceneAvailabilityChange, (RamsesInstanceId to, const SomeIPMsgHeader& header, const std::vector<SceneAvailabilityUpdate>& update), (override));
        MOCK_METHOD(bool, sendSceneSubscriptionChange, (RamsesInstanceId to, const SomeIPMsgHeader& header, const std::vector<SceneSubscriptionUpdate>& update), (override));
        MOCK_METHOD(bool, sendInitializeScene, (RamsesInstanceId to, const SomeIPMsgHeader& header, SceneId sceneId), (override));
    };

}

#endif
