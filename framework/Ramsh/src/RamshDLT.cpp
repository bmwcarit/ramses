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
    RamshDLT::RamshDLT(String prompt /* = "noname" */):
        Ramsh(prompt)
        , m_started(false)
        , m_dltChannel()
    {
    }

    bool RamshDLT::start()
    {
        if (m_started)
        {
            return false;
        }

        m_dltChannel.registerRamsh(*this);
        m_started = true;

        return true;
    }


    bool RamshDLT::internalStop()
    {
        if (!m_started)
        {
            return false;
        }

        m_started = false;
        return true;
    }

    bool RamshDLT::stop()
    {
        return internalStop();
    }

    RamshDLT::~RamshDLT()
    {
        internalStop();
    }
}

