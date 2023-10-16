//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFrameworkTypes.h"
#include "internal/Ramsh/Ramsh.h"

#include <string>

namespace ramses::internal
{
    class RamshCommunicationChannelConsole;

    class RamshStandardSetup : public Ramsh
    {
    public:
        explicit RamshStandardSetup(ERamsesShellType type, std::string prompt = "ramses");
        ~RamshStandardSetup() override;

        bool start();
        bool stop();

    private:
        const ERamsesShellType m_type;
        const std::string m_prompt;
        bool m_started = false;
        std::unique_ptr<RamshCommunicationChannelConsole> m_consoleChannel;
    };

}
