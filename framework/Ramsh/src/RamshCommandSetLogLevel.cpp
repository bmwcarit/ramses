//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Ramsh/RamshCommandSetLogLevel.h"
#include "Ramsh/Ramsh.h"
#include "Collections/HashSet.h"
#include "Utils/LogMacros.h"
#include "Utils/RamsesLogger.h"

namespace ramses_internal
{
    RamshCommandSetLogLevel::RamshCommandSetLogLevel(const Ramsh& ramsh) : m_ramsh(ramsh)
    {
        //commands to set a common log level for all appenders
        registerKeyword("setLogLevel");
        registerKeyword("l");
        //commands to set the log level for console appenders
        registerKeyword("setLogLevelConsole");
        registerKeyword("lc");
        //commands to set the log level for dlt appenders
        registerKeyword("setLogLevelDlt");
        registerKeyword("ld");

        description = "Commands to set the log level. Usage: setLogLevel <0..7>";
    }

    Bool RamshCommandSetLogLevel::executeInput(const RamshInput& input)
    {
        const String command = input.toString();

        //find out position of separator and split into command and log level
        const Int spacePosition = command.indexOf(' ');
        const String token = command.substr(0,spacePosition);
        const ELogLevel logLevel = RamsesLogger::GetLoglevelFromInt(static_cast<Int32>(atoi(command.substr(spacePosition+1,1).c_str())));

        //create log level string
        RamsesLogger& logger = GetRamsesLogger();
        const char* logLevelString = RamsesLogger::GetLogLevelText(logLevel);

        //apply new log levels for chosen appenders
        if(token==String("l") || token == String("setLogLevel"))
        {
            LOG_INFO(CONTEXT_RAMSH, "Logging all Contexts in level: " << logLevelString);
            LOG_INFO(CONTEXT_RAMSH, "Logging Console in level: " << logLevelString);
            LOG_INFO(CONTEXT_RAMSH, "Logging DLT in level: " << logLevelString);
            logger.setLogLevelForContexts(logLevel);
            logger.setLogLevelForAppenderType(ELogAppenderType::Console, logLevel);
            logger.setLogLevelForAppenderType(ELogAppenderType::Dlt, logLevel);
            return true;
        }

        if(token==String("lc") || token == String("setLogLevelConsole"))
        {
            LOG_INFO(CONTEXT_RAMSH, "Logging Console in level: " << logLevelString );
            logger.setLogLevelForAppenderType(ELogAppenderType::Console, logLevel);
            return true;
        }

        if(token==String("ld") || token == String("setLogLevelDlt"))
        {
            LOG_INFO(CONTEXT_RAMSH, "Logging DLT in level: " << logLevelString);
            logger.setLogLevelForAppenderType(ELogAppenderType::Dlt, logLevel);
            return true;
        }

        return false;
    }
}
