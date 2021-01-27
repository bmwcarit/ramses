//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/LogHelper.h"
#include "Utils/LogMacros.h"
#include "Utils/StringUtils.h"


namespace ramses_internal
{
    namespace LogHelper
    {
        bool StringToLogLevel(ramses_internal::String str, ELogLevel& logLevel)
        {
            str.toLowerCase();
            // Trim string because envvar on windows may have space(s) appended
            str = StringUtils::Trim(str.c_str());
            if (str == ramses_internal::String("trace") || str == ramses_internal::String("6"))
            {
                logLevel = ELogLevel::Trace;
                return true;
            }
            else if (str == ramses_internal::String("debug") || str == ramses_internal::String("5"))
            {
                logLevel = ELogLevel::Debug;
                return true;
            }
            else if (str == ramses_internal::String("info") || str == ramses_internal::String("4"))
            {
                logLevel = ELogLevel::Info;
                return true;
            }
            else if (str == ramses_internal::String("warn") || str == ramses_internal::String("3"))
            {
                logLevel = ELogLevel::Warn;
                return true;
            }
            else if (str == ramses_internal::String("error") || str == ramses_internal::String("2"))
            {
                logLevel = ELogLevel::Error;
                return true;
            }
            else if (str == ramses_internal::String("fatal") || str == ramses_internal::String("1"))
            {
                logLevel = ELogLevel::Fatal;
                return true;
            }
            else if (str == ramses_internal::String("off") || str == ramses_internal::String("0"))
            {
                logLevel = ELogLevel::Off;
                return true;
            }
            return false;
        }

        std::vector<ContextFilter> ParseContextFilters(const String& filterCommand)
        {
            std::vector<ContextFilter> returnValue;
            // loop over commands separated by ','
            UInt currentCommandStart = 0;
            do
            {
                Int currentCommandEnd = filterCommand.find(',', currentCommandStart);
                if (currentCommandEnd <= 0)
                {
                    // no more ',', so command goes until end of string
                    currentCommandEnd = static_cast<int32_t>(filterCommand.size());
                }
                const Int positionOfColon = filterCommand.find(':', currentCommandStart);
                const Int lengthOfLogLevelString = positionOfColon - currentCommandStart;
                if (lengthOfLogLevelString > 0)
                {
                    const String logLevelStr = filterCommand.substr(currentCommandStart, lengthOfLogLevelString);
                    ELogLevel logLevel;
                    if (StringToLogLevel(logLevelStr, logLevel))
                    {
                        const Int offsetOfLogContextPattern = lengthOfLogLevelString + 1;
                        const String contextPattern = filterCommand.substr(currentCommandStart + offsetOfLogContextPattern, currentCommandEnd - currentCommandStart - offsetOfLogContextPattern);
                        if (contextPattern.size() > 0)
                        {
                            returnValue.push_back(ContextFilter(logLevel, contextPattern));
                        }
                    }
                    else
                    {
                        LOG_WARN(CONTEXT_FRAMEWORK, "LogHelper::ParseContextFilters: Skip unknown log level '" << logLevelStr << "'");
                    }
                }
                currentCommandStart = currentCommandEnd + 1;
            } while (currentCommandStart <= filterCommand.size());
            return returnValue;
        }

        ELogLevel GetLoglevelFromInt(Int32 logLevelInt)
        {
            switch (logLevelInt)
            {
            case static_cast<Int32>(ELogLevel::Off) : return ELogLevel::Off;
            case static_cast<Int32>(ELogLevel::Fatal) : return ELogLevel::Fatal;
            case static_cast<Int32>(ELogLevel::Error) : return ELogLevel::Error;
            case static_cast<Int32>(ELogLevel::Warn) : return ELogLevel::Warn;
            case static_cast<Int32>(ELogLevel::Info) : return ELogLevel::Info;
            case static_cast<Int32>(ELogLevel::Debug) : return ELogLevel::Debug;
            case static_cast<Int32>(ELogLevel::Trace) : return ELogLevel::Trace;
            default:                                   return ELogLevel::Trace;
            }
        }
    }
}
