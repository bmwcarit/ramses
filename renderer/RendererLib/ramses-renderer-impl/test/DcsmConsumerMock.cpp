//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DcsmConsumerMock.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"

using namespace testing;

namespace ramses
{
    DcsmConsumerMock::DcsmConsumerMock()
    {
        ON_CALL(*this, dispatchEvents(_)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, assignContentToConsumer(_, _)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, contentSizeChange(_, _, _)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, contentStateChange(_, _, _)).WillByDefault(Return(StatusOK));
        ON_CALL(*this, acceptStopOffer(_, _)).WillByDefault(Return(StatusOK));
    }
}
