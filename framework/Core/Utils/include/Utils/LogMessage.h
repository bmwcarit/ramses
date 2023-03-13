//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGMESSAGE_H
#define RAMSES_LOGMESSAGE_H

#include "Utils/LogLevel.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    class LogContext;

    class LogMessage
    {
    public:
        LogMessage(const LogContext& context, ELogLevel logLevel, const StringOutputStream& stream);

        [[nodiscard]] const StringOutputStream& getStream() const;
        [[nodiscard]] const LogContext& getContext() const;
        [[nodiscard]] ELogLevel getLogLevel() const;

    private:
        const LogContext& m_context;
        const ELogLevel m_logLevel;
        const StringOutputStream& m_outputStream;
    };

    inline LogMessage::LogMessage(const LogContext& context, ELogLevel logLevel, const StringOutputStream& stream)
        : m_context(context)
        , m_logLevel(logLevel)
        , m_outputStream(stream)
    {
    }

    inline const StringOutputStream& LogMessage::getStream() const
    {
        return m_outputStream;
    }

    inline const LogContext& LogMessage::getContext() const
    {
        return m_context;
    }

    inline ELogLevel LogMessage::getLogLevel() const
    {
        return m_logLevel;
    }
}

#endif
