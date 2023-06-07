//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGLEVEL_H
#define RAMSES_LOGLEVEL_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses_internal
{
    enum class ELogLevel
    {
        // the integer value of these may not be changed
        Off = 0,
        Fatal = 1,
        Error = 2,
        Warn = 3,
        Info = 4,
        Debug = 5,
        Trace = 6
    };

    inline ELogLevel GetELogLevelInternal(ramses::ELogLevel logLevel)
    {
        switch (logLevel)
        {
        case ramses::ELogLevel::Off:
            return ramses_internal::ELogLevel::Off;
        case ramses::ELogLevel::Fatal:
            return ramses_internal::ELogLevel::Fatal;
        case ramses::ELogLevel::Error:
            return ramses_internal::ELogLevel::Error;
        case ramses::ELogLevel::Warn:
            return ramses_internal::ELogLevel::Warn;
        case ramses::ELogLevel::Info:
            return ramses_internal::ELogLevel::Info;
        case ramses::ELogLevel::Debug:
            return ramses_internal::ELogLevel::Debug;
        case ramses::ELogLevel::Trace:
            return ramses_internal::ELogLevel::Trace;
        }

        return ramses_internal::ELogLevel::Off;
    }
}

#endif
