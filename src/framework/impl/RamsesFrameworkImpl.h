//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/TaskFramework/ThreadedTaskExecutor.h"
#include "internal/Components/ResourceComponent.h"
#include "internal/Components/SceneGraphComponent.h"
#include "internal/Core/Common/ParticipantIdentifier.h"
#include "internal/Core/Utils/PeriodicLogger.h"
#include "internal/Core/Utils/StatisticCollection.h"
#include "internal/Communication/TransportCommon/LogConnectionInfo.h"
#include "internal/Communication/TransportCommon/EConnectionProtocol.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/EFeatureLevel.h"
#include "ramses/framework/Issue.h"
#include "impl/RamsesObjectFactoryInterfaces.h"
#include "impl/ErrorReporting.h"

#include <unordered_map>
#include <memory>
#include <string_view>
#include <optional>

namespace ramses
{
    class RamsesFramework;
    class RamsesFrameworkConfig;
    class RamsesClient;
    class RamsesRenderer;
    class RendererConfig;
    class IRamshCommand;
}

namespace ramses::internal
{
    class ICommunicationSystem;
    class RamshStandardSetup;
    class Ramsh;
    class PublicRamshCommand;
    class RamsesFrameworkConfigImpl;

    class RamsesFrameworkImpl
    {
    public:
        ~RamsesFrameworkImpl();

        RamsesRenderer* createRenderer(const RendererConfig& config);
        RamsesClient* createClient(std::string_view applicationName);

        bool destroyRenderer(RamsesRenderer& renderer);
        bool destroyClient(RamsesClient& client);

        bool connect();
        bool isConnected() const;
        bool disconnect();

        const RamsesFramework& getHLRamsesFramework() const;
        RamsesFramework& getHLRamsesFramework();
        EFeatureLevel getFeatureLevel() const;
        ErrorReporting& getErrorReporting();
        [[nodiscard]] std::optional<Issue> getLastError();

        ResourceComponent& getResourceComponent();
        SceneGraphComponent& getScenegraphComponent();
        ParticipantIdentifier getParticipantAddress() const;
        Ramsh& getRamsh();
        PlatformLock& getFrameworkLock();
        const ThreadWatchdogConfig& getThreadWatchdogConfig() const;
        ITaskQueue& getTaskQueue();
        PeriodicLogger& getPeriodicLogger();
        StatisticCollectionFramework& getStatisticCollection();
        static void SetLogHandler(const LogHandlerFunc& logHandlerFunc);
        bool addRamshCommand(const std::shared_ptr<IRamshCommand>& command);
        bool executeRamshCommand(const std::string& input);

        void setHLFramework(RamsesFramework& framework);
        static std::unique_ptr<RamsesFrameworkImpl> CreateImpl(const RamsesFrameworkConfig& config);

    private:
        RamsesFrameworkImpl(const RamsesFrameworkConfigImpl& config, const ParticipantIdentifier& participantAddress);
        static void LogEnvironmentVariableIfSet(std::string_view envVarName);
        static void LogAvailableCommunicationStacks();
        static void LogBuildInformation();

        // the framework-wide mutex that is used by all framework-base classes to synchronize access to shared resource
        // has to be used by all logic, component, etc classes
        PlatformLock m_frameworkLock;
        std::unique_ptr<RamshStandardSetup> m_ramsh;
        std::vector<std::shared_ptr<PublicRamshCommand>> m_publicRamshCommands;
        StatisticCollectionFramework m_statisticCollection;
        ParticipantIdentifier m_participantAddress;
        EConnectionProtocol m_connectionProtocol;
        std::unique_ptr<ICommunicationSystem> m_communicationSystem;
        PeriodicLogger m_periodicLogger;
        bool m_connected;
        const ThreadWatchdogConfig m_threadWatchdogConfig;
        ThreadedTaskExecutor m_threadedTaskExecutor;
        ResourceComponent m_resourceComponent;
        SceneGraphComponent m_scenegraphComponent;
        std::shared_ptr<LogConnectionInfo> m_ramshCommandLogConnectionInformation;

        EFeatureLevel m_featureLevel;
        ErrorReporting m_errorReporting;

        RamsesFramework* m_hlFramework = nullptr;
        std::unordered_map<RamsesClient*, ClientUniquePtr> m_ramsesClients;
        RendererUniquePtr m_ramsesRenderer;
    };
}
