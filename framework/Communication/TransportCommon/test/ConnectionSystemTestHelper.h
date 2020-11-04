//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORK_CONNECTIONSYSTEMTESTHELPER_H
#define RAMSES_FRAMEWORK_CONNECTIONSYSTEMTESTHELPER_H

#include "Utils/StatisticCollection.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "gtest/gtest.h"
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>

namespace ramses
{
    class RamsesFrameworkConfigImpl;
}

namespace ramses_internal
{
    class IDiscoveryDaemon;

    class AsyncEventCounter
    {
    public:
        explicit AsyncEventCounter(UInt32 waitTimeMs = 20000);

        void signal();
        testing::AssertionResult waitForEvents(UInt32 numberEventsToWaitFor, UInt32 waitTimeMsOverride = 0);

        void discardPendingEvents();

    private:
        std::mutex lock;
        std::condition_variable cond;
        UInt32 m_eventCounter;
        UInt32 m_waitTimeMs;
    };

    class ConnectionSystemTestDaemon
    {
    public:
        explicit ConnectionSystemTestDaemon(const std::function<void(ramses::RamsesFrameworkConfigImpl&)>& configModifier = {});
        ~ConnectionSystemTestDaemon();

        bool start();
        bool stop();
    private:
        std::unique_ptr<IDiscoveryDaemon> daemon;
        PlatformLock frameworkLock;
        StatisticCollectionFramework statisticCollection;
    };
}

#endif
