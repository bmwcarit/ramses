//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_THREADWATCHDOG_H
#define RAMSES_THREADWATCHDOG_H

#include "ThreadWatchdogConfig.h"
#include "IThreadAliveNotifier.h"
#include "Watchdog/PlatformWatchdog.h"
#include <mutex>
#include <unordered_map>

namespace ramses_internal
{
    class ThreadWatchdog : public IThreadAliveNotifier
    {
    public:
        ThreadWatchdog(const ThreadWatchdogConfig& watchdogConfig, ramses::ERamsesThreadIdentifier identifier);

        uint64_t registerThread() final override;
        void unregisterThread(uint64_t identifier) final override;

        void notifyAlive(uint64_t identifier) final override;
        std::chrono::milliseconds calculateTimeout() const final override;

    private:
        void checkNotifyPlatform();

        mutable std::mutex                  m_aliveLock;
        uint64_t                            m_nextID = 0;
        std::unordered_map<uint64_t, bool>  m_watchedThreads;
        PlatformWatchdog                    m_watchdogNotifier;
    };
}

#endif
