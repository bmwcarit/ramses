//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGAPPENDERBASE_H
#define RAMSES_LOGAPPENDERBASE_H

#include "Utils/LogLevel.h"
#include <atomic>

namespace ramses_internal
{
    class LogMessage;

    class LogAppenderBase
    {
    public:
        LogAppenderBase();
        virtual ~LogAppenderBase() {}

        virtual void logMessage(const LogMessage& logMessage) = 0;
        void log(const LogMessage& logMessage);

        ELogLevel getLogLevel() const;
        void setLogLevel(ELogLevel level);

    protected:
        std::atomic<ELogLevel> m_logLevel;
    };

    inline LogAppenderBase::LogAppenderBase()
        : m_logLevel(ELogLevel::Info)
    {
    }

    inline ELogLevel LogAppenderBase::getLogLevel() const
    {
        return m_logLevel;
    }

    inline void LogAppenderBase::setLogLevel(ELogLevel level)
    {
        m_logLevel = level;
    }
}

#endif
