//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Common/ParticipantIdentifier.h"

#include <string>
#include <string_view>

namespace ramses::internal
{
    class NetworkParticipantAddress : public ParticipantIdentifier
    {
    public:
        NetworkParticipantAddress() = default;

        NetworkParticipantAddress(const Guid& id, std::string_view name, std::string_view ip, uint16_t port)
            : ParticipantIdentifier(id, name)
            , m_ip(ip)
            , m_port(port)
        {
        }

        [[nodiscard]] const std::string& getIp() const
        {
            return m_ip;
        }

        [[nodiscard]] uint16_t getPort() const
        {
            return m_port;
        }

        bool operator==(const NetworkParticipantAddress& other) const
        {
            return ParticipantIdentifier::operator==(other) &&
                m_ip == other.m_ip &&
                m_port == other.m_port;
        }

        bool operator!=(const NetworkParticipantAddress& other) const
        {
            return !(*this == other);
        }

    private:
        std::string m_ip{"127.0.0.1"};
        uint16_t m_port{0};
    };
}

