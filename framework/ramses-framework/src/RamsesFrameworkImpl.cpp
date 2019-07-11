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
#include "Utils/RamsesLogger.h"
#include "RamsesFrameworkConfigImpl.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "Ramsh/RamshFactory.h"
#include "PlatformAbstraction/synchronized_clock.h"
#include "DcsmProviderImpl.h"
#include "ramses-framework-api/DcsmProvider.h"
#include "DcsmConsumerImpl.h"
#include "ramses-framework-api/DcsmConsumer.h"

namespace ramses
{
    using namespace ramses_internal;

    RamsesFrameworkImpl::RamsesFrameworkImpl(const RamsesFrameworkConfigImpl& config, const ramses_internal::ParticipantIdentifier& participantAddress)
        : StatusObjectImpl()
        , m_ramsh(RamshFactory::ConstructRamsh(config))
        , m_participantAddress(participantAddress)
        // NOTE: if you add something here consider using m_frameworkLock for all locking purposes inside this new class
        , m_connectionProtocol(config.getUsedProtocol())
        , m_communicationSystem(CommunicationSystemFactory::ConstructCommunicationSystem(config, participantAddress, m_frameworkLock, m_statisticCollection))
        , m_periodicLogger(m_frameworkLock, m_statisticCollection)
        , m_connected(false)
        , m_threadWatchdogConfig(config.m_watchdogConfig)
        // NOTE: ThreadingSystem must always be constructed after CommunicationSystem
        , m_threadStrategy(3, config.m_watchdogConfig)
        , m_resourceComponent(m_threadStrategy.e, m_participantAddress.getParticipantId(), *m_communicationSystem, m_communicationSystem->getRamsesConnectionStatusUpdateNotifier(),
            m_statisticCollection, m_frameworkLock, config.getMaximumTotalBytesForAsyncResourceLoading())
        , m_scenegraphComponent(m_participantAddress.getParticipantId(), *m_communicationSystem, m_communicationSystem->getRamsesConnectionStatusUpdateNotifier(), m_frameworkLock)
        , m_dcsmComponent(m_participantAddress.getParticipantId(), *m_communicationSystem, m_communicationSystem->getDcsmConnectionStatusUpdateNotifier(), m_frameworkLock)
        , m_ramshCommandLogConnectionInformation(*m_communicationSystem)
        , m_ramshCommandLogDcsmInformation(m_dcsmComponent)
    {
        m_ramsh->start();
        m_ramsh->add(m_ramshCommandLogConnectionInformation);
        m_ramsh->add(m_ramshCommandLogDcsmInformation);
        m_periodicLogger.registerPeriodicLogSupplier(m_communicationSystem.get());
        m_periodicLogger.registerPeriodicLogSupplier(&m_dcsmComponent);
    }

    RamsesFrameworkImpl::~RamsesFrameworkImpl()
    {
        LOG_INFO(CONTEXT_CLIENT, "RamsesFramework::~RamsesFramework: guid " << m_participantAddress.getParticipantId() << ", wasConnected " << m_connected);
        if (m_connected)
        {
            disconnect();
        }
        m_periodicLogger.removePeriodicLogSupplier(m_communicationSystem.get());
        m_periodicLogger.removePeriodicLogSupplier(&m_dcsmComponent);
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

    ramses_internal::IConnectionStatusUpdateNotifier& RamsesFrameworkImpl::getRamsesConnectionStatusUpdateNotifier()
    {
        return m_communicationSystem->getRamsesConnectionStatusUpdateNotifier();
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
        return m_threadStrategy.e;
    }

    ramses_internal::PeriodicLogger& RamsesFrameworkImpl::getPeriodicLogger()
    {
        return m_periodicLogger;
    }

    ramses_internal::StatisticCollectionFramework& RamsesFrameworkImpl::getStatisticCollection()
    {
        return m_statisticCollection;
    }

    ramses::status_t RamsesFrameworkImpl::connect()
    {
        if (m_connected)
        {
            return addErrorEntry("Already connected, cannot connect twice");
        }

        if (!m_communicationSystem->connectServices())
        {
            return addErrorEntry("Could not connect to daemon");
        }

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
            addErrorEntry("RamsesFramework::createDcsmProvider: can only create one provider per framework");
            return nullptr;
        }
        DcsmProviderImpl* impl = new DcsmProviderImpl(getDcsmComponent());
        m_dcsmProvider.reset(new DcsmProvider(*impl));
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
            addErrorEntry("RamsesFramework::createDcsmConsumer: can only create one consumer per framework");
            return nullptr;
        }
        DcsmConsumerImpl* impl = new DcsmConsumerImpl(*this);
        m_dcsmConsumer.reset(new DcsmConsumer(*impl));
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

    RamsesFrameworkImpl& RamsesFrameworkImpl::createImpl(int32_t argc, const char * argv[])
    {
        RamsesFrameworkConfig config(argc, argv);
        return createImpl(config);
    }

    RamsesFrameworkImpl& RamsesFrameworkImpl::createImpl(const RamsesFrameworkConfig& config)
    {
        const ramses_internal::CommandLineParser& parser = config.impl.getCommandLineParser();
        ramses_internal::GetRamsesLogger().initialize(parser, config.getDLTApplicationID(), config.getDLTApplicationDescription(), false);

        const ramses_internal::String& participantName = GetParticipantName(config);

        ramses_internal::Guid myGuid(false);
        if (myGuid.isInvalid())
        {
            // try from user first, fall back to random if still invalid
            myGuid = config.impl.getUserProvidedGuid();

            // make sure generated ids do not start with all zeros to avoid collisions with explicit guids
            while (myGuid.isInvalid() || myGuid.getGuidData().Data1 == 0)
            {
                myGuid = Guid(true);
            }
        }
        ramses_internal::ParticipantIdentifier participantAddress(myGuid, participantName);

        LOG_INFO(CONTEXT_FRAMEWORK, "Starting Ramses Client Application: " << participantAddress.getParticipantName() << " guid:" << participantAddress.getParticipantId());
        const ramses_internal::ArgumentUInt32 periodicLogTimeout(parser, "plt", "periodicLogTimeout", uint32_t(PeriodicLogIntervalInSeconds));

        logEnvironmentVariableIfSet("XDG_RUNTIME_DIR");
        logEnvironmentVariableIfSet("LIBGL_DRIVERS_PATH");

        // trigger initialization of synchronized time and print as reference (or let Get* print error when init failed)
        const auto currentSyncTime = synchronized_clock::now();
        if (asMilliseconds(currentSyncTime) != 0)
        {
            const UInt64 systemClockTime = PlatformTime::GetMicrosecondsAbsolute();
            LOG_INFO(CONTEXT_FRAMEWORK, "Ramses synchronized time support enabled using " << synchronized_clock::source() <<
                     ". Currrent sync time " << asMicroseconds(currentSyncTime) << " us, system clock is " << systemClockTime << " us");
        }

        RamsesFrameworkImpl* impl = new RamsesFrameworkImpl(config.impl, participantAddress);
        if (config.impl.m_periodicLogsEnabled)
        {
            impl->m_periodicLogger.startLogging(periodicLogTimeout);
        }
        return *impl;
    }

    ramses_internal::String RamsesFrameworkImpl::GetParticipantName(const RamsesFrameworkConfig& config)
    {
        const CommandLineParser& parser = config.impl.getCommandLineParser();
        String participantName;

        // use executable name
        const ramses_internal::String& programName = parser.getProgramName();
        if (programName.getLength() > 0)
        {
            participantName = programName;
            Int slash = participantName.lastIndexOf('/');
            if (slash < 0)
            {
                slash = participantName.lastIndexOf('\\');
            }
            if (slash >= 0)
            {
                participantName = participantName.substr(slash + 1, participantName.getLength() - slash - 1);
            }
        }
        else
        {
            participantName = String("clientName");
        }

        // use communication user
        participantName += "_";

        if (config.impl.getUsedProtocol() == EConnectionProtocol_TCP)
        {
            participantName += "TCP";
        }
        else
        {
            participantName += "UnknownComm";
        }

        return participantName;
    }

    void RamsesFrameworkImpl::logEnvironmentVariableIfSet(const ramses_internal::String& envVarName)
    {
        ramses_internal::String envVarValue;
        // TODO(tobias) envVarValue.getLength should not be there because empty variable is also set. remove when capu fixed
        if (ramses_internal::PlatformEnvironmentVariables::get(envVarName, envVarValue) && envVarValue.getLength() != 0)
        {
            LOG_INFO(CONTEXT_FRAMEWORK, "Environment variable set: " << envVarName << "=" << envVarValue);
        }
    }
}
