//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMABSTRACTION_PLATFORMWATCHDOG_H
#define RAMSES_PLATFORMABSTRACTION_PLATFORMWATCHDOG_H

#include "ramses-framework-api/IThreadWatchdogNotification.h"
#include "Collections/String.h"

namespace ramses_internal
{
    inline
    static String EnumToString(const ramses::ERamsesThreadIdentifier& thread)
    {
        switch (thread)
        {
            case ramses::ERamsesThreadIdentifier_Workers: return "ERamsesThread_Workers";
            case ramses::ERamsesThreadIdentifier_Renderer: return "ERamsesThread_Renderer";
            default: return "ERamsesThread_Unknown";
        }
    }

    class PlatformWatchdog
    {
    public:
        PlatformWatchdog(uint32_t notificationIntervalMilliSeconds, ramses::ERamsesThreadIdentifier thread, ramses::IThreadWatchdogNotification* callback);
        virtual ~PlatformWatchdog();

        void     notifyWatchdog();
        uint32_t calculateTimeout() const;

    private:
        const uint32_t m_intervalMilliSeconds;
        ramses::ERamsesThreadIdentifier m_thread;
        ramses::IThreadWatchdogNotification* m_watchdogCallback;
        uint64_t m_lastNotificationTimeMilliSeconds;
    };
}

#endif
