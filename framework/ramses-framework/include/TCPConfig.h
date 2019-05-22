//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TCPCONFIG_H
#define RAMSES_TCPCONFIG_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Collections/String.h"
#include <chrono>

namespace ramses
{
    class TCPConfig
    {
    public:
        TCPConfig();

        uint16_t getPort(bool isDaemon = false) const;
        const ramses_internal::String& getIPAddress() const;
        uint16_t getDaemonPort() const;
        const ramses_internal::String& getDaemonIPAddress() const;

        void setPort(uint16_t port);
        void setIPAddress(const ramses_internal::String& ipAddress);
        void setDaemonPort(uint16_t port);
        void setDaemonIPAddress(const ramses_internal::String& ipAddress);

        std::chrono::milliseconds getAliveInterval() const;
        std::chrono::milliseconds getAliveTimeout() const;
        void setAliveInterval(std::chrono::milliseconds interval);
        void setAliveTimeout(std::chrono::milliseconds timeout);

    private:
        static const uint16_t DefaultPort;
        static const uint16_t DefaultDaemonPort;

        bool m_portHasBeenSet;
        uint16_t m_port;
        ramses_internal::String m_ipAddress;
        uint16_t m_daemonPort;
        ramses_internal::String m_daemonIP;
        std::chrono::milliseconds m_aliveInterval;
        std::chrono::milliseconds m_aliveTimeout;
    };
}

#endif
