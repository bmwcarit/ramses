//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMWATCHDOGMOCK_H
#define RAMSES_PLATFORMWATCHDOGMOCK_H

#include "gmock/gmock.h"
#include "ramses-framework-api/IThreadWatchdogNotification.h"

namespace ramses_internal
{
    class PlatformWatchdogMockCallback : public ramses::IThreadWatchdogNotification
    {
    public:
        MOCK_METHOD(void, notifyThread, (ramses::ERamsesThreadIdentifier), (override));
        MOCK_METHOD(void, registerThread, (ramses::ERamsesThreadIdentifier), (override));
        MOCK_METHOD(void, unregisterThread, (ramses::ERamsesThreadIdentifier), (override));
    };
}

#endif
