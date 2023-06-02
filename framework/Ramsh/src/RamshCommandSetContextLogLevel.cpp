//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshCommandSetContextLogLevel.h"
#include "Ramsh/Ramsh.h"
#include "Utils/LogMacros.h"
#include "Utils/RamsesLogger.h"
#include "Utils/LogHelper.h"

namespace ramses_internal
{
    RamshCommandSetContextLogLevel::RamshCommandSetContextLogLevel(const Ramsh& ramsh) : m_ramsh(ramsh)
    {
        //commands to set a common log level for all contexts
        registerKeyword("setContextLogLevel");
        registerKeyword("cl");

        description = "Commands to set the log level of all contexts. Usage: setContextLogLevel <0..7>";
    }

    bool RamshCommandSetContextLogLevel::executeInput(const std::vector<std::string>& input)
    {
        if (input.size() != 2)
            return false;

        ELogLevel level;
        if (!LogHelper::StringToLogLevel(input[1], level))
            return false;

        RamsesLogger& logger = GetRamsesLogger();
        logger.setLogLevelForContexts(level);

        for (const auto& info : logger.getAllContextsInformation())
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
