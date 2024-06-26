//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LogLevel.h"
#include <atomic>

namespace ramses::internal
{
    class LogContext
    {
    public:
        LogContext(const char* name, const char* id);

        [[nodiscard]] const char* getContextName() const;
        [[nodiscard]] const char* getContextId() const;

        void setLogLevel(const ELogLevel logLevel);
        [[nodiscard]] ELogLevel getLogLevel() const;

        void setUserData(void* dataPtr);
        [[nodiscard]] void* getUserData() const;
    private:
        const char* m_contextName;
        const char* m_contextId;

        std::atomic<ELogLevel> m_logLevel{ELogLevel::Info};

        void* m_data = nullptr;
    };

    inline LogContext::LogContext(const char* name, const char* id)
        : m_contextName(name)
        , m_contextId(id)
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
