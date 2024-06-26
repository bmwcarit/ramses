//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Watchdog/ThreadWatchdog.h"
#include <algorithm>
#include <cassert>

namespace ramses::internal
{
    ThreadWatchdog::ThreadWatchdog(const ThreadWatchdogConfig& watchdogConfig, ERamsesThreadIdentifier identifier)
        : m_watchdogNotifier(std::chrono::milliseconds{ watchdogConfig.getWatchdogNotificationInterval(identifier) }, identifier, watchdogConfig.getCallBack())
    {
    }

    uint64_t ThreadWatchdog::registerThread()
    {
        std::lock_guard<std::mutex> g(m_aliveLock);
        m_watchedThreads[m_nextID] = false;
        return m_nextID++;
    }

    void ThreadWatchdog::unregisterThread(uint64_t identifier)
    {
        std::lock_guard<std::mutex> g(m_aliveLock);
        m_watchedThreads.erase(identifier);

        checkNotifyPlatform();
    }

    void ThreadWatchdog::notifyAlive(uint64_t identifier)
    {
        std::lock_guard<std::mutex> g(m_aliveLock);
        assert(m_watchedThreads.count(identifier) == 1);
        m_watchedThreads[identifier] = true;

        checkNotifyPlatform();
    }

    std::chrono::milliseconds ThreadWatchdog::calculateTimeout() const
    {
        std::lock_guard<std::mutex> g(m_aliveLock);
        return m_watchdogNotifier.calculateTimeout();
    }

    void ThreadWatchdog::checkNotifyPlatform()
    {
        if (m_watchedThreads.empty() || std::any_of(std::cbegin(m_watchedThreads), std::cend(m_watchedThreads), [](auto const& v) { return !v.second; }))
            return;

        m_watchdogNotifier.notifyWatchdog();
        for (auto& entry : m_watchedThreads)
            entry.second = false;
    }

}
