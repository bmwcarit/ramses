//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshCommunicationChannelDLT.h"
#include "Ramsh/Ramsh.h"
#include "Ramsh/RamshTools.h"
#include "Utils/LogMacros.h"
#include "Utils/RamsesLogger.h"


namespace ramses_internal
{
    RamshCommunicationChannelDLT* RamshCommunicationChannelDLT::m_instance = nullptr;

    int RamshCommunicationChannelDLT::dltInjectionCallbackF(uint32_t sid, void* data, uint32_t length)
    {
        String incoming = String(reinterpret_cast<char*>(data), 0, length - 1);//use length to avoid unterminated strings copied to target buffer

        LOG_DEBUG(CONTEXT_RAMSH, "Received dlt injection with service id " << sid << ", length is " << length);
        LOG_INFO(CONTEXT_RAMSH, "Calling command '" << incoming << "' received from dlt injection");

        if (RamshCommunicationChannelDLT::m_instance)
        {
            RamshCommunicationChannelDLT::m_instance->processInput(incoming);
        }
        return 0;
    }

    RamshCommunicationChannelDLT::RamshCommunicationChannelDLT(Ramsh& ramsh)
        : m_ramsh(ramsh)
    {
        m_instance = this;
        const UInt32 serviceId = 5000u;
        GetRamsesLogger().registerInjectionCallback(CONTEXT_RAMSH, serviceId, &RamshCommunicationChannelDLT::dltInjectionCallbackF);
    }

    RamshCommunicationChannelDLT::~RamshCommunicationChannelDLT()
    {
        if (this == m_instance)
        {
            m_instance = nullptr;
        }
    }

    void RamshCommunicationChannelDLT::processInput(const String& s)
    {
        m_ramsh.execute(RamshTools::parseCommandString(s));
    }
}
