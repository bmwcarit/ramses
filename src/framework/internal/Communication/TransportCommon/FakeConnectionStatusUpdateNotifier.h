//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Communication/TransportCommon/IConnectionStatusUpdateNotifier.h"


namespace ramses::internal
{
    class IConnectionStatusListener;

    class FakeConnectionStatusUpdateNotifier : public IConnectionStatusUpdateNotifier
    {
    public:
        ~FakeConnectionStatusUpdateNotifier() override = default;

        void registerForConnectionUpdates(IConnectionStatusListener* /*listener*/) override
        {
        }

        void unregisterForConnectionUpdates(IConnectionStatusListener* /*listener*/) override
        {
        }
    };
}
