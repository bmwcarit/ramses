//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TCPConfig.h"

namespace ramses
{
    const uint16_t TCPConfig::DefaultPort(0);
    const uint16_t TCPConfig::DefaultDaemonPort(5999);

    TCPConfig::TCPConfig()
        : m_portHasBeenSet(false)
        , m_port(DefaultPort)
        , m_ipAddress("127.0.0.1")
        , m_daemonPort(DefaultDaemonPort)
        , m_daemonIP("127.0.0.1")
        , m_aliveInterval(300)
        , m_aliveTimeout(m_aliveInterval * 6)
    {
    }

    uint16_t TCPConfig::getPort(bool isDaemon) const
    {
        return (isDaemon && !m_portHasBeenSet) ? DefaultDaemonPort : m_port;
    }

    const ramses_internal::String& TCPConfig::getIPAddress() const
    {
        return m_ipAddress;
    }

    uint16_t TCPConfig::getDaemonPort() const
    {
        return m_daemonPort;
    }

    const ramses_internal::String& TCPConfig::getDaemonIPAddress() const
    {
        return m_daemonIP;
    }

    void TCPConfig::setPort(uint16_t port)
    {
        m_port = port;
        m_portHasBeenSet = true;
    }

    void TCPConfig::setIPAddress(const ramses_internal::String& ipAddress)
    {
        m_ipAddress = ipAddress;
    }

    void TCPConfig::setDaemonPort(uint16_t port)
    {
        m_daemonPort = port;
    }

    void TCPConfig::setDaemonIPAddress(const ramses_internal::String& ipAddress)
    {
        m_daemonIP = ipAddress;
    }


    std::chrono::milliseconds TCPConfig::getAliveInterval() const
    {
        return m_aliveInterval;
    }

    std::chrono::milliseconds TCPConfig::getAliveTimeout() const
    {
        return m_aliveTimeout;
    }

    void TCPConfig::setAliveInterval(std::chrono::milliseconds interval)
    {
        m_aliveInterval = interval;
    }

    void TCPConfig::setAliveTimeout(std::chrono::milliseconds factor)
    {
        m_aliveTimeout = factor;
    }
}
