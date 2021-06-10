//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RamsesFrameworkImpl.h"
#include "Utils/Argument.h"
#include "Utils/LogMacros.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "ramses-sdk-build-config.h"
#include "TransportCommon/CommunicationSystemFactory.h"
#include "TransportCommon/ICommunicationSystem.h"
#include "TransportCommon/SomeIPAdapter.h"
#include "Utils/RamsesLogger.h"
#include "RamsesFrameworkConfigImpl.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "Ramsh/RamshStandardSetup.h"
#include "PlatformAbstraction/synchronized_clock.h"
#include "DcsmProviderImpl.h"
#include "ramses-framework-api/DcsmProvider.h"
#include "DcsmConsumerImpl.h"
#include "ramses-framework-api/DcsmConsumer.h"
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
        , m_scenegraphComponent(m_participantAddress.getParticipantId(), *m_communicationSystem, m_communicationSystem->getRamsesConnectionStatusUpdateNotifier(), m_resourceComponent, m_frameworkLock)
        , m_dcsmComponent(m_participantAddress.getParticipantId(), *m_communicationSystem, m_communicationSystem->getDcsmConnectionStatusUpdateNotifier(), m_frameworkLock)
        , m_ramshCommandLogConnectionInformation(std::make_shared<ramses_internal::LogConnectionInfo>(*m_communicationSystem))
        , m_ramshCommandLogDcsmInformation(std::make_shared<ramses_internal::LogDcsmInfo>(m_dcsmComponent))
        , m_ramsesClients()
        , m_ramsesRenderer(nullptr, [](RamsesRenderer*) {})
    {
        m_ramsh->start();
        m_ramsh->add(m_ramshCommandLogConnectionInformation);
        m_ramsh->add(m_ramshCommandLogDcsmInformation);
        m_periodicLogger.registerPeriodicLogSupplier(m_communicationSystem.get());
        m_periodicLogger.registerPeriodicLogSupplier(&m_dcsmComponent);
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
        m_periodicLogger.removePeriodicLogSupplier(&m_dcsmComponent);
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
        m_ramsesRenderer = FrameworkFactoryRegistry::GetInstance().getRendererFactory()->createRenderer(this, config);
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
        ClientUniquePtr client = FrameworkFactoryRegistry::GetInstance().getClientFactory()->createClient(this, applicationName);
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

    ramses_internal::ResourceComponent& RamsesFrameworkImpl::getResourceComponent()
    {
        return m_resourceComponent;
    }

    ramses_internal::SceneGraphComponent& RamsesFrameworkImpl::getScenegraphComponent()
    {
        return m_scenegraphComponent;
    }

    ramses_internal::DcsmComponent& RamsesFrameworkImpl::getDcsmComponent()
    {
        return m_dcsmComponent;
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

        m_dcsmComponent.connect();

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

        m_dcsmComponent.disconnect();

        m_scenegraphComponent.disconnectFromNetwork();
        m_communicationSystem->disconnectServices();

        m_connected = false;
        LOG_INFO(CONTEXT_SMOKETEST, "RamsesFrameworkImpl::disconnect end of disconnect");
        LOG_INFO(CONTEXT_FRAMEWORK, "RamsesFrameworkImpl::disconnect: done ok");

        return StatusOK;
    }

    DcsmProvider* RamsesFrameworkImpl::createDcsmProvider()
    {
        LOG_INFO(ramses_internal::CONTEXT_FRAMEWORK, "RamsesFramework::createDcsmProvider for " << m_participantAddress.getParticipantId() << " / " << m_participantAddress.getParticipantName());

        // TODO(tobias) check if creation allowed based on m_participantAddress

        if (m_dcsmProvider)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "RamsesFramework::createDcsmProvider: can only create one provider per framework");
            return nullptr;
        }
        DcsmProviderImpl* impl = new DcsmProviderImpl(getDcsmComponent());
        m_dcsmProvider = std::make_unique<DcsmProvider>(*impl);
        return m_dcsmProvider.get();
    }

    status_t RamsesFrameworkImpl::destroyDcsmProvider(const DcsmProvider& provider)
    {
        LOG_INFO(ramses_internal::CONTEXT_FRAMEWORK, "RamsesFramework::destroyDcsmProvider for " << m_participantAddress.getParticipantId() << " / " << m_participantAddress.getParticipantName());

        if (!m_dcsmProvider || m_dcsmProvider.get() != &provider)
        {
            return addErrorEntry("RamsesFramework::destroyDcsmProvider: provider does not belong to this framework");
        }
        m_dcsmProvider.reset();
        return StatusOK;
    }

    DcsmConsumer* RamsesFrameworkImpl::createDcsmConsumer()
    {
        LOG_INFO(ramses_internal::CONTEXT_FRAMEWORK, "RamsesFramework::createDcsmConsumer for " << m_participantAddress.getParticipantId() << " / " << m_participantAddress.getParticipantName());

        // TODO(tobias) check if creation allowed based on m_participantAddress

        if (m_dcsmConsumer)
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "RamsesFramework::createDcsmConsumer: can only create one consumer per framework");
            return nullptr;
        }
        DcsmConsumerImpl* impl = new DcsmConsumerImpl(*this);
        m_dcsmConsumer = std::make_unique<DcsmConsumer>(*impl);
        return m_dcsmConsumer.get();
    }

    status_t RamsesFrameworkImpl::destroyDcsmConsumer(const DcsmConsumer& consumer)
    {
        LOG_INFO(ramses_internal::CONTEXT_FRAMEWORK, "RamsesFramework::destroyDcsmConsumer for " << m_participantAddress.getParticipantId() << " / " << m_participantAddress.getParticipantName());

        if (!m_dcsmConsumer || m_dcsmConsumer.get() != &consumer)
        {
            return addErrorEntry("RamsesFramework::destroyDcsmConsumer: consumer does not belong to this framework");
        }
        m_dcsmConsumer.reset();
        return StatusOK;
    }

    RamsesFrameworkImpl& RamsesFrameworkImpl::createImpl(int32_t argc, char const* const* argv)
    {
        RamsesFrameworkConfig config(argc, argv);
        return createImpl(config);
    }

    RamsesFrameworkImpl& RamsesFrameworkImpl::createImpl(const RamsesFrameworkConfig& config)
    {
        const ramses_internal::CommandLineParser& parser = config.impl.getCommandLineParser();
        ramses_internal::GetRamsesLogger().initialize(parser, config.getDLTApplicationID(), config.getDLTApplicationDescription(), false, config.impl.getDltApplicationRegistrationEnabled());

        const ramses_internal::String& participantName = GetParticipantName(config);

        ramses_internal::Guid myGuid;
        myGuid = SomeIPAdapter::GetGuidForCommunicationUser(config.impl.getSomeipCommunicationUserID());
        if (!myGuid.isValid())
        {
            // check if user provided one
            myGuid = config.impl.getUserProvidedGuid();

            // generate from someip communication user when set
            if (!myGuid.isValid() && config.impl.getSomeipCommunicationUserID() != SomeIPAdapter::GetInvalidCommunicationUser())
            {
                myGuid = Guid(0x10000 + config.impl.getSomeipCommunicationUserID());
            }

            // generate randomly when invalid or overlappping with reserved values (make sure generated ids do not collide with explicit guids)
            if (myGuid.isInvalid() || myGuid.get() <= 0xFF)
            {
                // minimum value is 256, ensure never collides with explicit guid
                std::mt19937 gen(std::random_device{}());
                std::uniform_int_distribution<uint64_t> dis(256);
                myGuid = Guid(dis(gen));
            }
        }
        ramses_internal::ParticipantIdentifier participantAddress(myGuid, participantName);

        LOG_INFO(CONTEXT_FRAMEWORK, "Starting Ramses Client Application: " << participantAddress.getParticipantName() << " guid:" << participantAddress.getParticipantId() <<
                 " stack: " << config.impl.getUsedProtocol());

        const ramses_internal::ArgumentString executionIdentifier(parser, "executionIdentifier", "executionIdentifier", String());
        if (executionIdentifier.hasValue())
            LOG_INFO(CONTEXT_FRAMEWORK, "Program execution identifier: '" << static_cast<String>(executionIdentifier) << "'");

        const ramses_internal::ArgumentUInt32 periodicLogTimeout(parser, "plt", "periodicLogTimeout", uint32_t(PeriodicLogIntervalInSeconds));

        LogEnvironmentVariableIfSet("XDG_RUNTIME_DIR");
        LogEnvironmentVariableIfSet("LIBGL_DRIVERS_PATH");
        LogAvailableCommunicationStacks();
        LogBuildInformation();

        const auto currentSyncTime = synchronized_clock::now();
        const UInt64 systemClockTime = PlatformTime::GetMicrosecondsAbsolute();
        LOG_INFO(CONTEXT_FRAMEWORK, "Ramses synchronized time is using " << synchronized_clock::source() <<
                     ". Currrent sync time " << asMicroseconds(currentSyncTime) << " us, system clock is " << systemClockTime << " us");

        RamsesFrameworkImpl* impl = new RamsesFrameworkImpl(config.impl, participantAddress);
        if (config.impl.m_periodicLogsEnabled)
        {
            LOG_INFO_P(CONTEXT_FRAMEWORK, "RamsesFramework: periodic logs enabled with period of {}s", periodicLogTimeout);
            impl->m_periodicLogger.startLogging(periodicLogTimeout);
        }
        else
        {
            LOG_INFO_P(CONTEXT_FRAMEWORK, "RamsesFramework: periodic logs disabled");
        }
        return *impl;
    }

    ramses_internal::String RamsesFrameworkImpl::GetParticipantName(const RamsesFrameworkConfig& config)
    {
        const CommandLineParser& parser = config.impl.getCommandLineParser();
        String participantName;

        // use executable name
        const ramses_internal::String& programName = parser.getProgramName();
        if (programName.size() > 0)
        {
            participantName = programName;
            size_t slash = participantName.rfind('/');
            if (slash == String::npos)
            {
                slash = participantName.rfind('\\');
            }
            if (slash != String::npos)
            {
                participantName = participantName.substr(slash + 1, participantName.size() - slash - 1);
            }
        }
        else
        {
            participantName = String("clientName");
        }

        // use communication user
        participantName += "_";

        String someipStr;
        if (SomeIPAdapter::GetCommunicationUserAsString(config.impl.getSomeipCommunicationUserID(), someipStr))
        {
            participantName += someipStr;
        }
        else if (config.impl.getUsedProtocol() == EConnectionProtocol::TCP)
        {
            participantName += "TCP";
        }
        else
        {
            participantName += "UnknownComm";
        }

        return participantName;
    }

    void RamsesFrameworkImpl::SetConsoleLogLevel(ELogLevel logLevel)
    {
        GetRamsesLogger().setConsoleLogLevelProgrammatically(GetELogLevelInternal(logLevel));
    }

    ramses_internal::ELogLevel RamsesFrameworkImpl::GetELogLevelInternal(ELogLevel logLevel)
    {
        switch (logLevel)
        {
        case ramses::ELogLevel::Off:
            return ramses_internal::ELogLevel::Off;
        case ramses::ELogLevel::Fatal:
            return ramses_internal::ELogLevel::Fatal;
        case ramses::ELogLevel::Error:
            return ramses_internal::ELogLevel::Error;
        case ramses::ELogLevel::Warn:
            return ramses_internal::ELogLevel::Warn;
        case ramses::ELogLevel::Info:
            return ramses_internal::ELogLevel::Info;
        case ramses::ELogLevel::Debug:
            return ramses_internal::ELogLevel::Debug;
        case ramses::ELogLevel::Trace:
            return ramses_internal::ELogLevel::Trace;
        }

        assert(false);
        return ramses_internal::ELogLevel::Off;
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
                       if (SomeIPAdapter::IsSomeIPStackCompiled(EConnectionProtocol::SomeIP_HU))
                           sos << " SomeIP-HU";
                       if (SomeIPAdapter::IsSomeIPStackCompiled(EConnectionProtocol::SomeIP_IC))
                           sos << " SomeIP-IC";
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
                       sos << "RamsesBuildInfo: Version " << ramses_sdk::RAMSES_SDK_PROJECT_VERSION_STRING
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
