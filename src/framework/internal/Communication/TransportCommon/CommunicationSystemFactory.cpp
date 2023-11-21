//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Communication/TransportCommon/CommunicationSystemFactory.h"

#include "internal/Core/Utils/LogMacros.h"
#include "internal/PlatformAbstraction/PlatformEnvironmentVariables.h"

#include "internal/Communication/TransportCommon/FakeConnectionSystem.h"
#include "internal/Communication/TransportCommon/FakeDiscoveryDaemon.h"
#include "internal/Communication/TransportCommon/IDiscoveryDaemon.h"
#include "internal/Communication/TransportCommon/ICommunicationSystem.h"

#if defined(HAS_TCP_COMM)
#include "internal/Communication/TransportTCP/TCPConnectionSystem.h"
#include "internal/Communication/TransportTCP/TcpDiscoveryDaemon.h"
#endif

#include "impl/RamsesFrameworkConfigImpl.h"
#include "ramses/framework/RamsesFrameworkConfig.h"
#include <memory>


namespace ramses::internal
{
    namespace {

#if defined(HAS_TCP_COMM)
        // Construct TCPConnectionSystem
        auto ConstructTCPConnectionManager(const RamsesFrameworkConfigImpl& config, const ParticipantIdentifier& participantIdentifier,
            PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection)
        {
            LOG_INFO(CONTEXT_COMMUNICATION, "Use TCPConnectionSystem");

            // own address
            const bool isDaemon = false;
            const NetworkParticipantAddress participantNetworkAddress(participantIdentifier.getParticipantId(), participantIdentifier.getParticipantName(), config.m_tcpConfig.getIPAddress(), config.m_tcpConfig.getPort(isDaemon));

            LOG_DEBUG(CONTEXT_COMMUNICATION, "ConstructTCPConnectionManager: My Address: {}:{}", participantNetworkAddress.getIp(), participantNetworkAddress.getPort());

            // daemon address
            const NetworkParticipantAddress daemonNetworkAddress = NetworkParticipantAddress(TCPConnectionSystem::GetDaemonId(), "SM", config.m_tcpConfig.getDaemonIPAddress(), config.m_tcpConfig.getDaemonPort());

            LOG_DEBUG(CONTEXT_COMMUNICATION, "ConstructTCPConnectionManager: Daemon Address: {}:{}", daemonNetworkAddress.getIp(), daemonNetworkAddress.getPort());

            // allocate
            return std::make_unique<TCPConnectionSystem>(participantNetworkAddress, config.getProtocolVersion(), daemonNetworkAddress, false, frameworkLock, statisticCollection, config.m_tcpConfig.getAliveInterval(), config.m_tcpConfig.getAliveTimeout());
        }
#endif
    }

    std::unique_ptr<IDiscoveryDaemon> CommunicationSystemFactory::ConstructDiscoveryDaemon([[maybe_unused]] const RamsesFrameworkConfigImpl& config,
                                                                                           [[maybe_unused]] PlatformLock&                    frameworkLock,
                                                                                           [[maybe_unused]] StatisticCollectionFramework&    statisticCollection,
                                                                                           [[maybe_unused]] Ramsh*                           optionalRamsh)
    {
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

            case EConnectionProtocol::Off:
                constructedDaemon = std::make_unique<FakeDiscoveryDaemon>();
                break;

            case EConnectionProtocol::Invalid:
                break;
        }
        return constructedDaemon;
    }

    std::unique_ptr<ICommunicationSystem> CommunicationSystemFactory::ConstructCommunicationSystem(const RamsesFrameworkConfigImpl&               config,
                                                                                                   [[maybe_unused]] const ParticipantIdentifier&  participantIdentifier,
                                                                                                   [[maybe_unused]] PlatformLock&                 frameworkLock,
                                                                                                   [[maybe_unused]] StatisticCollectionFramework& statisticCollection)
    {
        switch (config.getUsedProtocol())
        {
#if defined(HAS_TCP_COMM)
        case EConnectionProtocol::TCP:
        {
            return ConstructTCPConnectionManager(config, participantIdentifier, frameworkLock, statisticCollection);
        }
#endif
        case EConnectionProtocol::Off:
        {
            LOG_INFO(CONTEXT_COMMUNICATION, "Using no connection system");
            return std::make_unique<FakeConnectionSystem>();
        }
        default:
            LOG_FATAL(CONTEXT_COMMUNICATION, "Unable to construct connection system for given protocol: {}. Ensure that TCP or the fake connection system is enabled.", config.getUsedProtocol());
            assert(false && "Unable to construct connection system for given protocol. Ensure that TCP or the fake connection system is enabled.");
            return nullptr;
        }
    }
}
