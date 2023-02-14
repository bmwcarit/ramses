//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportCommon/CommunicationSystemFactory.h"

#include "Utils/Argument.h"
#include "Utils/LogMacros.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"

#include "TransportCommon/FakeConnectionSystem.h"
#include "TransportCommon/FakeDiscoveryDaemon.h"
#include "TransportCommon/IDiscoveryDaemon.h"
#include "TransportCommon/ICommunicationSystem.h"

#if defined(HAS_TCP_COMM)
#include "TransportTCP/TCPConnectionSystem.h"
#include "TransportTCP/TcpDiscoveryDaemon.h"
#endif

#include "RamsesFrameworkConfigImpl.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include <memory>


namespace ramses_internal
{
    namespace {

#if defined(HAS_TCP_COMM)
        // Construct TCPConnectionSystem
        auto ConstructTCPConnectionManager(const ramses::RamsesFrameworkConfigImpl& config, const ParticipantIdentifier& participantIdentifier,
            PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection)
        {
            LOG_INFO(CONTEXT_COMMUNICATION, "Use TCPConnectionSystem");

            // own address
            const bool isDaemon = false;
            const NetworkParticipantAddress participantNetworkAddress(participantIdentifier.getParticipantId(), participantIdentifier.getParticipantName(), config.m_tcpConfig.getIPAddress(), config.m_tcpConfig.getPort(isDaemon));

            LOG_DEBUG(CONTEXT_COMMUNICATION, "ConstructTCPConnectionManager: My Address: " << participantNetworkAddress.getIp() << ":" << participantNetworkAddress.getPort());

            // daemon address
            const NetworkParticipantAddress daemonNetworkAddress = NetworkParticipantAddress(TCPConnectionSystem::GetDaemonId(), "SM", config.m_tcpConfig.getDaemonIPAddress(), config.m_tcpConfig.getDaemonPort());

            LOG_DEBUG(CONTEXT_COMMUNICATION, "ConstructTCPConnectionManager: Daemon Address: " << daemonNetworkAddress.getIp() << ":" << daemonNetworkAddress.getPort());

            // allocate
            return std::make_unique<TCPConnectionSystem>(participantNetworkAddress, config.getProtocolVersion(), daemonNetworkAddress, false, frameworkLock, statisticCollection, config.m_tcpConfig.getAliveInterval(), config.m_tcpConfig.getAliveTimeout());
        }
#endif
    }

    std::unique_ptr<IDiscoveryDaemon> CommunicationSystemFactory::ConstructDiscoveryDaemon(const ramses::RamsesFrameworkConfigImpl& config, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection, Ramsh* optionalRamsh)
    {
        UNUSED(config);
        UNUSED(frameworkLock);
        UNUSED(statisticCollection);
        UNUSED(optionalRamsh);

        std::unique_ptr<IDiscoveryDaemon> constructedDaemon;
        switch(config.getUsedProtocol())
        {
            case EConnectionProtocol::TCP:
            {
#if defined(HAS_TCP_COMM)
                constructedDaemon = std::make_unique<TcpDiscoveryDaemon>(config, frameworkLock, statisticCollection, optionalRamsh);
#endif
            }
                break;

            case EConnectionProtocol::Fake:
#if defined(HAS_FALLBACK_FAKE_COMMUNICATION_SYSTEM)
                constructedDaemon = std::make_unique<FakeDiscoveryDaemon>();
#endif
                break;

            case EConnectionProtocol::Invalid:
                break;
        }
        return constructedDaemon;
    }

    std::unique_ptr<ICommunicationSystem> CommunicationSystemFactory::ConstructCommunicationSystem(const ramses::RamsesFrameworkConfigImpl& config, const ParticipantIdentifier& participantIdentifier,
        PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection)
    {
        UNUSED(participantIdentifier);
        UNUSED(frameworkLock);
        UNUSED(statisticCollection);

        switch (config.getUsedProtocol())
        {
#if defined(HAS_TCP_COMM)
        case EConnectionProtocol::TCP:
        {
            return ConstructTCPConnectionManager(config, participantIdentifier, frameworkLock, statisticCollection);
        }
#endif
#if defined(HAS_FALLBACK_FAKE_COMMUNICATION_SYSTEM)
        case EConnectionProtocol::Fake:
        {
            LOG_INFO(CONTEXT_COMMUNICATION, "Using no connection system");
            return std::make_unique<FakeConnectionSystem>();
        }
#endif
        default:
            LOG_FATAL(CONTEXT_COMMUNICATION, "Unable to construct connection system for given protocol: " << config.getUsedProtocol() << ". Ensure that TCP or the fake connection system is enabled.");
            assert(false && "Unable to construct connection system for given protocol. Ensure that TCP or the fake connection system is enabled.");
            return nullptr;
        }
    }
}
