//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESFRAMEWORKIMPL_H
#define RAMSES_RAMSESFRAMEWORKIMPL_H

#include "Utils/CommandLineParser.h"
#include "TaskFramework/ThreadingSystem.h"
#include "Utils/ScopedPointer.h"
#include "Ramsh/RamshStandardSetup.h"
#include "Components/ResourceComponent.h"
#include "Components/SceneGraphComponent.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "StatusObjectImpl.h"
#include "Common/ParticipantIdentifier.h"
#include "Utils/PeriodicLogger.h"
#include "TransportCommon/LogConnectionInfo.h"
#include "Utils/StatisticCollection.h"
#include "Ramsh/RamshDLT.h"
#include "Components/DcsmComponent.h"
#include "Components/LogDcsmInfo.h"

namespace ramses_internal
{
    class ICommunicationSystem;
}

namespace ramses
{
    class RamsesFrameworkConfig;
    class RamsesFrameworkConfigImpl;
    class DcsmProvider;
    class DcsmConsumer;

    class RamsesFrameworkImpl : public StatusObjectImpl
    {
    public:
        ~RamsesFrameworkImpl();

        static RamsesFrameworkImpl& createImpl(const RamsesFrameworkConfig& config);
        static RamsesFrameworkImpl& createImpl(int32_t argc, const char * argv[]);

        status_t connect();
        bool isConnected() const;
        status_t disconnect();
        DcsmConsumer* createDcsmConsumer();
        status_t destroyDcsmConsumer(const DcsmConsumer&);
        DcsmProvider* createDcsmProvider();
        status_t destroyDcsmProvider(const DcsmProvider&);

        ramses_internal::ResourceComponent& getResourceComponent();
        ramses_internal::SceneGraphComponent& getScenegraphComponent();
        ramses_internal::DcsmComponent& getDcsmComponent();
        ramses_internal::IConnectionStatusUpdateNotifier& getRamsesConnectionStatusUpdateNotifier();
        ramses_internal::ParticipantIdentifier getParticipantAddress() const;
        ramses_internal::Ramsh& getRamsh();
        ramses_internal::PlatformLock& getFrameworkLock();
        const ramses_internal::ThreadWatchdogConfig& getThreadWatchdogConfig() const;
        ramses_internal::ITaskQueue& getTaskQueue();
        ramses_internal::PeriodicLogger& getPeriodicLogger();
        ramses_internal::StatisticCollectionFramework& getStatisticCollection();

    private:
        RamsesFrameworkImpl(const RamsesFrameworkConfigImpl& config, const ramses_internal::ParticipantIdentifier& participantAddress);
        static ramses_internal::String GetParticipantName(const RamsesFrameworkConfig& config);
        static void logEnvironmentVariableIfSet(const ramses_internal::String& envVarName);

        static constexpr uint32_t PeriodicLogIntervalInSeconds = 2;
        // the framework-wide mutex that is used by all framework-base classes to synchronize access to shared resource
        // has to be used by all logic, component, etc classes
        ramses_internal::PlatformLock m_frameworkLock;
        ramses_internal::ScopedPointer<ramses_internal::Ramsh> m_ramsh;
        ramses_internal::StatisticCollectionFramework m_statisticCollection;
        ramses_internal::ParticipantIdentifier m_participantAddress;
        ramses_internal::EConnectionProtocol m_connectionProtocol;
        ramses_internal::ScopedPointer<ramses_internal::ICommunicationSystem> m_communicationSystem;
        ramses_internal::PeriodicLogger m_periodicLogger;
        bool m_connected;
        const ramses_internal::ThreadWatchdogConfig m_threadWatchdogConfig;
        ramses_internal::ThreadingSystem m_threadStrategy;
        ramses_internal::ResourceComponent m_resourceComponent;
        ramses_internal::SceneGraphComponent m_scenegraphComponent;
        ramses_internal::DcsmComponent m_dcsmComponent;
        ramses_internal::LogConnectionInfo m_ramshCommandLogConnectionInformation;
        ramses_internal::LogDcsmInfo m_ramshCommandLogDcsmInformation;
        std::unique_ptr<DcsmProvider> m_dcsmProvider;
        std::unique_ptr<DcsmConsumer> m_dcsmConsumer;
    };
}

#endif
