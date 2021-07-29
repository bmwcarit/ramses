//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshCommandDcsmStatusMessage.h"
#include "Ramsh/Ramsh.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    RamshCommandSendDcsmStatusMessage::RamshCommandSendDcsmStatusMessage()
    {
        registerKeyword("senddcsmstatus");
        description = "send a dcsm contentstatusmessage to provider of content";
    }

    bool RamshCommandSendDcsmStatusMessage::executeInput(const std::vector<std::string>& input)
    {
        if (input.size() != 2)
            return false;

        LOG_INFO(CONTEXT_RAMSH, "Received ramsh command senddcsmstatus");

        const std::string& argVal(input[1]);
        uint64_t contentId = atoll(argVal.c_str());
        if (!contentId)
            return false;

        std::lock_guard<std::mutex> lock(m_mutex);
        m_contentsSendRequested.push(ramses::ContentID{ contentId });
        return true;
    }

    ramses::ContentID RamshCommandSendDcsmStatusMessage::popContentID()
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_contentsSendRequested.empty())
            return ramses::ContentID::Invalid();
        else
        {
            auto content = m_contentsSendRequested.front();
            m_contentsSendRequested.pop();
            return content;
        }
    }
}
