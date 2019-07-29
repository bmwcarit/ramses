//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Watchdog/PlatformWatchdog.h"
#include "Utils/LogMacros.h"
#include "PlatformAbstraction/PlatformTime.h"

namespace ramses_internal
{

    PlatformWatchdog::PlatformWatchdog(uint32_t notificationIntervalMilliSeconds, ramses::ERamsesThreadIdentifier thread, ramses::IThreadWatchdogNotification* callback)
        : m_intervalMilliSeconds(notificationIntervalMilliSeconds / 2)
        , m_thread(thread)
        , m_watchdogCallback(callback)
        , m_lastNotificationTimeMilliSeconds(0u)
    {
        if (nullptr != m_watchdogCallback)
        {
            m_watchdogCallback->registerThread(m_thread);
        }
    }

    PlatformWatchdog::~PlatformWatchdog()
    {
        if (nullptr != m_watchdogCallback)
        {
            m_watchdogCallback->unregisterThread(m_thread);
        }
    }

    void PlatformWatchdog::notifyWatchdog()
    {
        const uint64_t timeNowMilliSeconds = PlatformTime::GetMillisecondsMonotonic();
        if ( (timeNowMilliSeconds - m_lastNotificationTimeMilliSeconds) >= m_intervalMilliSeconds / 2)
        {
            LOG_TRACE(CONTEXT_FRAMEWORK, "notify watchdog from thread " << EnumToString(m_thread));
            if (nullptr != m_watchdogCallback)
            {
                m_watchdogCallback->notifyThread(m_thread);
            }
            m_lastNotificationTimeMilliSeconds = timeNowMilliSeconds;
        }
    }

    uint32_t PlatformWatchdog::calculateTimeout() const
    {
        if (m_lastNotificationTimeMilliSeconds == 0)
        {
            return m_intervalMilliSeconds;
        }

        const auto timeSinceLastNotify =
            static_cast<Int64>(PlatformTime::GetMillisecondsMonotonic() - m_lastNotificationTimeMilliSeconds);
        Int64 timeToNext = m_intervalMilliSeconds - timeSinceLastNotify;
        while (timeToNext <= 0)
        {
            timeToNext += m_intervalMilliSeconds;
        }
        return static_cast<uint32_t>(timeToNext);
    }
}
