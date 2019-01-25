//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WaylandUtilities/WaylandEnvironmentUtils.h"
#include "WaylandUtilities/UnixDomainSocket.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "Utils/File.h"
#include "Utils/LoggingUtils.h"
#include "Utils/LogMacros.h"

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

        bool IsEnvironmentInProperState()
        {
            String xdgPath;
            String waylandDisplay;
            String waylandSocket;
            const bool xdgPathFound = PlatformEnvironmentVariables::get("XDG_RUNTIME_DIR", xdgPath);
            const bool waylandDisplayFound = PlatformEnvironmentVariables::get("WAYLAND_DISPLAY", waylandDisplay);
            const bool waylandSocketFound  = PlatformEnvironmentVariables::get("WAYLAND_SOCKET",  waylandSocket);

            if (waylandSocketFound && waylandDisplayFound)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandEnvironmentUtils::IsEnvironmentInProperState Environment variables WAYLAND_DISPLAY and WAYLAND_SOCKET are both set.");
                return false;
            }

            if (waylandSocketFound)
            {
                if (UnixDomainSocket::IsFileDescriptorForValidSocket(atoi(waylandSocket.c_str())))
                {
                    return true;
                }
                else
                {
                    LOG_ERROR(CONTEXT_RENDERER, "WaylandEnvironmentUtils::IsEnvironmentInProperState Environment variable WAYLAND_SOCKET does not contain a valid socket file descriptor :" << waylandSocket);
                    return false;
                }
            }

            if (!xdgPathFound)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandEnvironmentUtils::IsEnvironmentInProperState XDG_RUNTIME_DIR environment variable not set.");
                return false;
            }

            const File xdgDir(xdgPath);
            if (!xdgDir.isDirectory())
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandEnvironmentUtils::IsEnvironmentInProperState XDG_RUNTIME_DIR does not point to a valid directory.");
                return false;
            }


            if (waylandDisplayFound)
            {
                const File socketFile(xdgPath + "/" + waylandDisplay);
                if (!socketFile.exists())
                {
                    LOG_ERROR(CONTEXT_RENDERER, "Socket file " << waylandDisplay << " referenced by environment variable WAYLAND_DISPLAY does not exist.");
                    return false;
                }
            }

            return true;
        }
    }
}
