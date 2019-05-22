//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TransportTCP//TcpDiscoveryDaemon.h"
#include "TransportTCP/NetworkParticipantAddress.h"
#include "TransportTCP/TCPConnectionSystem.h"
#include "RamsesFrameworkConfigImpl.h"
#include "TransportCommon/RamsesTransportProtocolVersion.h"
#include "Ramsh/Ramsh.h"

namespace ramses_internal
{
    TcpDiscoveryDaemon::TcpDiscoveryDaemon(const ramses::RamsesFrameworkConfigImpl& config, PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection, Ramsh* optionalRamsh)
        : m_started(false)
    {
        // own address
        const Bool isDaemon = true;
        const NetworkParticipantAddress participantNetworkAddress(TCPConnectionSystem::GetDaemonId(), "SM", config.m_tcpConfig.getIPAddress(), config.m_tcpConfig.getPort(isDaemon));

        LOG_DEBUG(CONTEXT_COMMUNICATION, "TcpDiscoveryDaemon::TcpDiscoveryDaemon: My Address: " << participantNetworkAddress.getIp() << ":" << participantNetworkAddress.getPort());

        const NetworkParticipantAddress daemonNetworkAddress;
        m_communicationSystem.reset(new TCPConnectionSystem(participantNetworkAddress, config.getProtocolVersion(), daemonNetworkAddress, true, frameworkLock, statisticCollection, config.m_tcpConfig.getAliveInterval(), config.m_tcpConfig.getAliveTimeout()));

        if (optionalRamsh)
        {
            m_commandLogConnectionInformation.reset(new LogConnectionInfo(*m_communicationSystem));
            optionalRamsh->add(*m_commandLogConnectionInformation);
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
