//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDENVIRONMENTUTILS_H
#define RAMSES_WAYLANDENVIRONMENTUTILS_H

#include <PlatformAbstraction/PlatformTypes.h>

#include <string>

namespace ramses_internal
{
    enum class WaylandEnvironmentVariable : uint8_t
    {
        XDGRuntimeDir = 0,
        WaylandSocket,
        WaylandDisplay,
        NUMBER_OF_ELEMENTS
    };


    namespace WaylandEnvironmentUtils
    {
        void         SetVariable(WaylandEnvironmentVariable variableName, const std::string& value);
        void         UnsetVariable(WaylandEnvironmentVariable variableName);
        std::string  GetVariable(WaylandEnvironmentVariable variableName);
        void         LogEnvironmentState(const std::string& waylandDisplayName);
    };
}

#endif
