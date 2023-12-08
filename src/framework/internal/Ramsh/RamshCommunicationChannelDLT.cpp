//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Ramsh/RamshCommunicationChannelDLT.h"
#include "internal/Ramsh/Ramsh.h"
#include "internal/Ramsh/RamshTools.h"
#include "internal/Core/Utils/LogMacros.h"
#include "impl/RamsesLoggerImpl.h"

#include <string>

namespace ramses::internal
{
    int RamshCommunicationChannelDLT::dltInjectionCallbackF(uint32_t sid, void* data, uint32_t length)
    {
        std::string incoming{static_cast<const char*>(data), 0, length - 1};//use length to avoid unterminated strings copied to target buffer

        LOG_DEBUG(CONTEXT_RAMSH, "Received dlt injection with service id {}, length is {}", sid, length);
        LOG_INFO(CONTEXT_RAMSH, "Calling command '{}' received from dlt injection", incoming);

        RamshCommunicationChannelDLT::GetInstance().processInput(incoming);

        return 0;
    }

    RamshCommunicationChannelDLT::RamshCommunicationChannelDLT()
    {
        const uint32_t serviceId = 5000u;
        GetRamsesLogger().registerInjectionCallback(CONTEXT_RAMSH, serviceId, &RamshCommunicationChannelDLT::dltInjectionCallbackF);
    }

    void RamshCommunicationChannelDLT::registerRamsh(Ramsh& ramsh)
    {
        // if used in parallel with multiple instances, the last registered ramsh is used
        std::lock_guard<std::mutex> guard(m_ramshLock);
        m_ramsh = &ramsh;
    }

    void RamshCommunicationChannelDLT::unregisterRamsh(Ramsh& ramsh)
    {
        // if used in parallel with multiple instances, the currently used ramsh might not be the one to unregister
        std::lock_guard<std::mutex> guard(m_ramshLock);
        if (m_ramsh == &ramsh)
            m_ramsh = nullptr;
    }

    void RamshCommunicationChannelDLT::processInput(const std::string& s)
    {
        std::lock_guard<std::mutex> guard(m_ramshLock);
        if (m_ramsh)
            m_ramsh->execute(RamshTools::parseCommandString(s));
    }
}
