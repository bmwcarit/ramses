//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>

namespace ramses::internal
{
    enum class WaylandEnvironmentVariable : uint8_t
    {
        XDGRuntimeDir = 0,
        WaylandSocket,
        WaylandDisplay,
    };


    namespace WaylandEnvironmentUtils
    {
        void         SetVariable(WaylandEnvironmentVariable variableName, const std::string& value);
        void         UnsetVariable(WaylandEnvironmentVariable variableName);
        std::string  GetVariable(WaylandEnvironmentVariable variableName);
        void         LogEnvironmentState(const std::string& waylandDisplayName);
    };
}
