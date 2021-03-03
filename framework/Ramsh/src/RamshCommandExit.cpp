//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshCommandExit.h"
#include "Ramsh/Ramsh.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    RamshCommandExit::RamshCommandExit()
        : m_exitRequested(false)
    {
        registerKeyword("exit");
        description = "exit program";
    }

    bool RamshCommandExit::executeInput(const std::vector<std::string>& input)
    {
        UNUSED(input);
        LOG_INFO(CONTEXT_RAMSH, "Received ramsh command exit");
        m_exitRequested = true;
        m_exitEvent.signal();
        return true;
    }

    bool RamshCommandExit::exitRequested()
    {
        return m_exitRequested.load();
    }

    void RamshCommandExit::waitForExitRequest(uint32_t timeoutMillisec)
    {
        m_exitEvent.wait(timeoutMillisec);
    }
}
