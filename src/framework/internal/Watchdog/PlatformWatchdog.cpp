//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Watchdog/PlatformWatchdog.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/PlatformAbstraction/PlatformTime.h"

using namespace std::chrono_literals;
namespace ramses::internal
{

    PlatformWatchdog::PlatformWatchdog(std::chrono::milliseconds notificationInterval, ERamsesThreadIdentifier thread, IThreadWatchdogNotification* callback)
        : m_interval(notificationInterval / 2)
        , m_thread(thread)
        , m_watchdogCallback(callback)
        , m_lastNotificationTime(0ms)
    {
        if (nullptr != m_watchdogCallback)
            m_watchdogCallback->registerThread(m_thread);
    }

    PlatformWatchdog::~PlatformWatchdog()
    {
        if (nullptr != m_watchdogCallback)
            m_watchdogCallback->unregisterThread(m_thread);
    }

    void PlatformWatchdog::notifyWatchdog()
    {
        const std::chrono::milliseconds timeNow{ PlatformTime::GetMillisecondsMonotonic() };
        if ((timeNow - m_lastNotificationTime) >= (m_interval / 2) || m_lastNotificationTime == 0ms) // always allow first notification
        {
            if (nullptr != m_watchdogCallback)
            {
                LOG_TRACE(CONTEXT_FRAMEWORK, "notify watchdog from thread {}", EnumToString(m_thread));
                m_watchdogCallback->notifyThread(m_thread);
            }
            m_lastNotificationTime = timeNow;
        }
    }

    std::chrono::milliseconds PlatformWatchdog::calculateTimeout() const
    {
        if (m_lastNotificationTime == 0ms || m_interval == 0ms)
            return m_interval;

        const auto timeSinceLastNotify =
            std::chrono::milliseconds(PlatformTime::GetMillisecondsMonotonic()) - m_lastNotificationTime;
        std::chrono::milliseconds timeToNext = m_interval - timeSinceLastNotify;
        while (timeToNext <= 0ms)
        {
            timeToNext += m_interval;
        }
        return timeToNext;
    }
}
