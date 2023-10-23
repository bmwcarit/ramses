//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/StatisticCollection.h"
#include "internal/PlatformAbstraction/PlatformLock.h"
#include "gtest/gtest.h"
#include <mutex>
#include <condition_variable>
#include <memory>
#include <functional>


namespace ramses::internal
{
    class RamsesFrameworkConfigImpl;

    class IDiscoveryDaemon;

    class AsyncEventCounter
    {
    public:
        explicit AsyncEventCounter(uint32_t waitTimeMs = 20000);

        void signal();
        testing::AssertionResult waitForEvents(uint32_t numberEventsToWaitFor, uint32_t waitTimeMsOverride = 0);

        void discardPendingEvents();

    private:
        std::mutex lock;
        std::condition_variable cond;
        uint32_t m_eventCounter{0};
        uint32_t m_waitTimeMs;
    };

    class ConnectionSystemTestDaemon
    {
    public:
        explicit ConnectionSystemTestDaemon(const std::function<void(RamsesFrameworkConfigImpl&)>& configModifier = {});
        ~ConnectionSystemTestDaemon();

        bool start();
        bool stop();
    private:
        std::unique_ptr<IDiscoveryDaemon> daemon;
        PlatformLock frameworkLock;
        StatisticCollectionFramework statisticCollection;
    };
}
