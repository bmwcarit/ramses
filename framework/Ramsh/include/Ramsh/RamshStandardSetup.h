//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHSTANDARDSETUP_H
#define RAMSES_RAMSHSTANDARDSETUP_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Ramsh/Ramsh.h"
#include "Collections/String.h"

namespace ramses_internal
{
    class RamshCommunicationChannelConsole;

    class RamshStandardSetup : public Ramsh
    {
    public:
        explicit RamshStandardSetup(ramses::ERamsesShellType type, String prompt = "ramses");
        virtual ~RamshStandardSetup() override;

        bool start();
        bool stop();

    private:
        const ramses::ERamsesShellType m_type;
        const String m_prompt;
        bool m_started = false;
        std::unique_ptr<RamshCommunicationChannelConsole> m_consoleChannel;
    };

}

#endif
