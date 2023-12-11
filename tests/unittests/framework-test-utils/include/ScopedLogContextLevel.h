//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LogContext.h"
#include "impl/RamsesLoggerImpl.h"

namespace ramses::internal
{
    class ScopedLogContextLevel
    {
    public:
        ScopedLogContextLevel(LogContext& context, ELogLevel scopeLogLevel)
            : m_context(context)
            , m_savedLogLevel(m_context.getLogLevel())
        {
            m_context.setLogLevel(scopeLogLevel);
        }

        using LogHandler = std::function<void(ELogLevel, std::string_view)>;

        ScopedLogContextLevel(LogContext& context, ELogLevel scopeLogLevel, const LogHandler& handler)
            : ScopedLogContextLevel(context, scopeLogLevel)
        {
            m_handler = handler;
            m_unsetLogHandler = true;
            auto ramsesHandler = [&](ELogLevel level, std::string_view ctx, std::string_view msg) {
                if (ctx == m_context.getContextId())
                {
                    m_handler(level, msg);
                }
            };
            GetRamsesLogger().setLogHandler(ramsesHandler);
        }

        ~ScopedLogContextLevel()
        {
            m_context.setLogLevel(m_savedLogLevel);
            if (m_unsetLogHandler)
            {
                GetRamsesLogger().setLogHandler(LogHandlerFunc());
            }
        }

    private:
        LogContext& m_context;
        LogHandler  m_handler;
        ELogLevel m_savedLogLevel;
        bool m_unsetLogHandler = false;
    };
}
