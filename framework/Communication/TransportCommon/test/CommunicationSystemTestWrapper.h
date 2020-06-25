//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMMUNICATIONSYSTEMTESTWRAPPER_H
#define RAMSES_COMMUNICATIONSYSTEMTESTWRAPPER_H

#include "MockConnectionStatusListener.h"
#include "TransportCommon/ICommunicationSystem.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "RamsesFrameworkConfigImpl.h"
#include "Common/BitForgeMacro.h"
#include "Utils/StatisticCollection.h"
#include "ServiceHandlerMocks.h"
#include <memory>
#include <mutex>
#include <condition_variable>

namespace ramses_internal
{
    class CommandLineParser;
    class IDiscoveryDaemon;

    enum ECommunicationSystemType
    {
        ECommunicationSystemType_Tcp = BIT(0u),
        ECommunicationSystemType_All = BIT(3u) - 1u
    };

    enum ECommunicationSystemTestConfiguration
    {
        //default configuration with auto-generated instance ids
        ECommunicationSystemTestConfiguration_Default = 0,
        ECommunicationSystemTestConfiguration_LimitedInstances1,
        ECommunicationSystemTestConfiguration_LimitedInstances2,
        ECommunicationSystemTestConfiguration_ParticipantInfoMismatch1,
        ECommunicationSystemTestConfiguration_ParticipantInfoMismatch2,
        ECommunicationSystemTestConfiguration_LimitedClientInstances1,
        ECommunicationSystemTestConfiguration_LimitedClientInstances2
    };

    enum class EServiceType
    {
        Ramses,
        Dcsm
    };

    // better output from gtest
    void PrintTo(const ECommunicationSystemType& type, std::ostream* os);

    class CommunicationSystemTestWrapper;

    class AsyncEventCounter
    {
    public:
        explicit AsyncEventCounter(UInt32 waitTimeMs = 20000);

        void signal();
        testing::AssertionResult waitForEvents(UInt32 numberEventsToWaitFor, UInt32 waitTimeMsOverride = 0);

    private:
        std::mutex lock;
        std::condition_variable cond;
        UInt32 m_eventCounter;
        UInt32 m_waitTimeMs;
    };

    class CommunicationSystemTestState
    {
    public:
        explicit CommunicationSystemTestState(ECommunicationSystemType type, EServiceType serviceType);
        virtual ~CommunicationSystemTestState();

        void connectAll();
        void disconnectAll();
        testing::AssertionResult blockOnAllConnected(UInt32 waitTimeMsOverride = 0);
        virtual void startUpHook() {};
        virtual void shutdownHook() {}
        virtual void applyConfigurationForSelectedConnectionSystemType(ramses::RamsesFrameworkConfigImpl& /*config*/, bool /*isDaemon*/, ECommunicationSystemTestConfiguration /*commSysConfig*/) {}

        void sendEvent();

        static std::vector<ECommunicationSystemType> GetAvailableCommunicationSystemTypes(uint32_t mask = ECommunicationSystemType_All);
        ECommunicationSystemType communicationSystemType;
        EServiceType serviceType;
        AsyncEventCounter event;
        std::vector<CommunicationSystemTestWrapper*> knownCommunicationSystems;
    };

    class CommunicationSystemTestWrapper
    {
    public:
        CommunicationSystemTestWrapper(CommunicationSystemTestState& state_, const String& name, const Guid& id_,
                                       ECommunicationSystemTestConfiguration commSysConfig_);
        virtual ~CommunicationSystemTestWrapper();

        virtual void registerAsEventReceiver() {}

        void registerForConnectionUpdates();
        void unregisterForConnectionUpdates();

        Guid id;
        PlatformLock frameworkLock;
        MockConnectionStatusListener statusUpdateListener;
        std::unique_ptr<ICommunicationSystem> commSystem;
        StatisticCollectionFramework statisticCollection;

    protected:
        CommunicationSystemTestState& state;
        testing::NiceMock<DcsmProviderServiceHandlerMock> dcsmProviderMock;
        testing::NiceMock<DcsmConsumerServiceHandlerMock> dcsmConsumerMock;
    };

    class CommunicationSystemDiscoveryDaemonTestWrapper
    {
    public:
        explicit CommunicationSystemDiscoveryDaemonTestWrapper(CommunicationSystemTestState& state_);
        ~CommunicationSystemDiscoveryDaemonTestWrapper();

        bool start();
        bool stop();
    private:
        std::unique_ptr<IDiscoveryDaemon> daemon;
        PlatformLock frameworkLock;
        StatisticCollectionFramework statisticCollection;
    };
}

#endif
