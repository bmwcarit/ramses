//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RecordingLogAppender.h"
#include "Collections/StringOutputStream.h"
#include "Utils/RamsesLogger.h"
#include "Utils/LogMessage.h"

namespace ramses_internal
{
    RecordingLogAppender::RecordingLogAppender(ELogLevel logLevel)
    {
        setLogLevel(logLevel);
    }

    void RecordingLogAppender::reset()
    {
        m_messages.clear();
    }

    void RecordingLogAppender::logMessage(const LogMessage& logMessage)
    {
        if (logMessage.getLogLevel() >= m_logLevel)
        {
            m_messages.push_back(logMessage);
        }
    }

    Bool RecordingLogAppender::hasMessages() const
    {
        return m_messages.size() > 0;
    }

    StringVector RecordingLogAppender::getFormattedMessages() const
    {
        StringVector result;
        for (const auto& msg : m_messages)
        {
            result.push_back(formatMessage(msg));
        }
        return result;
    }

    const std::vector<LogMessage>& RecordingLogAppender::getLogMessages() const
    {
        return m_messages;
    }

    String RecordingLogAppender::getConcatenatedMessages() const
    {
        String result;
        for (const auto& msg : m_messages)
        {
            result += formatMessage(msg) + "\n";
        }
        return result;
    }

    String RecordingLogAppender::formatMessage(const LogMessage& msg) const
    {
        StringOutputStream stream;
        stream << "[" << RamsesLogger::GetLogLevelText(msg.getLogLevel()) << "] ";
        stream << msg.getContext().getContextName() << ": " << msg.getStream().c_str();
        return stream.release();
    }
}
