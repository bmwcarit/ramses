//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/Logger.h"
#include "impl/LoggerImpl.h"

namespace rlogic::Logger
{
    void SetLogVerbosityLimit(ELogMessageType verbosityLimit)
    {
        internal::LoggerImpl::GetInstance().setLogVerbosityLimit(verbosityLimit);
    }

    ELogMessageType GetLogVerbosityLimit()
    {
        return internal::LoggerImpl::GetInstance().getLogVerbosityLimit();
    }

    void SetLogHandler(const LogHandlerFunc& logHandlerFunc)
    {
        internal::LoggerImpl::GetInstance().setLogHandler(logHandlerFunc);
    }

    void SetDefaultLogging(bool loggingEnabled)
    {
        internal::LoggerImpl::GetInstance().setDefaultLogging(loggingEnabled);
    }
}
