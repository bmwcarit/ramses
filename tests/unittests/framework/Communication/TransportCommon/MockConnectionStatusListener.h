//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"

#include "internal/Communication/TransportCommon/IConnectionStatusListener.h"

namespace ramses::internal
{
    class MockConnectionStatusListener : public IConnectionStatusListener
    {
    public:
        MockConnectionStatusListener();
        ~MockConnectionStatusListener() override;

        MOCK_METHOD(void, newParticipantHasConnected, (const Guid& guid), (override));
        MOCK_METHOD(void, participantHasDisconnected, (const Guid& guid), (override));
    };
}
