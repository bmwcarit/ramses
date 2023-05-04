//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesFrameworkImpl.h"
#include "Utils/LogMacros.h"
#include "Ramsh/RamshTools.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "ramses-sdk-build-config.h"
#include "TransportCommon/CommunicationSystemFactory.h"
#include "TransportCommon/ICommunicationSystem.h"
#include "Utils/RamsesLogger.h"
#include "RamsesFrameworkConfigImpl.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "Ramsh/RamshStandardSetup.h"
#include "PlatformAbstraction/synchronized_clock.h"
#include "FrameworkFactoryRegistry.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "PublicRamshCommand.h"
#include "ramses-framework-api/IRamshCommand.h"
#include <random>

namespace ramses
{
    using namespace ramses_internal;

    RamsesFrameworkImpl::RamsesFrameworkImpl(const RamsesFrameworkConfigImpl& config, const ramses_internal::ParticipantIdentifier& participantAddress)
        : StatusObjectImpl()
        , m_ramsh(new RamshStandardSetup(config.m_shellType))
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
        , m_ramshCommandLogConnectionInformation(std::make_shared<ramses_internal::LogConnectionInfo>(*m_communicationSystem))
        , m_featureLevel{ config.getFeatureLevel() }
        , m_ramsesClients()
        , m_ramsesRenderer(nullptr, [](RamsesRenderer*) {})
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

        LOG_INFO(ramses_internal::CONTEXT_FRAMEWORK, "RamsesFramework::createRamsesRenderer");
        m_ramsesRenderer = FrameworkFactoryRegistry::GetInstance().getRendererFactory()->createRenderer(*this, config);
        return m_ramsesRenderer.get();
    }

    RamsesClient* RamsesFrameworkImpl::createClient(const char* applicationName)
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

        LOG_INFO(ramses_internal::CONTEXT_FRAMEWORK, "RamsesFramework::createRamsesClient");
        ClientUniquePtr client = FrameworkFactoryRegistry::GetInstance().getClientFactory()->createClient(*this, applicationName);
        auto clientPtr = client.get();
        m_ramsesClients.emplace(std::make_pair(clientPtr, std::move(client)));
        return clientPtr;
    }

    status_t RamsesFrameworkImpl::destroyRenderer(RamsesRenderer& renderer)
    {
        if (!FrameworkFactoryRegistry::GetInstance().getRendererFactory())
            return addErrorEntry("RamsesFramework::destroyRenderer: renderer destruction failed because ramses was built without renderer support");

        LOG_INFO(ramses_internal::CONTEXT_FRAMEWORK, "RamsesFramework::destroyRenderer");

        if (!m_ramsesRenderer || m_ramsesRenderer.get() != &renderer)
        {
            return addErrorEntry("RamsesFramework::destroyRenderer: renderer does not belong to this framework");
        }
        if (m_connected)
        {
            return addErrorEntry("RamsesFramework::destroyRenderer: framework may not be connected on renderer destruction");
        }

        m_ramsesRenderer.reset();
        return StatusOK;
    }

    status_t RamsesFrameworkImpl::destroyClient(RamsesClient& client)
    {
        if (!FrameworkFactoryRegistry::GetInstance().getClientFactory())
            return addErrorEntry("RamsesFramework::destroyClient: client destruction failed because ramses was built without client support");

        LOG_INFO(ramses_internal::CONTEXT_FRAMEWORK, "RamsesFramework::destroyClient");

        auto clientIt = m_ramsesClients.find(&client);
        if (clientIt == m_ramsesClients.end())
        {
            return addErrorEntry("RamsesFramework::destroyClient: client does not belong to this framework");
        }
        if (m_connected)
        {
            return addErrorEntry("RamsesFramework::destroyClient: framework may not be connected on client destruction");
        }

        m_ramsesClients.erase(clientIt);
        return StatusOK;
    }

    EFeatureLevel RamsesFrameworkImpl::getFeatureLevel() const
    {
        return m_featureLevel;
    }

    ramses_internal::ResourceComponent& RamsesFrameworkImpl::getResourceComponent()
    {
        return m_resourceComponent;
    }

    ramses_internal::SceneGraphComponent& RamsesFrameworkImpl::getScenegraphComponent()
    {
        return m_scenegraphComponent;
    }

    ramses_internal::ParticipantIdentifier RamsesFrameworkImpl::getParticipantAddress() const
    {
        return m_participantAddress;
    }

    ramses_internal::Ramsh& RamsesFrameworkImpl::getRamsh()
    {
        return *m_ramsh;
    }

    ramses_internal::PlatformLock& RamsesFrameworkImpl::getFrameworkLock()
    {
        return m_frameworkLock;
    }

    const ramses_internal::ThreadWatchdogConfig& RamsesFrameworkImpl::getThreadWatchdogConfig() const
    {
        return m_threadWatchdogConfig;
    }

    ramses_internal::ITaskQueue& RamsesFrameworkImpl::getTaskQueue()
    {
        return m_threadedTaskExecutor;
    }

    ramses_internal::PeriodicLogger& RamsesFrameworkImpl::getPeriodicLogger()
    {
        return m_periodicLogger;
    }

    ramses_internal::StatisticCollectionFramework& RamsesFrameworkImpl::getStatisticCollection()
    {
        return m_statisticCollection;
    }

    status_t RamsesFrameworkImpl::addRamshCommand(const std::shared_ptr<IRamshCommand>& command)
    {
        if (!command)
            return addErrorEntry("addRamshCommand: command may not be null");
        LOG_INFO_P(CONTEXT_FRAMEWORK, "RamsesFrameworkImpl::addRamshCommand: keyword '{}'", command->keyword());
        auto commandWrapper = std::make_shared<ramses_internal::PublicRamshCommand>(command);
        if (!m_ramsh->add(commandWrapper, false))
            return addErrorEntry("addRamshCommand: command not valid");
        m_publicRamshCommands.push_back(commandWrapper);
        return StatusOK;
    }

    status_t RamsesFrameworkImpl::executeRamshCommand(const std::string& input)
    {
        if (input.empty())
            return addErrorEntry("executeRamshCommand: command may not be empty");
        LOG_INFO_P(CONTEXT_FRAMEWORK, "RamsesFrameworkImpl::executeRamshCommand: '{}'", input);
        if (!m_ramsh->execute(RamshTools::parseCommandString(input.c_str())))
            return addErrorEntry("executeRamshCommand: executing command failed");
        return StatusOK;
    }

    ramses::status_t RamsesFrameworkImpl::connect()
    {
        LOG_INFO(CONTEXT_FRAMEWORK, "RamsesFrameworkImpl::connect");

        if (m_connected)
        {
            return addErrorEntry("Already connected, cannot connect twice");
        }

        if (!m_communicationSystem->connectServices())
        {
            return addErrorEntry("Could not connect to daemon");
        }

        m_scenegraphComponent.connectToNetwork();

        m_connected = true;
        return StatusOK;
    }

    bool RamsesFrameworkImpl::isConnected() const
    {
        return m_connected;
    }

    ramses::status_t RamsesFrameworkImpl::disconnect()
    {
        LOG_INFO(CONTEXT_FRAMEWORK, "RamsesFrameworkImpl::disconnect");

        if (!m_connected)
        {
            return addErrorEntry("Not connected, cannot disconnect");
        }

        m_scenegraphComponent.disconnectFromNetwork();
        m_communicationSystem->disconnectServices();

        m_connected = false;
        LOG_INFO(CONTEXT_FRAMEWORK, "RamsesFrameworkImpl::disconnect: done ok");

        return StatusOK;
    }

    std::unique_ptr<RamsesFrameworkImpl> RamsesFrameworkImpl::CreateImpl(const RamsesFrameworkConfig& config)
    {
        ramses_internal::GetRamsesLogger().initialize(config.m_impl.get().loggerConfig, false, config.m_impl.get().getDltApplicationRegistrationEnabled());

        ramses_internal::Guid myGuid = config.m_impl.get().getUserProvidedGuid();
        if (!myGuid.isValid())
        {
            // check if user provided one
            myGuid = config.m_impl.get().getUserProvidedGuid();

            // generate randomly when invalid or overlappping with reserved values (make sure generated ids do not collide with explicit guids)
            if (myGuid.isInvalid() || myGuid.get() <= 0xFF)
            {
                // minimum value is 256, ensure never collides with explicit guid
                std::mt19937 gen(std::random_device{}());
                std::uniform_int_distribution<uint64_t> dis(256);
                myGuid = Guid(dis(gen));
            }
        }
        ramses_internal::ParticipantIdentifier participantAddress(myGuid, config.m_impl.get().getParticipantName());

        LOG_INFO(CONTEXT_FRAMEWORK, "Starting Ramses Client Application: " << participantAddress.getParticipantName() << " guid:" << participantAddress.getParticipantId() <<
                 " stack: " << config.m_impl.get().getUsedProtocol());

        LogEnvironmentVariableIfSet("XDG_RUNTIME_DIR");
        LogEnvironmentVariableIfSet("LIBGL_DRIVERS_PATH");
        LogAvailableCommunicationStacks();
        LogBuildInformation();

        const auto currentSyncTime = synchronized_clock::now();
        const UInt64 systemClockTime = PlatformTime::GetMicrosecondsAbsolute();
        LOG_INFO(CONTEXT_FRAMEWORK, "Ramses synchronized time is using " << synchronized_clock::source() <<
                     ". Currrent sync time " << asMicroseconds(currentSyncTime) << " us, system clock is " << systemClockTime << " us");

        std::unique_ptr<RamsesFrameworkImpl> impl{ new RamsesFrameworkImpl(config.m_impl, participantAddress) };
        if (config.m_impl.get().m_periodicLogsEnabled)
        {
            LOG_INFO_P(CONTEXT_FRAMEWORK, "RamsesFramework: periodic logs enabled with period of {}s", config.m_impl.get().periodicLogTimeout);
            impl->m_periodicLogger.startLogging(config.m_impl.get().periodicLogTimeout);
        }
        else
        {
            LOG_INFO_P(CONTEXT_FRAMEWORK, "RamsesFramework: periodic logs disabled");
        }

        return impl;
    }

    void RamsesFrameworkImpl::SetConsoleLogLevel(ELogLevel logLevel)
    {
        GetRamsesLogger().setConsoleLogLevelProgrammatically(GetELogLevelInternal(logLevel));
    }

    void RamsesFrameworkImpl::SetLogHandler(const LogHandlerFunc& logHandlerFunc)
    {
        GetRamsesLogger().setLogHandler(logHandlerFunc);
    }

    void RamsesFrameworkImpl::LogEnvironmentVariableIfSet(const ramses_internal::String& envVarName)
    {
        ramses_internal::String envVarValue;
        // TODO(tobias) envVarValue.getLength should not be there because empty variable is also set. remove when fixed
        if (ramses_internal::PlatformEnvironmentVariables::get(envVarName, envVarValue) && envVarValue.size() != 0)
        {
            LOG_INFO(CONTEXT_FRAMEWORK, "Environment variable set: " << envVarName << "=" << envVarValue);
        }
    }

    void RamsesFrameworkImpl::LogAvailableCommunicationStacks()
    {
        // Create log function outside to work around broken MSVC macro in macro behavior
        auto fun = [](ramses_internal::StringOutputStream& sos) {
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
        auto fun = [](ramses_internal::StringOutputStream& sos) {
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
