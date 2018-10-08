//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSHSTANDARDSETUP_H
#define RAMSES_RAMSHSTANDARDSETUP_H

#include "Ramsh/Ramsh.h"
#include "Ramsh/RamshCommunicationChannelConsole.h"
#include "Ramsh/RamshDLT.h"

namespace ramses_internal
{
    class RamshStandardSetup : public RamshDLT
    {
    public:
        RamshStandardSetup(String prompt = "noname");

        virtual bool start();
        virtual bool stop();

        ~RamshStandardSetup();

    private:
        bool internalStop();
        RamshCommunicationChannelConsole m_consoleChannel;
    };

}// namespace ramses_internal

#endif
