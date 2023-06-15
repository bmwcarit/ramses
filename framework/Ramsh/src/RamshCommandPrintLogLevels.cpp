//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "Ramsh/RamshCommandPrintLogLevels.h"
#include "Ramsh/Ramsh.h"
#include "Utils/LogMacros.h"
#include "Utils/RamsesLogger.h"

namespace ramses_internal
{
    RamshCommandPrintLogLevels::RamshCommandPrintLogLevels(const Ramsh& ramsh) : m_ramsh(ramsh)
    {
        registerKeyword("printLogLevels");
        description = "print Log Levels";
    }

    bool RamshCommandPrintLogLevels::executeInput(const std::vector<std::string>& input)
    {
        UNUSED(input);

        const RamsesLogger& logger = GetRamsesLogger();
        const ELogLevel consoleLogLevel = logger.getConsoleLogLevel();

        LOG_INFO(CONTEXT_RAMSH,"");
        LOG_INFO(CONTEXT_RAMSH,"           Appender Log Levels          ");
        LOG_INFO(CONTEXT_RAMSH,"----------------------------------------");
        LOG_INFO(CONTEXT_RAMSH,"Console | " << static_cast<int32_t>(consoleLogLevel) << " | " << RamsesLogger::GetLogLevelText(consoleLogLevel));
        LOG_INFO(CONTEXT_RAMSH,"");
        LOG_INFO(CONTEXT_RAMSH,"           Context Log Levels           ");
        LOG_INFO(CONTEXT_RAMSH,"ID   | Name                  | V | Level");
        LOG_INFO(CONTEXT_RAMSH,"----------------------------------------");

        for (const auto& info : GetRamsesLogger().getAllContextsInformation())
        {
            LOG_INFO(CONTEXT_RAMSH, info.id << " | " << info.name
                        << " | "
                        << static_cast<int32_t>(info.logLevel)
                        << " | "
                        << RamsesLogger::GetLogLevelText(info.logLevel));
        }

        return true;
    }

}
