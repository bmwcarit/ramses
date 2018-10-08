//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RECORDINGLOGAPPENDER_H
#define RAMSES_RECORDINGLOGAPPENDER_H

#include "Utils/LogAppenderBase.h"
#include "Utils/StringUtils.h"
#include "Utils/LogLevel.h"

namespace ramses_internal
{
    class RecordingLogAppender : public LogAppenderBase
    {
    public:
        explicit RecordingLogAppender(ELogLevel logLevel);

        void reset();

        Bool hasMessages() const;
        StringVector getFormattedMessages() const;
        const Vector<LogMessage>& getLogMessages() const;

        String getConcatenatedMessages() const;

    private:
        virtual void logMessage(const LogMessage& message);
        String formatMessage(const LogMessage& msg) const;

        Vector<LogMessage> m_messages;
    };
}

#endif
