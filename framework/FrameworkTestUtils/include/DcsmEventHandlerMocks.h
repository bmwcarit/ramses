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

        MOCK_METHOD3(contentSizeChange, void(ramses::ContentID, ramses::SizeInfo, ramses::AnimationInformation));
        MOCK_METHOD4(contentStateChange, void(ramses::ContentID, ramses_internal::EDcsmState, ramses::SizeInfo, ramses::AnimationInformation));
    };

    class DcsmConsumerEventHandlerMock : public ramses::IDcsmConsumerEventHandler
    {
    public:
        DcsmConsumerEventHandlerMock();
        ~DcsmConsumerEventHandlerMock() override;

        MOCK_METHOD2(contentOffered, void(ramses::ContentID, ramses::Category));
        MOCK_METHOD3(contentReady, void(ramses::ContentID, ramses::ETechnicalContentType, ramses::TechnicalContentDescriptor));
        MOCK_METHOD1(contentFocusRequest, void(ramses::ContentID));
        MOCK_METHOD1(contentStopOfferRequest, void(ramses::ContentID));
        MOCK_METHOD1(forceContentOfferStopped, void(ramses::ContentID));
    };
}

namespace ramses
{
    class DcsmProviderEventHandlerMock : public IDcsmProviderEventHandler
    {
    public:
        DcsmProviderEventHandlerMock();
        ~DcsmProviderEventHandlerMock() override;

        MOCK_METHOD2(contentHide, void(ContentID, AnimationInformation));
        MOCK_METHOD2(contentShow, void(ContentID, AnimationInformation));
        MOCK_METHOD2(contentRelease, void(ContentID, AnimationInformation));
        MOCK_METHOD3(contentSizeChange, void(ContentID, SizeInfo, AnimationInformation));
        MOCK_METHOD2(stopOfferAccepted, void(ContentID, AnimationInformation));
        MOCK_METHOD1(contentReadyRequested, void(ContentID));
    };
}

#endif
