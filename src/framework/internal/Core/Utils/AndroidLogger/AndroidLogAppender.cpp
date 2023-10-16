//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/AndroidLogger/AndroidLogAppender.h"
#include "internal/Core/Utils/LogMessage.h"
#include "internal/Core/Utils/LogContext.h"
#include "internal/Core/Utils/RamsesLogger.h"
#include "internal/Core/Utils/InplaceStringTokenizer.h"
#include <string>

#include <android/log.h>

namespace ramses::internal
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

        constexpr size_t maxLogSize = 1023;
        const std::string& str = logMessage.getStream().data();
        if (str.size() <= maxLogSize)
            __android_log_write(logLevel, logMessage.getContext().getContextName(), str.c_str());
        else
        {
            // create modifyable copy of msg
            std::string modStr = str;
            InplaceStringTokenizer::TokenizeToMultilineCStrings(modStr, maxLogSize, '\n',
                [&](const char* tok) {
                    __android_log_write(logLevel, logMessage.getContext().getContextName(), tok);
                });
        }
    }
}
