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
#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"

namespace ramses_internal
{
    class DcsmProviderEventHandlerMock : public IDcsmProviderEventHandler
    {
    public:
        DcsmProviderEventHandlerMock();
        ~DcsmProviderEventHandlerMock() override;

        MOCK_METHOD4(canvasSizeChange, void(ramses::ContentID, ramses::SizeInfo, ramses::AnimationInformation, const Guid&));
        MOCK_METHOD4(contentStatusChange, void(ramses::ContentID, ramses::EDcsmStatus, ramses::AnimationInformation, const Guid&));
    };

    class DcsmConsumerEventHandlerMock : public ramses::IDcsmConsumerEventHandler
    {
    public:
        DcsmConsumerEventHandlerMock();
        ~DcsmConsumerEventHandlerMock() override;

        MOCK_METHOD2(registerContent, void(ramses::ContentID, ramses::Category));
        MOCK_METHOD3(contentAvailable, void(ramses::ContentID, ramses::ETechnicalContentType, ramses::TechnicalContentDescriptor));
        MOCK_METHOD1(categoryContentSwitchRequest, void(ramses::ContentID));
        MOCK_METHOD1(requestUnregisterContent, void(ramses::ContentID));
        MOCK_METHOD1(forceUnregisterContent, void(ramses::ContentID));
    };
}

#endif
