//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Communication/TransportTCP//TcpDiscoveryDaemon.h"
#include "internal/Communication/TransportTCP/NetworkParticipantAddress.h"
#include "internal/Communication/TransportTCP/TCPConnectionSystem.h"
#include "impl/RamsesFrameworkConfigImpl.h"
#include "internal/Communication/TransportCommon/RamsesTransportProtocolVersion.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Ramsh/Ramsh.h"

namespace ramses::internal
{
    TcpDiscoveryDaemon::TcpDiscoveryDaemon(const RamsesFrameworkConfigImpl& config, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection, Ramsh* optionalRamsh)
        : m_started(false)
    {
        // own address
        const bool isDaemon = true;
        const NetworkParticipantAddress participantNetworkAddress(TCPConnectionSystem::GetDaemonId(), "SM", config.m_tcpConfig.getIPAddress(), config.m_tcpConfig.getPort(isDaemon));

        LOG_DEBUG(CONTEXT_COMMUNICATION, "TcpDiscoveryDaemon::TcpDiscoveryDaemon: My Address: {}:{}", participantNetworkAddress.getIp(), participantNetworkAddress.getPort());

        const NetworkParticipantAddress daemonNetworkAddress;
        m_communicationSystem = std::make_unique<TCPConnectionSystem>(participantNetworkAddress, config.getProtocolVersion(),
                                                                      daemonNetworkAddress, true,
                                                                      frameworkLock,
                                                                      statisticCollection,
                                                                      config.m_tcpConfig.getAliveInterval(), config.m_tcpConfig.getAliveTimeout());

        if (optionalRamsh)
        {
            m_commandLogConnectionInformation = std::make_shared<LogConnectionInfo>(*m_communicationSystem);
            optionalRamsh->add(m_commandLogConnectionInformation);
        }
    }

    TcpDiscoveryDaemon::~TcpDiscoveryDaemon()
    {
        if (m_started)
        {
            stop();
        }
    }

    bool TcpDiscoveryDaemon::start()
    {
        if (m_started)
        {
            return false;
        }

        if (m_communicationSystem->connectServices())
        {
            m_started = true;
        }
        return m_started;
    }

    bool TcpDiscoveryDaemon::stop()
    {
        if (!m_started)
        {
            return false;
        }
        m_communicationSystem->disconnectServices();
        m_started = false;
        return true;
    }
}
