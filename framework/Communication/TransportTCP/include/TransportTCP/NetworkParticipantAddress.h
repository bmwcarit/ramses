//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_NETWORKPARTICIPANTADDRESS_H
#define RAMSES_NETWORKPARTICIPANTADDRESS_H

#include "Common/ParticipantIdentifier.h"

namespace ramses_internal
{
    class NetworkParticipantAddress : public ParticipantIdentifier
    {
    public:
        NetworkParticipantAddress()
            : ParticipantIdentifier()
            , m_ip("127.0.0.1")
            , m_port(0)
        {
        };

        NetworkParticipantAddress(const Guid& id, const String& name, const String& ip, UInt16 port)
            : ParticipantIdentifier(id, name)
            , m_ip(ip)
            , m_port(port)
        {
        }

        const String& getIp() const
        {
            return m_ip;
        }

        UInt16 getPort() const
        {
            return m_port;
        }

    private:
        String m_ip;
        UInt16 m_port;
    };
}


#endif
