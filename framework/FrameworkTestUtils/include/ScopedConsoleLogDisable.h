//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEWORKTESTUTILS_SCOPEDCONSOLELOGDISABLE_H
#define RAMSES_FRAMEWORKTESTUTILS_SCOPEDCONSOLELOGDISABLE_H

#include "Utils/RamsesLogger.h"

namespace ramses_internal
{
    class ScopedConsoleLogDisable
    {
    public:
        ScopedConsoleLogDisable()
            : m_logLevel(GetRamsesLogger().getConsoleLogLevel())
        {
            GetRamsesLogger().setConsoleLogLevel(ELogLevel::Off);
        }

        ~ScopedConsoleLogDisable()
        {
            GetRamsesLogger().setConsoleLogLevel(m_logLevel);
        }

    private:
        ELogLevel m_logLevel;
    };
}

#endif
