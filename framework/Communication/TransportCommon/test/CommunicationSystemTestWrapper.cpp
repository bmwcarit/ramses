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
#include "TransportCommon/IConnectionStatusUpdateNotifier.h"
#include "TestRandom.h"
#include "RamsesFrameworkConfigImpl.h"
#include <array>

namespace ramses_internal
{
    using namespace testing;

    void PrintTo(const ECommunicationSystemType& type, std::ostream* os)
    {
        switch (type)
        {
        case ECommunicationSystemType::Tcp:
            *os << "ECommunicationSystemType::Tcp";
            return;
        };
        *os << static_cast<int>(type) << " (INVALID ECommunicationSystemType)";
    }

    CommunicationSystemTestState::CommunicationSystemTestState(ECommunicationSystemType type, EServiceType serviceType_)
        : communicationSystemType(type)
        , serviceType(serviceType_)
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
        for(const auto& comSystem : knownCommunicationSystems)
        {
            assert(comSystem != nullptr);
            EXPECT_TRUE(comSystem->commSystem->connectServices());
        }
    }

    void CommunicationSystemTestState::disconnectAll()
    {
        for (const auto& comSystem : knownCommunicationSystems)
        {
            assert(comSystem != nullptr);
            comSystem->commSystem->disconnectServices();
        }
    }

    testing::AssertionResult CommunicationSystemTestState::blockOnAllConnected(uint32_t waitTimeMsOverride)
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
            assert(comSystem != nullptr);
            comSystem->registerForConnectionUpdates();
        }

        testing::AssertionResult result = event.waitForEvents(expectedEvents, waitTimeMsOverride);

        for (const auto& comSystem : knownCommunicationSystems)
        {
            comSystem->unregisterForConnectionUpdates();
        }

        return result;
    }

    std::vector<ECommunicationSystemType> CommunicationSystemTestState::GetAvailableCommunicationSystemTypes()
    {
        std::vector<ECommunicationSystemType> ret;
#if defined(HAS_TCP_COMM)
        ret.push_back(ECommunicationSystemType::Tcp);
#endif
        return ret;
    }

    CommunicationSystemTestWrapper::CommunicationSystemTestWrapper(CommunicationSystemTestState& state_, std::string_view name, const Guid& id_)
        : id(id_.isValid() ? id_ : Guid(TestRandom::Get(255, std::numeric_limits<size_t>::max())))
        , state(state_)
    {
        ramses::RamsesFrameworkConfigImpl config(ramses::EFeatureLevel_Latest);

        commSystem = CommunicationSystemFactory::ConstructCommunicationSystem(config, ParticipantIdentifier(id, name), frameworkLock, statisticCollection);
        state.knownCommunicationSystems.push_back(this);

        // trigger event on mock call
        ON_CALL(statusUpdateListener, newParticipantHasConnected(_)).WillByDefault(InvokeWithoutArgs([&]() { state.sendEvent(); }));
        ON_CALL(statusUpdateListener, participantHasDisconnected(_)).WillByDefault(InvokeWithoutArgs([&]() { state.sendEvent(); }));
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
        }
    }

    void CommunicationSystemTestWrapper::unregisterForConnectionUpdates()
    {
        switch (state.serviceType)
        {
        case EServiceType::Ramses:
            commSystem->getRamsesConnectionStatusUpdateNotifier().unregisterForConnectionUpdates(&statusUpdateListener);
            break;
        }
    }
}
