//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/UserLogAppender.h"
#include "Utils/LogMessage.h"
#include "Utils/LogContext.h"

namespace
{
    ramses::ELogLevel GetELogLevel(ramses_internal::ELogLevel logLevel)
    {
        switch (logLevel)
        {
        case ramses_internal::ELogLevel::Off:
            return ramses::ELogLevel::Off;
        case ramses_internal::ELogLevel::Fatal:
            return ramses::ELogLevel::Fatal;
        case ramses_internal::ELogLevel::Error:
            return ramses::ELogLevel::Error;
        case ramses_internal::ELogLevel::Warn:
            return ramses::ELogLevel::Warn;
        case ramses_internal::ELogLevel::Info:
            return ramses::ELogLevel::Info;
        case ramses_internal::ELogLevel::Debug:
            return ramses::ELogLevel::Debug;
        case ramses_internal::ELogLevel::Trace:
            return ramses::ELogLevel::Trace;
        }

        return ramses::ELogLevel::Off;
    }
}

namespace ramses_internal
{
    UserLogAppender::UserLogAppender(const ramses::LogHandlerFunc& f)
        : m_func(f)
    {
    }

    void UserLogAppender::log(const LogMessage& logMessage)
    {
        m_func(GetELogLevel(logMessage.getLogLevel()), logMessage.getContext().getContextId(), logMessage.getStream().data());
    }
}
