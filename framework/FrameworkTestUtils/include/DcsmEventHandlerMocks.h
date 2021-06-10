//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_DCSMEVENTHANDLERMOCKS_H
#define RAMSES_DCSMEVENTHANDLERMOCKS_H

#include "Components/IDcsmProviderEventHandler.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"
#include "ramses-framework-api/IDcsmProviderEventHandler.h"
#include "ramses-framework-api/DcsmMetadataUpdate.h"
#include "framework_common_gmock_header.h"
#include "DcsmGmockPrinter.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    class DcsmProviderEventHandlerMock : public IDcsmProviderEventHandler
    {
    public:
        DcsmProviderEventHandlerMock();
        ~DcsmProviderEventHandlerMock() override;

        MOCK_METHOD(void, contentSizeChange, (ramses::ContentID, const ramses::CategoryInfoUpdate&, ramses::AnimationInformation), (override));
        MOCK_METHOD(void, contentStateChange, (ramses::ContentID, ramses_internal::EDcsmState, const ramses::CategoryInfoUpdate&, ramses::AnimationInformation), (override));
        MOCK_METHOD(void, contentStatus, (ramses::ContentID, std::unique_ptr<ramses::DcsmStatusMessageImpl>&&), (override));
    };

    class DcsmConsumerEventHandlerMock : public ramses::IDcsmConsumerEventHandler
    {
    public:
        DcsmConsumerEventHandlerMock();
        ~DcsmConsumerEventHandlerMock() override;

        MOCK_METHOD(void, contentOffered, (ramses::ContentID, ramses::Category, ramses::ETechnicalContentType), (override));
        MOCK_METHOD(void, contentDescription, (ramses::ContentID, ramses::TechnicalContentDescriptor), (override));
        MOCK_METHOD(void, contentReady, (ramses::ContentID), (override));
        MOCK_METHOD(void, contentEnableFocusRequest, (ramses::ContentID, int32_t focusRequest), (override));
        MOCK_METHOD(void, contentDisableFocusRequest, (ramses::ContentID, int32_t focusRequest), (override));
        MOCK_METHOD(void, contentStopOfferRequest, (ramses::ContentID), (override));
        MOCK_METHOD(void, forceContentOfferStopped, (ramses::ContentID), (override));
        MOCK_METHOD(void, contentMetadataUpdated, (ramses::ContentID contentID, const ramses::DcsmMetadataUpdate& metadataUpdate), (override));
    };
}

namespace ramses
{
    class DcsmProviderEventHandlerMock : public IDcsmProviderEventHandlerExtended
    {
    public:
        DcsmProviderEventHandlerMock();
        ~DcsmProviderEventHandlerMock() override;

        MOCK_METHOD(void, contentHide, (ContentID, AnimationInformation), (override));
        MOCK_METHOD(void, contentShow, (ContentID, AnimationInformation), (override));
        MOCK_METHOD(void, contentRelease, (ContentID, AnimationInformation), (override));
        MOCK_METHOD(void, contentSizeChange, (ContentID, const CategoryInfoUpdate&, AnimationInformation), (override));
        MOCK_METHOD(void, stopOfferAccepted, (ContentID, AnimationInformation), (override));
        MOCK_METHOD(void, contentReadyRequested, (ContentID), (override));
        MOCK_METHOD(void, contentStatus, (ContentID, DcsmStatusMessage const&), (override));
    };
}

#endif
