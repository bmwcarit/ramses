//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Ramsh/RamshCommandSetContextLogLevelFilter.h"
#include "internal/Ramsh/Ramsh.h"
#include "internal/Core/Utils/LogMacros.h"
#include "impl/RamsesLoggerImpl.h"

namespace ramses::internal
{
    RamshCommandSetContextLogLevelFilter::RamshCommandSetContextLogLevelFilter(const Ramsh& ramsh)
        : m_ramsh(ramsh)
    {
        //commands to set a common log level for all appenders
        registerKeyword("setContextLogLevelFilter");
        registerKeyword("clf");

        description = "Commands to set the log level of specific contexts. Usage: setContextLogLevelFilter 0..7:ContextIdPattern,0..7:ContextIdPattern,...";
    }

    bool RamshCommandSetContextLogLevelFilter::executeInput(const std::vector<std::string>& input)
    {
        if (input.size() != 2)
        {
            LOG_ERROR(CONTEXT_RAMSH, "RamshCommandSetContextLogLevelFilter: Wrong usage, setContextLogLevelFilter 0..7:ContextIdPattern,0..7:ContextIdPattern,...");
            return false;
        }

        GetRamsesLogger().applyContextFilterCommand(input[1]);
        return true;
    }
}
