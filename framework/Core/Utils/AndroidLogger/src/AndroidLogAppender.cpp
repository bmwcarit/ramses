//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AndroidLogger/AndroidLogAppender.h"
#include "Utils/LogMessage.h"
#include "Utils/LogContext.h"
#include "Utils/RamsesLogger.h"

#include <android/log.h>

namespace ramses_internal
{
    void AndroidLogAppender::log(const LogMessage& logMessage)
    {
        android_LogPriority logLevel;

        switch(logMessage.getLogLevel())
        {
        case ELogLevel::Trace:
            logLevel = ANDROID_LOG_VERBOSE;
            break;
        case ELogLevel::Debug :
            logLevel = ANDROID_LOG_DEBUG;
            break;
        case ELogLevel::Info :
            logLevel = ANDROID_LOG_INFO;
            break;
        case ELogLevel::Warn :
            logLevel = ANDROID_LOG_WARN;
            break;
        case ELogLevel::Error :
            logLevel = ANDROID_LOG_ERROR;
            break;
        case ELogLevel::Fatal :
            logLevel = ANDROID_LOG_FATAL;
            break;
        default:
            logLevel = ANDROID_LOG_UNKNOWN;
            break;
        }

        __android_log_print(logLevel, logMessage.getContext().getContextName(), "%s", logMessage.getStream().c_str());
    }
}
