//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MOCKCONNECTIONSTATUSUPDATENOTIFIER_H
#define RAMSES_MOCKCONNECTIONSTATUSUPDATENOTIFIER_H

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "TransportCommon/IConnectionStatusUpdateNotifier.h"


namespace ramses_internal
{
    class MockConnectionStatusUpdateNotifier : public IConnectionStatusUpdateNotifier
    {
    public:
        MockConnectionStatusUpdateNotifier();
        ~MockConnectionStatusUpdateNotifier() override;

        MOCK_METHOD1(registerForConnectionUpdates, void(IConnectionStatusListener* listener));
        MOCK_METHOD1(unregisterForConnectionUpdates, void(IConnectionStatusListener* listener));
    };
}

#endif
