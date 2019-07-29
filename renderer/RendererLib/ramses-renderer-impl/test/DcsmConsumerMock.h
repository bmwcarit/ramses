//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMCONSUMERMOCK_H
#define RAMSES_DCSMCONSUMERMOCK_H

#include "gmock/gmock.h"
#include "IDcsmConsumerImpl.h"

namespace ramses
{
    class IDcsmConsumerEventHandler;

    class DcsmConsumerMock : public IDcsmConsumerImpl
    {
    public:
        DcsmConsumerMock();

        MOCK_METHOD1(dispatchEvents, status_t(IDcsmConsumerEventHandler&));
        MOCK_METHOD2(assignContentToConsumer, status_t(ContentID, SizeInfo));
        MOCK_METHOD3(contentSizeChange, status_t(ContentID, SizeInfo, AnimationInformation));
        MOCK_METHOD3(contentStateChange, status_t(ContentID, EDcsmState status, AnimationInformation));
        MOCK_METHOD2(acceptStopOffer, status_t(ContentID, AnimationInformation));
    };
}

#endif
