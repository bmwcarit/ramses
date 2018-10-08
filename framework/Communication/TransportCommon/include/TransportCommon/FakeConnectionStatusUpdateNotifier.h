//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FAKECONNECTIONSTATUSUPDATENOTIFIER_H
#define RAMSES_FAKECONNECTIONSTATUSUPDATENOTIFIER_H

#include "TransportCommon/IConnectionStatusUpdateNotifier.h"


namespace ramses_internal
{
    class IConnectionStatusListener;

    class FakeConnectionStatusUpdateNotifier : public IConnectionStatusUpdateNotifier
    {
    public:
        ~FakeConnectionStatusUpdateNotifier() override
        {
        }

        virtual void registerForConnectionUpdates(IConnectionStatusListener*) override
        {
        }

        virtual void unregisterForConnectionUpdates(IConnectionStatusListener*) override
        {
        }
    };
}

#endif
