//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGCONTEXT_H
#define RAMSES_LOGCONTEXT_H

#include "Utils/LogLevel.h"
#include <atomic>

namespace ramses_internal
{
    class LogContext
    {
    public:
        LogContext(const char* name, const char* id);

        const char* getContextName() const;
        const char* getContextId() const;

        void setEnabled(bool enabled);
        bool isEnabled() const;

        void setLogLevel(const ELogLevel logLevel);
        ELogLevel getLogLevel() const;

        void setUserData(void* dataPtr);
        void* getUserData() const;

    private:
        const char* m_contextName;
        const char* m_contextId;

        std::atomic<ELogLevel> m_logLevel;

        void* m_data;
    };

    inline LogContext::LogContext(const char* name, const char* id)
        : m_contextName(name)
        , m_contextId(id)
        , m_logLevel(ELogLevel::Info)
        , m_data(nullptr)
    {
    }

    inline void LogContext::setLogLevel(ELogLevel logLevel)
    {
        m_logLevel = logLevel;
    }

    inline ELogLevel LogContext::getLogLevel() const
    {
        return m_logLevel;
    }

    inline const char* LogContext::getContextName() const
    {
        return m_contextName;
    }

    inline const char* LogContext::getContextId() const
    {
        return m_contextId;
    }

    inline void LogContext::setUserData(void* dataPtr)
    {
        m_data = dataPtr;
    }

    inline void* LogContext::getUserData() const
    {
        return m_data;
    }
}

#endif
