//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHCOMMANDDCSMSTATUSMESSAGE_H
#define RAMSES_RAMSHCOMMANDDCSMSTATUSMESSAGE_H

#include "Ramsh/RamshCommand.h"
#include "ramses-framework-api/DcsmApiTypes.h"

#include <queue>
#include <mutex>

namespace ramses_internal
{
    class Ramsh;

    class RamshCommandSendDcsmStatusMessage : public RamshCommand
    {
    public:
        explicit RamshCommandSendDcsmStatusMessage();
        virtual bool executeInput(const std::vector<std::string>& input) override;

        ramses::ContentID popContentID();

    private:
        std::mutex m_mutex;
        std::queue<ramses::ContentID> m_contentsSendRequested;
    };
}

#endif
