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


namespace ramses_internal
{
    namespace {

#if defined(HAS_TCP_COMM)
        // Construct TCPConnectionSystem
        TCPConnectionSystem* ConstructTCPConnectionManager(const ramses::RamsesFrameworkConfigImpl& config, const ParticipantIdentifier& participantIdentifier,
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
            return new TCPConnectionSystem(participantNetworkAddress, config.getProtocolVersion(), daemonNetworkAddress, false, frameworkLock, statisticCollection, config.m_tcpConfig.getAliveInterval(), config.m_tcpConfig.getAliveTimeout());
        }
#endif

    }

    IDiscoveryDaemon* CommunicationSystemFactory::ConstructDiscoveryDaemon(const ramses::RamsesFrameworkConfigImpl& config, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection, Ramsh* optionalRamsh)
    {
        UNUSED(config);
        UNUSED(frameworkLock);
        UNUSED(statisticCollection);
        UNUSED(optionalRamsh);

        IDiscoveryDaemon* constructedDaemon = 0;
        switch(config.getUsedProtocol())
        {
            case EConnectionProtocol_TCP:
            {
#if defined(HAS_TCP_COMM)
                constructedDaemon = new TcpDiscoveryDaemon(config, frameworkLock, statisticCollection, optionalRamsh);
                break;
#endif
            }

            case EConnectionProtocol_Fake:
                constructedDaemon = new FakeDiscoveryDaemon();
                break;

            default:
                break;
        }
        return constructedDaemon;
    }

    ICommunicationSystem* CommunicationSystemFactory::ConstructCommunicationSystem(const ramses::RamsesFrameworkConfigImpl& config, const ParticipantIdentifier& participantIdentifier,
        PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection)
    {
        UNUSED(participantIdentifier);
        UNUSED(frameworkLock);
        UNUSED(statisticCollection)

        switch (config.getUsedProtocol())
        {
#if defined(HAS_TCP_COMM)
        case EConnectionProtocol_TCP:
        {
            TCPConnectionSystem* connectionManager = ConstructTCPConnectionManager(config, participantIdentifier, frameworkLock, statisticCollection);
            return connectionManager;
        }
#endif
        case EConnectionProtocol_Fake:
        {
            LOG_INFO(CONTEXT_COMMUNICATION, "Using no connection system");
            return new FakeConnectionSystem();
        }
        default:
            assert(false && "Unable to construct connection system for given protocol");
            LOG_FATAL(CONTEXT_COMMUNICATION, "Unable to construct connection system for given protocol: " << config.getUsedProtocol());
            return 0;
        }
    }
}
