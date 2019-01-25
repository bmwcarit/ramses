//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDENVIRONMENTUTILS_H
#define RAMSES_WAYLANDENVIRONMENTUTILS_H

#include "Collections/String.h"

namespace ramses_internal
{
    enum class WaylandEnvironmentVariable : UInt8
    {
        XDGRuntimeDir = 0,
        WaylandSocket,
        WaylandDisplay,
        NUMBER_OF_ELEMENTS
    };


    namespace WaylandEnvironmentUtils
    {
        void    SetVariable(WaylandEnvironmentVariable variableName, const String& value);
        void    UnsetVariable(WaylandEnvironmentVariable variableName);
        String  GetVariable(WaylandEnvironmentVariable variableName);
        bool    IsEnvironmentInProperState();
    };
}

#endif
