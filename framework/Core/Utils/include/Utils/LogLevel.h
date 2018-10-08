//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LOGLEVEL_H
#define RAMSES_LOGLEVEL_H

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
}

#endif
