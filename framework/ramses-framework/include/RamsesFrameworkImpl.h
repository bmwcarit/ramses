//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESFRAMEWORKIMPL_H
#define RAMSES_RAMSESFRAMEWORKIMPL_H

#include "StatusObjectImpl.h"
#include "TaskFramework/ThreadedTaskExecutor.h"
#include "Components/ResourceComponent.h"
#include "Components/SceneGraphComponent.h"
#include "Common/ParticipantIdentifier.h"
#include "Utils/PeriodicLogger.h"
#include "Utils/StatisticCollection.h"
#include "TransportCommon/LogConnectionInfo.h"
#include "TransportCommon/EConnectionProtocol.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/EFeatureLevel.h"
#include "RamsesObjectFactoryInterfaces.h"
#include <unordered_map>
#include <memory>

namespace ramses_internal
{
    class ICommunicationSystem;
    class RamshStandardSetup;
    class Ramsh;
    class PublicRamshCommand;
}

namespace ramses
{
    class RamsesFrameworkConfig;
    class RamsesFrameworkConfigImpl;
    class RamsesClient;
    class RamsesRenderer;
    class RendererConfig;
    class IRamshCommand;

    class RamsesFrameworkImpl : public StatusObjectImpl
    {
    public:
        ~RamsesFrameworkImpl() override;

        static RamsesFrameworkImpl& createImpl(const RamsesFrameworkConfig& config);

        RamsesRenderer* createRenderer(const RendererConfig& config);
        RamsesClient* createClient(const char* applicationName);

        status_t destroyRenderer(RamsesRenderer& renderer);
        status_t destroyClient(RamsesClient& client);

        status_t connect();
        bool isConnected() const;
        status_t disconnect();

        EFeatureLevel getFeatureLevel() const;
        ramses_internal::ResourceComponent& getResourceComponent();
        ramses_internal::SceneGraphComponent& getScenegraphComponent();
        ramses_internal::ParticipantIdentifier getParticipantAddress() const;
        ramses_internal::Ramsh& getRamsh();
        ramses_internal::PlatformLock& getFrameworkLock();
        const ramses_internal::ThreadWatchdogConfig& getThreadWatchdogConfig() const;
        ramses_internal::ITaskQueue& getTaskQueue();
        ramses_internal::PeriodicLogger& getPeriodicLogger();
        ramses_internal::StatisticCollectionFramework& getStatisticCollection();
        static void SetConsoleLogLevel(ELogLevel logLevel);
        static void SetLogHandler(const LogHandlerFunc& logHandlerFunc);
        status_t addRamshCommand(const std::shared_ptr<IRamshCommand>& command);
        status_t executeRamshCommand(const std::string& input);

    private:
        RamsesFrameworkImpl(const RamsesFrameworkConfigImpl& config, const ramses_internal::ParticipantIdentifier& participantAddress);
        static void LogEnvironmentVariableIfSet(const ramses_internal::String& envVarName);
        static void LogAvailableCommunicationStacks();
        static void LogBuildInformation();

        // the framework-wide mutex that is used by all framework-base classes to synchronize access to shared resource
        // has to be used by all logic, component, etc classes
        ramses_internal::PlatformLock m_frameworkLock;
        std::unique_ptr<ramses_internal::RamshStandardSetup> m_ramsh;
        std::vector<std::shared_ptr<ramses_internal::PublicRamshCommand>> m_publicRamshCommands;
        ramses_internal::StatisticCollectionFramework m_statisticCollection;
        ramses_internal::ParticipantIdentifier m_participantAddress;
        ramses_internal::EConnectionProtocol m_connectionProtocol;
        std::unique_ptr<ramses_internal::ICommunicationSystem> m_communicationSystem;
        ramses_internal::PeriodicLogger m_periodicLogger;
        bool m_connected;
        const ramses_internal::ThreadWatchdogConfig m_threadWatchdogConfig;
        ramses_internal::ThreadedTaskExecutor m_threadedTaskExecutor;
        ramses_internal::ResourceComponent m_resourceComponent;
        ramses_internal::SceneGraphComponent m_scenegraphComponent;
        std::shared_ptr<ramses_internal::LogConnectionInfo> m_ramshCommandLogConnectionInformation;

        EFeatureLevel m_featureLevel;
        std::unordered_map<RamsesClient*, ClientUniquePtr> m_ramsesClients;
        RendererUniquePtr m_ramsesRenderer;
    };
}

#endif
