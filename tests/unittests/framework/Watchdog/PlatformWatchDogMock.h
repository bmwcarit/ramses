//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "ramses/framework/IThreadWatchdogNotification.h"

namespace ramses::internal
{
    class PlatformWatchdogMockCallback : public IThreadWatchdogNotification
    {
    public:
        MOCK_METHOD(void, notifyThread, (ERamsesThreadIdentifier), (override));
        MOCK_METHOD(void, registerThread, (ERamsesThreadIdentifier), (override));
        MOCK_METHOD(void, unregisterThread, (ERamsesThreadIdentifier), (override));
    };
}
