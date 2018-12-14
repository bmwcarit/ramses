//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WaylandUtilities/WaylandEnvironmentUtils.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    namespace
    {
        const char* EnvironmentVariableNames[] =
        {
            "XDG_RUNTIME_DIR",
            "WAYLAND_SOCKET",
            "WAYLAND_DISPLAY"
        };

        ENUM_TO_STRING(WaylandEnvironmentVariable, EnvironmentVariableNames, WaylandEnvironmentVariable::NUMBER_OF_ELEMENTS);
    }

    namespace WaylandEnvironmentUtils
    {

        void SetVariable(WaylandEnvironmentVariable variableName, const String& value)
        {
            PlatformEnvironmentVariables::SetEnvVar(EnumToString(variableName), value);
        }

        void UnsetVariable(WaylandEnvironmentVariable variableName)
        {
            PlatformEnvironmentVariables::UnsetEnvVar(EnumToString(variableName));
        }

        String GetVariable(WaylandEnvironmentVariable variableName)
        {
            String variable;
            PlatformEnvironmentVariables::get(EnumToString(variableName), variable);
            return variable;
        }

    }
}
