//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ISOMEIPDCSMSTACK_H
#define RAMSES_ISOMEIPDCSMSTACK_H

#include "Components/DcsmTypes.h"
#include "TransportCommon/SomeIPStackCommon.h"
#include "absl/types/span.h"
#include <vector>
#include <cstdint>

namespace ramses_internal
{
    class CategoryInfo;

    using DcsmInstanceId = StronglyTypedValue<uint16_t, 0xFFFF, struct DcsmInstanceIdTag>;

    class ISomeIPDcsmStackCallbacks : public ISomeIPStackCallbacksCommon<DcsmInstanceId>
    {
    public:
        virtual void handleOfferContent(const SomeIPMsgHeader& header, ContentID content, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName, uint32_t sortOrder) = 0;
        virtual void handleRequestStopOfferContent(const SomeIPMsgHeader& header, ContentID content, bool forceStopOffer) = 0;
        virtual void handleUpdateContentMetadata(const SomeIPMsgHeader& header, ContentID content, absl::Span<const Byte> metadata) = 0;

        virtual void handleCanvasSizeChange(const SomeIPMsgHeader& header, ContentID content, const CategoryInfo& categoryInfo, uint16_t dpi, const AnimationInformation& animation) = 0;
        virtual void handleContentDescription(const SomeIPMsgHeader& header, ContentID content, TechnicalContentDescriptor technicalContentDescriptor) = 0;
        virtual void handleContentReady(const SomeIPMsgHeader& header, ContentID content) = 0;
        virtual void handleContentStateChange(const SomeIPMsgHeader& header, ContentID content, EDcsmState state, const CategoryInfo& sizeInfo, const AnimationInformation& animation) = 0;
        virtual void handleContentEnableFocusRequest(const SomeIPMsgHeader& header, ContentID content, int32_t focusRequest) = 0;
        virtual void handleContentDisableFocusRequest(const SomeIPMsgHeader& header, ContentID content, int32_t focusRequest) = 0;
        virtual void handleResponse(const SomeIPMsgHeader& header, uint64_t originalMessageId, uint64_t originalSessionId, uint64_t responseCode) = 0;
    };

    struct DcsmStackSendDataSizes
    {
        uint32_t metadataSize;
    };

    class ISomeIPDcsmStack : public ISomeIPStackCommon<DcsmInstanceId>
    {
    public:
        virtual void setCallbacks(ISomeIPDcsmStackCallbacks* handlers) = 0;
        virtual DcsmStackSendDataSizes getSendDataSizes() const = 0;

        virtual bool sendOfferContent(DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, Category category, ETechnicalContentType technicalContentType, const std::string& friendlyName, uint32_t sortOrder) = 0;
        virtual bool sendRequestStopOfferContent(DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, bool forceStopOffer) = 0;
        virtual bool sendUpdateContentMetadata(DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, const std::vector<Byte>& data) = 0;

        virtual bool sendCanvasSizeChange(DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, const CategoryInfo& categoryInfo, uint16_t dpi, const AnimationInformation& animation) = 0;
        virtual bool sendContentDescription(DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, TechnicalContentDescriptor technicalContentDescriptor) = 0;
        virtual bool sendContentReady(DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content) = 0;
        virtual bool sendContentStateChange(DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, EDcsmState state, const CategoryInfo& sizeInfo, const AnimationInformation& animation) = 0;
        virtual bool sendContentEnableFocusRequest(DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, int32_t focusRequest) = 0;
        virtual bool sendContentDisableFocusRequest(DcsmInstanceId to, const SomeIPMsgHeader& header, ContentID content, int32_t focusRequest) = 0;
        virtual bool sendResponse(DcsmInstanceId to, const SomeIPMsgHeader& header, uint64_t originalMessageId, uint64_t originalSessionId, uint64_t responseCode) = 0;
    };
}

MAKE_STRONGLYTYPEDVALUE_PRINTABLE(ramses_internal::DcsmInstanceId)

#endif
