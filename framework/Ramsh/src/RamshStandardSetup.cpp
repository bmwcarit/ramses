//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshStandardSetup.h"

namespace ramses_internal
{
    RamshStandardSetup::RamshStandardSetup(String prompt /* = "noname" */):
        RamshDLT(prompt)
        , m_consoleChannel()
    {
    }

    bool RamshStandardSetup::start()
    {
        if (m_started)
        {
            return false;
        }

        m_consoleChannel.startThread();
        m_consoleChannel.registerRamsh(*this);
        return RamshDLT::start();
    }

    bool RamshStandardSetup::internalStop()
    {
        if (!m_started)
        {
            return false;
        }

        m_consoleChannel.stopThread();
        return RamshDLT::stop();
    }

    bool RamshStandardSetup::stop()
    {
        return internalStop();

    }

    RamshStandardSetup::~RamshStandardSetup()
    {
        internalStop();
    }
}

