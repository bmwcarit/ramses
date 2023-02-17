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
#include <chrono>

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
        PlatformWatchdog(std::chrono::milliseconds notificationInterval, ramses::ERamsesThreadIdentifier thread, ramses::IThreadWatchdogNotification* callback);
        virtual ~PlatformWatchdog();

        void                      notifyWatchdog();
        [[nodiscard]] std::chrono::milliseconds calculateTimeout() const;

    private:
        const std::chrono::milliseconds m_interval;
        ramses::ERamsesThreadIdentifier m_thread;
        ramses::IThreadWatchdogNotification* m_watchdogCallback;
        std::chrono::milliseconds m_lastNotificationTime;
    };
}

#endif
