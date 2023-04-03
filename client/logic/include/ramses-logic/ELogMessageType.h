//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cassert>

namespace rlogic
{
    /**
     * ELogMessageType lists the types of available log messages. The integer represents the
     * priority of the log messages (lower means more important, thus higher priority).
     */
    enum class ELogMessageType : int
    {
        Off = 0,    ///< No logs shall be issues, no matter the severity
        Fatal = 1,  ///< Log only fatal errors
        Error = 2,  ///< Log all errors
        Warn = 3,   ///< Log warnings + errors
        Info = 4,   ///< Include general info logs in addition to warn + errors
        Debug = 5,  ///< Debug logs - use this only for debugging
        Trace = 6   ///< Verbose trace logs - use only for debugging and inspection
    };
}
