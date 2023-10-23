//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/RamsesLogger.h"

namespace ramses::internal
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
