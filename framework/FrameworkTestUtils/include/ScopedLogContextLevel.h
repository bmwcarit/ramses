//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORKTESTUTILS_SCOPEDLOGCONTEXTLEVEL_H
#define RAMSES_FRAMEWORKTESTUTILS_SCOPEDLOGCONTEXTLEVEL_H

#include "Utils/LogContext.h"

namespace ramses_internal
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

        ~ScopedLogContextLevel()
        {
            m_context.setLogLevel(m_savedLogLevel);
        }

    private:
        LogContext& m_context;
        ELogLevel m_savedLogLevel;
    };
}

#endif
