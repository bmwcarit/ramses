//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"

#include <string>
#include <chrono>

namespace ramses::internal
{
    class TCPConfig
    {
    public:
        TCPConfig();

        [[nodiscard]] uint16_t getPort(bool isDaemon = false) const;
        [[nodiscard]] const std::string& getIPAddress() const;
        [[nodiscard]] uint16_t getDaemonPort() const;
        [[nodiscard]] const std::string& getDaemonIPAddress() const;

        void setPort(uint16_t port);
        void setIPAddress(std::string_view ipAddress);
        void setDaemonPort(uint16_t port);
        void setDaemonIPAddress(std::string_view ipAddress);

        [[nodiscard]] std::chrono::milliseconds getAliveInterval() const;
        [[nodiscard]] std::chrono::milliseconds getAliveTimeout() const;
        void setAliveInterval(std::chrono::milliseconds interval);
        void setAliveTimeout(std::chrono::milliseconds timeout);

    private:
        static const uint16_t DefaultPort;
        static const uint16_t DefaultDaemonPort;

        bool m_portHasBeenSet{false};
        uint16_t m_port;
        std::string m_ipAddress;
        uint16_t m_daemonPort;
        std::string m_daemonIP;
        std::chrono::milliseconds m_aliveInterval;
        std::chrono::milliseconds m_aliveTimeout;
    };
}
