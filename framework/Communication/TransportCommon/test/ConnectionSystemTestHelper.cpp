//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ConnectionSystemTestHelper.h"
#include "TransportCommon/CommunicationSystemFactory.h"
#include "TransportCommon/IDiscoveryDaemon.h"
#include "RamsesFrameworkConfigImpl.h"

namespace ramses_internal
{
    AsyncEventCounter::AsyncEventCounter(UInt32 waitTimeMs)
        : m_eventCounter(0)
        , m_waitTimeMs(waitTimeMs)
    {
    }

    void AsyncEventCounter::signal()
    {
        std::lock_guard<std::mutex> g(lock);
        ++m_eventCounter;
        cond.notify_one();
    }

    testing::AssertionResult AsyncEventCounter::waitForEvents(UInt32 numberEventsToWaitFor, UInt32 waitTimeMsOverride)
    {
        std::unique_lock<std::mutex> l(lock);
        if (cond.wait_for(l,
                          std::chrono::milliseconds{(waitTimeMsOverride > 0 ? waitTimeMsOverride : m_waitTimeMs)},
                          [&]() { return m_eventCounter >= numberEventsToWaitFor; }))
        {
            // only consume requested number of events
            m_eventCounter -= numberEventsToWaitFor;
            return testing::AssertionSuccess();
        }
        else
        {
            // clear all events on timeout
            const auto currentEventCounter = m_eventCounter;
            m_eventCounter = 0;
            return testing::AssertionFailure() << "Timeout while waiting for SyncEvents (waitForEvents): Expected " << numberEventsToWaitFor << ", got " << currentEventCounter;
        }
    }

    void AsyncEventCounter::discardPendingEvents()
    {
        std::unique_lock<std::mutex> l(lock);
        m_eventCounter = 0;
    }

    ConnectionSystemTestDaemon::ConnectionSystemTestDaemon(const std::function<void(ramses::RamsesFrameworkConfigImpl&)>& configModifier)
    {
        ramses::RamsesFrameworkConfigImpl config;
        config.enableProtocolVersionOffset();
        if (configModifier)
            configModifier(config);
        daemon = CommunicationSystemFactory::ConstructDiscoveryDaemon(config, frameworkLock, statisticCollection);
        assert(daemon);
    }

    ConnectionSystemTestDaemon::~ConnectionSystemTestDaemon() = default;

    bool ConnectionSystemTestDaemon::start()
    {
        return daemon->start();
    }

    bool ConnectionSystemTestDaemon::stop()
    {
        return daemon->stop();
    }
}
