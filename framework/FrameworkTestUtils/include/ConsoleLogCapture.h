//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CONSOLELOGCAPTURE_H
#define RAMSES_CONSOLELOGCAPTURE_H

#include "RecordingLogAppender.h"
#include "Utils/RamsesLogger.h"

namespace ramses_internal
{
    class ConsoleLogCapture
    {
    public:
        inline ConsoleLogCapture(ELogLevel logLevel = ELogLevel::Warn);
        inline ~ConsoleLogCapture();

        inline void reset();

        inline Bool hasMessages() const;

    private:
        RecordingLogAppender m_appender;
    };

    ConsoleLogCapture::ConsoleLogCapture(ELogLevel logLevel)
        : m_appender(logLevel)
    {
        GetRamsesLogger().addAppender(m_appender);
    }

    ConsoleLogCapture::~ConsoleLogCapture()
    {
        GetRamsesLogger().removeAppender(m_appender);
    }

    void ConsoleLogCapture::reset()
    {
        m_appender.reset();
    }

    Bool ConsoleLogCapture::hasMessages() const
    {
        return m_appender.hasMessages();
    }
}

#endif
