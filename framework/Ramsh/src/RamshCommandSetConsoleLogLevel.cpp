//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshCommandSetConsoleLogLevel.h"
#include "Ramsh/Ramsh.h"
#include "Utils/LogMacros.h"
#include "Utils/RamsesLogger.h"
#include "Utils/LogHelper.h"

namespace ramses_internal
{
    RamshCommandSetConsoleLogLevel::RamshCommandSetConsoleLogLevel(const Ramsh& ramsh) : m_ramsh(ramsh)
    {
        //commands to set the log level for console appender
        registerKeyword("setLogLevelConsole");
        registerKeyword("lc");
        description = "Command to set the console log level. Usage: setLogLevelConsole {off | fatal | error | warn | info | debug | trace}";
    }

    bool RamshCommandSetConsoleLogLevel::executeInput(const std::vector<std::string>& input)
    {
        if (input.size() != 2)
            return false;

        ELogLevel level;
        if (!LogHelper::StringToLogLevel(String(input[1]), level))
            return false;


        LOG_INFO(CONTEXT_RAMSH, "Logging Console in level: " << RamsesLogger::GetLogLevelText(level));
        GetRamsesLogger().setConsoleLogLevel(level);
        return true;
    }
}
