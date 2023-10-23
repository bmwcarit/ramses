//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RamsesFrameworkImpl.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Ramsh/RamshTools.h"
#include "internal/PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "ramses-sdk-build-config.h"
#include "internal/Communication/TransportCommon/CommunicationSystemFactory.h"
#include "internal/Communication/TransportCommon/ICommunicationSystem.h"
#include "internal/Core/Utils/RamsesLogger.h"
#include "impl/RamsesFrameworkConfigImpl.h"
#include "ramses/framework/RamsesFrameworkConfig.h"
#include "internal/Ramsh/RamshStandardSetup.h"
#include "internal/PlatformAbstraction/synchronized_clock.h"
#include "impl/FrameworkFactoryRegistry.h"
#include "internal/PlatformAbstraction/PlatformTime.h"
#include "PublicRamshCommand.h"
#include "ramses/framework/IRamshCommand.h"
#include <random>

namespace ramses::internal
{
    RamsesFrameworkImpl::RamsesFrameworkImpl(const RamsesFrameworkConfigImpl& config, const ParticipantIdentifier& participantAddress)
        : m_ramsh(new RamshStandardSetup(config.m_shellType))
        , m_participantAddress(participantAddress)
        // NOTE: if you add something here consider using m_frameworkLock for all locking purposes inside this new class
        , m_connectionProtocol(config.getUsedProtocol())
        , m_communicationSystem(CommunicationSystemFactory::ConstructCommunicationSystem(config, participantAddress, m_frameworkLock, m_statisticCollection))
        , m_periodicLogger(m_frameworkLock, m_statisticCollection)
        , m_connected(false)
        , m_threadWatchdogConfig(config.m_watchdogConfig)
        // NOTE: ThreadedTaskExecutor must always be constructed after CommunicationSystem
        , m_threadedTaskExecutor(3, config.m_watchdogConfig)
        , m_resourceComponent(m_statisticCollection, m_frameworkLock)
        , m_scenegraphComponent(
            m_participantAddress.getParticipantId(),
            *m_communicationSystem,
            m_communicationSystem->getRamsesConnectionStatusUpdateNotifier(),
            m_resourceComponent,
            m_frameworkLock,
            config.getFeatureLevel())
        , m_ramshCommandLogConnectionInformation(std::make_shared<LogConnectionInfo>(*m_communicationSystem))
        , m_featureLevel{ config.getFeatureLevel() }
        , m_ramsesRenderer(nullptr, [](RamsesRenderer* /*renderer*/) {})
    {
        m_ramsh->start();
        m_ramsh->add(m_ramshCommandLogConnectionInformation);
        m_periodicLogger.registerPeriodicLogSupplier(m_communicationSystem.get());
    }

    RamsesFrameworkImpl::~RamsesFrameworkImpl()
    {
        LOG_INFO(CONTEXT_CLIENT, "RamsesFramework::~RamsesFramework: guid " << m_participantAddress.getParticipantId() << ", wasConnected " << m_connected
            << ", has Renderer " << !!m_ramsesRenderer << ", number of Clients " << m_ramsesClients.size());

        if (m_connected)
            disconnect();

        LOG_INFO(CONTEXT_CLIENT, "RamsesFramework::~RamsesFramework: deleting clients");
        m_ramsesClients.clear();
        LOG_INFO(CONTEXT_CLIENT, "RamsesFramework::~RamsesFramework: deleting renderer");
        m_ramsesRenderer.reset();

        m_periodicLogger.removePeriodicLogSupplier(m_communicationSystem.get());
    }

    RamsesRenderer* RamsesFrameworkImpl::createRenderer(const RendererConfig& config)
    {
        if (!FrameworkFactoryRegistry::GetInstance().getRendererFactory())
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "RamsesFramework::createRamsesRenderer: renderer creation failed because ramses was built without renderer support");
            return nullptr;
        }
        if (m_connected)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "RamsesFramework::createRamsesClient: framework may not be connected on client creation");
            return nullptr;
        }
        if (m_ramsesRenderer)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "RamsesFramework::createRamsesRenderer: can only create one renderer per framework");
            return nullptr;
        }

        LOG_INFO(CONTEXT_FRAMEWORK, "RamsesFramework::createRamsesRenderer");
        m_ramsesRenderer = FrameworkFactoryRegistry::GetInstance().getRendererFactory()->createRenderer(*this, config);
        return m_ramsesRenderer.get();
    }

    RamsesClient* RamsesFrameworkImpl::createClient(std::string_view applicationName)
    {
        if (!FrameworkFactoryRegistry::GetInstance().getClientFactory())
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "RamsesFramework::createRamsesClient: client creation failed because ramses was built without client support");
            return nullptr;
        }
        if (m_connected)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "RamsesFramework::createRamsesClient: framework may not be connected on client creation");
            return nullptr;
        }

        LOG_INFO(CONTEXT_FRAMEWORK, "RamsesFramework::createRamsesClient");
        ClientUniquePtr client = FrameworkFactoryRegistry::GetInstance().getClientFactory()->createClient(*this, applicationName);
        auto clientPtr = client.get();
        m_ramsesClients.emplace(std::make_pair(clientPtr, std::move(client)));
        return clientPtr;
    }

    bool RamsesFrameworkImpl::destroyRenderer(RamsesRenderer& renderer)
    {
        if (!FrameworkFactoryRegistry::GetInstance().getRendererFactory())
        {
            m_errorReporting.set("RamsesFramework::destroyRenderer: renderer destruction failed because ramses was built without renderer support");
            return false;
        }

        LOG_INFO(CONTEXT_FRAMEWORK, "RamsesFramework::destroyRenderer");

        if (!m_ramsesRenderer || m_ramsesRenderer.get() != &renderer)
        {
            m_errorReporting.set("RamsesFramework::destroyRenderer: renderer does not belong to this framework");
            return false;
        }
        if (m_connected)
        {
            m_errorReporting.set("RamsesFramework::destroyRenderer: framework may not be connected on renderer destruction");
            return false;
        }

        m_ramsesRenderer.reset();
        return true;
    }

    bool RamsesFrameworkImpl::destroyClient(RamsesClient& client)
    {
        if (!FrameworkFactoryRegistry::GetInstance().getClientFactory())
        {
            m_errorReporting.set("RamsesFramework::destroyClient: client destruction failed because ramses was built without client support");
            return false;
        }

        LOG_INFO(CONTEXT_FRAMEWORK, "RamsesFramework::destroyClient");

        auto clientIt = m_ramsesClients.find(&client);
        if (clientIt == m_ramsesClients.end())
        {
            m_errorReporting.set("RamsesFramework::destroyClient: client does not belong to this framework");
            return false;
        }
        if (m_connected)
        {
            m_errorReporting.set("RamsesFramework::destroyClient: framework may not be connected on client destruction");
            return false;
        }

        m_ramsesClients.erase(clientIt);
        return true;
    }

    EFeatureLevel RamsesFrameworkImpl::getFeatureLevel() const
    {
        return m_featureLevel;
    }

    ErrorReporting& RamsesFrameworkImpl::getErrorReporting()
    {
        return m_errorReporting;
    }

    std::optional<Issue> RamsesFrameworkImpl::getLastError()
    {
        auto issue = m_errorReporting.getError();
        m_errorReporting.reset();
        return issue;
    }

    const RamsesFramework& RamsesFrameworkImpl::getHLRamsesFramework() const
    {
        assert(m_hlFramework);
        return *m_hlFramework;
    }

    RamsesFramework& RamsesFrameworkImpl::getHLRamsesFramework()
    {
        assert(m_hlFramework);
        return *m_hlFramework;
    }

    ResourceComponent& RamsesFrameworkImpl::getResourceComponent()
    {
        return m_resourceComponent;
    }

    SceneGraphComponent& RamsesFrameworkImpl::getScenegraphComponent()
    {
        return m_scenegraphComponent;
    }

    ParticipantIdentifier RamsesFrameworkImpl::getParticipantAddress() const
    {
        return m_participantAddress;
    }

    Ramsh& RamsesFrameworkImpl::getRamsh()
    {
        return *m_ramsh;
    }

    PlatformLock& RamsesFrameworkImpl::getFrameworkLock()
    {
        return m_frameworkLock;
    }

    const ThreadWatchdogConfig& RamsesFrameworkImpl::getThreadWatchdogConfig() const
    {
        return m_threadWatchdogConfig;
    }

    ITaskQueue& RamsesFrameworkImpl::getTaskQueue()
    {
        return m_threadedTaskExecutor;
    }

    PeriodicLogger& RamsesFrameworkImpl::getPeriodicLogger()
    {
        return m_periodicLogger;
    }

    StatisticCollectionFramework& RamsesFrameworkImpl::getStatisticCollection()
    {
        return m_statisticCollection;
    }

    bool RamsesFrameworkImpl::addRamshCommand(const std::shared_ptr<IRamshCommand>& command)
    {
        if (!command)
        {
            m_errorReporting.set("addRamshCommand: command may not be null");
            return false;
        }
        LOG_INFO_P(CONTEXT_FRAMEWORK, "RamsesFrameworkImpl::addRamshCommand: keyword '{}'", command->keyword());
        auto commandWrapper = std::make_shared<PublicRamshCommand>(command);
        if (!m_ramsh->add(commandWrapper, false))
        {
            m_errorReporting.set("addRamshCommand: command not valid");
            return false;
        }
        m_publicRamshCommands.push_back(commandWrapper);
        return true;
    }

    bool RamsesFrameworkImpl::executeRamshCommand(const std::string& input)
    {
        if (input.empty())
        {
            m_errorReporting.set("executeRamshCommand: command may not be empty");
            return false;
        }
        LOG_INFO_P(CONTEXT_FRAMEWORK, "RamsesFrameworkImpl::executeRamshCommand: '{}'", input);
        if (!m_ramsh->execute(RamshTools::parseCommandString(input)))
        {
            m_errorReporting.set("executeRamshCommand: executing command failed");
            return false;
        }
        return true;
    }

    bool RamsesFrameworkImpl::connect()
    {
        LOG_INFO(CONTEXT_FRAMEWORK, "RamsesFrameworkImpl::connect");

        if (m_connected)
        {
            m_errorReporting.set("Already connected, cannot connect twice");
            return false;
        }

        if (!m_communicationSystem->connectServices())
        {
            m_errorReporting.set("Could not connect to daemon");
            return false;
        }

        m_scenegraphComponent.connectToNetwork();

        m_connected = true;
        return true;
    }

    bool RamsesFrameworkImpl::isConnected() const
    {
        return m_connected;
    }

    bool RamsesFrameworkImpl::disconnect()
    {
        LOG_INFO(CONTEXT_FRAMEWORK, "RamsesFrameworkImpl::disconnect");

        if (!m_connected)
        {
            m_errorReporting.set("Not connected, cannot disconnect");
            return false;
        }

        m_scenegraphComponent.disconnectFromNetwork();
        m_communicationSystem->disconnectServices();

        m_connected = false;
        LOG_INFO(CONTEXT_FRAMEWORK, "RamsesFrameworkImpl::disconnect: done ok");

        return true;
    }

    void RamsesFrameworkImpl::setHLFramework(RamsesFramework& framework)
    {
        m_hlFramework = &framework;
    }

    std::unique_ptr<RamsesFrameworkImpl> RamsesFrameworkImpl::CreateImpl(const RamsesFrameworkConfig& config)
    {
        GetRamsesLogger().initialize(config.impl().loggerConfig, false, config.impl().getDltApplicationRegistrationEnabled());

        Guid myGuid = config.impl().getUserProvidedGuid();
        if (!myGuid.isValid())
        {
            // check if user provided one
            myGuid = config.impl().getUserProvidedGuid();

            // generate randomly when invalid or overlappping with reserved values (make sure generated ids do not collide with explicit guids)
            if (myGuid.isInvalid() || myGuid.get() <= 0xFF)
            {
                // minimum value is 256, ensure never collides with explicit guid
                std::mt19937 gen(std::random_device{}());
                std::uniform_int_distribution<uint64_t> dis(256);
                myGuid = Guid(dis(gen));
            }
        }
        ParticipantIdentifier participantAddress(myGuid, config.impl().getParticipantName());

        LOG_INFO(CONTEXT_FRAMEWORK, "Starting Ramses Client Application: " << participantAddress.getParticipantName() << " guid:" << participantAddress.getParticipantId() <<
                 " stack: " << config.impl().getUsedProtocol());

        LogEnvironmentVariableIfSet("XDG_RUNTIME_DIR");
        LogEnvironmentVariableIfSet("LIBGL_DRIVERS_PATH");
        LogAvailableCommunicationStacks();
        LogBuildInformation();

        const auto currentSyncTime = synchronized_clock::now();
        const uint64_t systemClockTime = PlatformTime::GetMicrosecondsAbsolute();
        LOG_INFO(CONTEXT_FRAMEWORK, "Ramses synchronized time is using " << synchronized_clock::source() <<
                     ". Currrent sync time " << asMicroseconds(currentSyncTime) << " us, system clock is " << systemClockTime << " us");

        std::unique_ptr<RamsesFrameworkImpl> impl{ new RamsesFrameworkImpl(config.impl(), participantAddress) };
        if (config.impl().m_periodicLogsEnabled)
        {
            LOG_INFO_P(CONTEXT_FRAMEWORK, "RamsesFramework: periodic logs enabled with period of {}s", config.impl().periodicLogTimeout);
            impl->m_periodicLogger.startLogging(config.impl().periodicLogTimeout);
        }
        else
        {
            LOG_INFO_P(CONTEXT_FRAMEWORK, "RamsesFramework: periodic logs disabled");
        }

        return impl;
    }

    void RamsesFrameworkImpl::SetLogHandler(const LogHandlerFunc& logHandlerFunc)
    {
        GetRamsesLogger().setLogHandler(logHandlerFunc);
    }

    void RamsesFrameworkImpl::LogEnvironmentVariableIfSet(std::string_view envVarName)
    {
        std::string envVarValue;
        // TODO(tobias) envVarValue.getLength should not be there because empty variable is also set. remove when fixed
        if (PlatformEnvironmentVariables::get(std::string{envVarName}, envVarValue) && !envVarValue.empty())
        {
            LOG_INFO(CONTEXT_FRAMEWORK, "Environment variable set: " << envVarName << "=" << envVarValue);
        }
    }

    void RamsesFrameworkImpl::LogAvailableCommunicationStacks()
    {
        // Create log function outside to work around broken MSVC macro in macro behavior
        auto fun = [](StringOutputStream& sos) {
                       sos << "Available communication stacks:";
#if defined(HAS_TCP_COMM)
                       sos << " TCP";
#endif
                   };
        LOG_INFO_F(CONTEXT_FRAMEWORK, fun);
    }

    void RamsesFrameworkImpl::LogBuildInformation()
    {
        // Create log function outside to work around broken MSVC macro in macro behavior
        auto fun = [](StringOutputStream& sos) {
                       sos << "RamsesBuildInfo: Version " << ramses_sdk::RAMSES_SDK_RAMSES_VERSION
                           << ", Compiler " << ramses_sdk::RAMSES_SDK_CMAKE_CXX_COMPILER_ID
                           << ", Config " << ramses_sdk::RAMSES_SDK_CMAKE_BUILD_TYPE
                           << ", Asserts "
#ifdef NDEBUG
                           << "off";
#else
                           << "on";
#endif
                   };
        LOG_INFO_F(CONTEXT_FRAMEWORK, fun);

    }
}
