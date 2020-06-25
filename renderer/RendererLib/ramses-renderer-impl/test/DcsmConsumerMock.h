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
#include "ramses-framework-api/CategoryInfoUpdate.h"

namespace ramses
{
    class IDcsmConsumerEventHandler;

    class DcsmConsumerMock : public IDcsmConsumerImpl
    {
    public:
        DcsmConsumerMock();

        MOCK_METHOD(status_t, dispatchEvents, (IDcsmConsumerEventHandler&), (override));
        MOCK_METHOD(status_t, assignContentToConsumer, (ContentID, const CategoryInfoUpdate&), (override));
        MOCK_METHOD(status_t, contentSizeChange, (ContentID, const CategoryInfoUpdate&, AnimationInformation), (override));
        MOCK_METHOD(status_t, contentStateChange, (ContentID, EDcsmState status, AnimationInformation), (override));
        MOCK_METHOD(status_t, acceptStopOffer, (ContentID, AnimationInformation), (override));
    };
}

#endif
