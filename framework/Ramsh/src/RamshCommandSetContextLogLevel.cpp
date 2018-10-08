//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshCommandSetContextLogLevel.h"
#include "Ramsh/Ramsh.h"
#include "Collections/HashSet.h"
#include "Utils/LogMacros.h"
#include "Common/Cpp11Macros.h"
#include "Utils/RamsesLogger.h"

namespace ramses_internal
{
    RamshCommandSetContextLogLevel::RamshCommandSetContextLogLevel(const Ramsh& ramsh) : m_ramsh(ramsh)
    {
        //commands to set a common log level for all appenders
        registerKeyword("setContextLogLevel");
        registerKeyword("cl");

        description = "Commands to set the log level of all contexts. Usage: setContextLogLevel <0..7>";
    }

    Bool RamshCommandSetContextLogLevel::executeInput(const RamshInput& input)
    {
        if (input.size() != 2)
        {
            LOG_ERROR(CONTEXT_RAMSH, "RamshCommandSetContextLogLevel: Wrong usage, usage: setContextLogLevel <0..7>");
            return false;
        }

        const ELogLevel logLevel = RamsesLogger::GetLoglevelFromInt(static_cast<Int32>(atoi(input[1].c_str())));
        RamsesLogger& logger = GetRamsesLogger();

        logger.setLogLevelForContexts(logLevel);
        for (const auto& info : GetRamsesLogger().getAllContextsInformation())
        {
            LOG_INFO(CONTEXT_RAMSH, info.id << " | " << info.name
                        << " | "
                        << static_cast<Int32>(info.logLevel)
                        << " | "
                        << RamsesLogger::GetLogLevelText(info.logLevel));
        }
        return true;
    }
}
