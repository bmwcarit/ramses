//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "CommunicationSystemTestWrapper.h"
#include "TransportCommon/CommunicationSystemFactory.h"
#include "Common/ParticipantIdentifier.h"
#include "Utils/CommandLineParser.h"
#include "Utils/File.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "TransportCommon/IDiscoveryDaemon.h"
#include "TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include <array>

namespace ramses_internal
{
    using namespace testing;

    void PrintTo(const ECommunicationSystemType& type, std::ostream* os)
    {
        switch (type)
        {
        case ECommunicationSystemType_Tcp:
            *os << type << " (ECommunicationSystemType_Tcp)";
            break;
        default:
            *os << type << " (INVALID ECommunicationSystemType)";
        }
    }

    AsyncEventCounter::AsyncEventCounter(UInt32 waitTimeMs)
        : m_eventCounter(0)
        , m_waitTimeMs(waitTimeMs)
    {
    }

    void AsyncEventCounter::signal()
    {
        PlatformLightweightGuard g(lock);
        ++m_eventCounter;
        cond.signal();
    }

    testing::AssertionResult AsyncEventCounter::waitForEvents(UInt32 numberEventsToWaitFor, UInt32 waitTimeMsOverride)
    {
        PlatformLightweightGuard g(lock);
        bool abortWaiting = false;
        UInt32 waitMs = (waitTimeMsOverride > 0 ? waitTimeMsOverride : m_waitTimeMs);

        while (m_eventCounter < numberEventsToWaitFor && !abortWaiting)
        {
            abortWaiting = (cond.wait(&lock, waitMs) != EStatus_RAMSES_OK);
        }
        if (abortWaiting)
        {
            // clear all events on timeout
            m_eventCounter = 0;
            return testing::AssertionFailure() << "Timeout while waiting for SyncEvents (waitForEvents)";
        }
        else
        {
            // only consume requested number of events
            m_eventCounter -= numberEventsToWaitFor;
            return testing::AssertionSuccess();
        }
    }

    CommunicationSystemTestState::CommunicationSystemTestState(ECommunicationSystemType type, EServiceType serviceType_)
        : communicationSystemType(type)
        , serviceType(serviceType_)
    {
        static bool initializedLogger = false;
        if (!initializedLogger)
        {
            initializedLogger = true;
            std::array<const char *const, 3> args{"", "-l", "warn"};
            GetRamsesLogger().initialize(CommandLineParser{static_cast<Int>(args.size()), args.data()}, "TEST", "TEST", true);
        }
    }

    void CommunicationSystemTestState::sendEvent()
    {
        event.signal();
    }

    CommunicationSystemTestState::~CommunicationSystemTestState()
    {
        assert(knownCommunicationSystems.empty());
    }

    void CommunicationSystemTestState::connectAll()
    {
        for(const auto& comSystem : knownCommunicationSystems)
        {
            assert(comSystem != 0);
            EXPECT_TRUE(comSystem->commSystem->connectServices());
        }

        startUpHook();
    }

    void CommunicationSystemTestState::disconnectAll()
    {
        for (const auto& comSystem : knownCommunicationSystems)
        {
            assert(comSystem != 0);
            comSystem->commSystem->disconnectServices();
        }

        shutdownHook();
    }

    testing::AssertionResult CommunicationSystemTestState::blockOnAllConnected(UInt32 waitTimeMsOverride)
    {
        uint32_t expectedEvents = 0;
        for (const auto& comSystemOuter : knownCommunicationSystems)
        {
            for (const auto& comSystemInner : knownCommunicationSystems)
            {
                if (comSystemOuter != comSystemInner)
                {
                    EXPECT_CALL(comSystemOuter->statusUpdateListener, newParticipantHasConnected(comSystemInner->id));
                    ++expectedEvents;
                }
            }
        }

        for (const auto& comSystem : knownCommunicationSystems)
        {
            assert(comSystem != 0);
            comSystem->registerForConnectionUpdates();
        }

        testing::AssertionResult result = event.waitForEvents(expectedEvents, waitTimeMsOverride);

        for (const auto& comSystem : knownCommunicationSystems)
        {
            comSystem->unregisterForConnectionUpdates();
        }

        return result;
    }

    std::vector<ECommunicationSystemType> CommunicationSystemTestState::GetAvailableCommunicationSystemTypes(uint32_t mask)
    {
        UNUSED(mask);

        std::vector<ECommunicationSystemType> ret;
#if defined(HAS_TCP_COMM)
        if (mask & ECommunicationSystemType_Tcp)
        {
            ret.push_back(ECommunicationSystemType_Tcp);
        }
#endif
        return ret;
    }

    CommunicationSystemTestWrapper::CommunicationSystemTestWrapper(CommunicationSystemTestState& state_, const String& name, const Guid& id_,
        ECommunicationSystemTestConfiguration commSysConfig_)
        : id(id_)
        , state(state_)
    {
        ramses::RamsesFrameworkConfigImpl config(0, NULL);
        config.enableProtocolVersionOffset();
        state.applyConfigurationForSelectedConnectionSystemType(config, false, commSysConfig_);

        commSystem.reset(CommunicationSystemFactory::ConstructCommunicationSystem(config, ParticipantIdentifier(id, name), frameworkLock, statisticCollection));
        state.knownCommunicationSystems.push_back(this);

        // trigger event on mock call
        ON_CALL(statusUpdateListener, newParticipantHasConnected(_)).WillByDefault(SendHandlerCalledEvent(&state));
        ON_CALL(statusUpdateListener, participantHasDisconnected(_)).WillByDefault(SendHandlerCalledEvent(&state));
    }

    CommunicationSystemTestWrapper::~CommunicationSystemTestWrapper()
    {
        const auto it = find_c(state.knownCommunicationSystems, this);
        assert(it != state.knownCommunicationSystems.end());
        state.knownCommunicationSystems.erase(it);
    }

    void CommunicationSystemTestWrapper::registerForConnectionUpdates()
    {
        switch (state.serviceType)
        {
        case EServiceType::Ramses:
            commSystem->getRamsesConnectionStatusUpdateNotifier().registerForConnectionUpdates(&statusUpdateListener);
            break;
        case EServiceType::Dcsm:
            commSystem->getDcsmConnectionStatusUpdateNotifier().registerForConnectionUpdates(&statusUpdateListener);
            break;
        }
    }

    void CommunicationSystemTestWrapper::unregisterForConnectionUpdates()
    {
        switch (state.serviceType)
        {
        case EServiceType::Ramses:
            commSystem->getRamsesConnectionStatusUpdateNotifier().unregisterForConnectionUpdates(&statusUpdateListener);
            break;
        case EServiceType::Dcsm:
            commSystem->getDcsmConnectionStatusUpdateNotifier().unregisterForConnectionUpdates(&statusUpdateListener);
            break;
        }
    }

    CommunicationSystemDiscoveryDaemonTestWrapper::CommunicationSystemDiscoveryDaemonTestWrapper(CommunicationSystemTestState& state_)
    {
        ramses::RamsesFrameworkConfigImpl config(0, NULL);
        config.enableProtocolVersionOffset();
        state_.applyConfigurationForSelectedConnectionSystemType(config, true, ECommunicationSystemTestConfiguration_Default);

        daemon.reset(CommunicationSystemFactory::ConstructDiscoveryDaemon(config, frameworkLock, statisticCollection));
    }

    CommunicationSystemDiscoveryDaemonTestWrapper::~CommunicationSystemDiscoveryDaemonTestWrapper()
    {
    }

    bool CommunicationSystemDiscoveryDaemonTestWrapper::start()
    {
        assert(daemon.get());
        return daemon->start();
    }

    bool CommunicationSystemDiscoveryDaemonTestWrapper::stop()
    {
        assert(daemon.get());
        return daemon->stop();
    }
}
