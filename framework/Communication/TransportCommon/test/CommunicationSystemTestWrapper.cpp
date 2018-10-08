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
#include "Common/Cpp11Macros.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"


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

    CommunicationSystemTestState::CommunicationSystemTestState(ECommunicationSystemType type)
        : communicationSystemType(type)
    {
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
        ramses_foreach(knownCommunicationSystems, it)
        {
            CommunicationSystemTestWrapper* csw = *it;
            assert(csw != 0);
            EXPECT_TRUE(csw->commSystem->connectServices());
        }

        startUpHook();
    }

    void CommunicationSystemTestState::disconnectAll()
    {
        ramses_foreach(knownCommunicationSystems, it)
        {
            CommunicationSystemTestWrapper* csw = *it;
            assert(csw != 0);
            csw->commSystem->disconnectServices();
        }

        shutdownHook();
    }

    testing::AssertionResult CommunicationSystemTestState::blockOnAllConnected(UInt32 waitTimeMsOverride)
    {
        uint32_t expectedEvents = 0;
        ramses_foreach(knownCommunicationSystems, it_outer)
        {
            ramses_foreach(knownCommunicationSystems, it_inner)
            {
                if (*it_outer != *it_inner)
                {
                    EXPECT_CALL((*it_outer)->statusUpdateListener, newParticipantHasConnected((*it_inner)->id));
                    ++expectedEvents;
                }
            }
        }

        ramses_foreach(knownCommunicationSystems, it)
        {
            CommunicationSystemTestWrapper* csw = *it;
            assert(csw != 0);
            csw->registerForConnectionUpdates();
        }

        testing::AssertionResult result = event.waitForEvents(expectedEvents, waitTimeMsOverride);

        ramses_foreach(knownCommunicationSystems, it)
        {
            (*it)->unregisterForConnectionUpdates();
        }

        return result;
    }

    Vector<ECommunicationSystemType> CommunicationSystemTestState::GetAvailableCommunicationSystemTypes(uint32_t mask)
    {
        UNUSED(mask);

        Vector<ECommunicationSystemType> ret;
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
        const auto it = state.knownCommunicationSystems.find(this);
        assert(it != state.knownCommunicationSystems.end());
        state.knownCommunicationSystems.erase(it);
    }

    void CommunicationSystemTestWrapper::registerForConnectionUpdates()
    {
        commSystem->getConnectionStatusUpdateNotifier().registerForConnectionUpdates(&statusUpdateListener);
    }

    void CommunicationSystemTestWrapper::unregisterForConnectionUpdates()
    {
        commSystem->getConnectionStatusUpdateNotifier().unregisterForConnectionUpdates(&statusUpdateListener);
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
