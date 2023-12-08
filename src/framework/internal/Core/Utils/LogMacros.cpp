//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    LogContext& CONTEXT_FRAMEWORK               = GetRamsesLogger().createContext("ramses.Framework     ", "RFRA");
    LogContext& CONTEXT_CLIENT                  = GetRamsesLogger().createContext("ramses.Client        ", "RCLI");
    LogContext& CONTEXT_RENDERER                = GetRamsesLogger().createContext("ramses.Renderer      ", "RRND");
    LogContext& CONTEXT_PERIODIC                = GetRamsesLogger().createContext("ramses.Periodic      ", "RPER");
    LogContext& CONTEXT_TEXT                    = GetRamsesLogger().createContext("ramses.Text          ", "RTXT");

    LogContext& CONTEXT_COMMUNICATION           = GetRamsesLogger().createContext("ramses.Communication ", "RCOM");
    LogContext& CONTEXT_PROFILING               = GetRamsesLogger().createContext("ramses.Profiling     ", "RPRO");

    LogContext& CONTEXT_HLAPI_CLIENT            = GetRamsesLogger().createContext("ramses.HLAPI.Client  ", "RAPI");
    LogContext& CONTEXT_HLAPI_RENDERER          = GetRamsesLogger().createContext("ramses.HLAPI.Renderer", "RAPR");

    LogContext& CONTEXT_RAMSH                   = GetRamsesLogger().createContext("ramses.Ramsh         ", "RMSH");
    LogContext& CONTEXT_SMOKETEST               = GetRamsesLogger().createContext("ramses.SmokeTest     ", "RSMT");
}
