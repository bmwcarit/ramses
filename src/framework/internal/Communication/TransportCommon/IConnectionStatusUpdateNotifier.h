//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

namespace ramses::internal
{
    class IConnectionStatusListener;

    class IConnectionStatusUpdateNotifier
    {
    public:
        virtual ~IConnectionStatusUpdateNotifier() = default;

        virtual void registerForConnectionUpdates(IConnectionStatusListener* listener) = 0;
        virtual void unregisterForConnectionUpdates(IConnectionStatusListener* listener) = 0;
    };
}
