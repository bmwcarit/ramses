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
        const char* const EnvironmentVariableNames[] =
        {
            "XDG_RUNTIME_DIR",
            "WAYLAND_SOCKET",
            "WAYLAND_DISPLAY"
        };

        ENUM_TO_STRING(WaylandEnvironmentVariable, EnvironmentVariableNames, WaylandEnvironmentVariable::NUMBER_OF_ELEMENTS);

        bool CheckXDGRuntimeDir()
        {
            String xdgPathEnvironmentVar;
            const Bool xdgPathFoundInEnvironmentVars = PlatformEnvironmentVariables::get("XDG_RUNTIME_DIR", xdgPathEnvironmentVar);

            if (!xdgPathFoundInEnvironmentVars)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: XDG_RUNTIME_DIR environment variable not set.");
                return false;
            }

            const File xdgDir(xdgPathEnvironmentVar);
            if (!xdgDir.isDirectory())
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: XDG_RUNTIME_DIR does not point to a valid directory.");
                return false;
            }

            LOG_INFO(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: XDG_RUNTIME_DIR is set correctly.");
            return true;
        }

        void CheckSocketFileExists(const String& xdgRuntimeDir, const String& socketFilename)
        {
            const bool socketIsAbsolute = (!socketFilename.empty()) && socketFilename[0] == '/';
            const auto socketFullPath = (socketIsAbsolute ? socketFilename : (xdgRuntimeDir + "/" + socketFilename));

            if(!socketIsAbsolute)
                if(!CheckXDGRuntimeDir())
                    return;

            const File socketFile(socketFullPath);
            if (socketFile.exists())
                LOG_INFO(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: Socket file " << socketFullPath << " exists.");
            else
                LOG_WARN(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: Socket file " << socketFullPath << " does not exist.");
        }

        bool CheckSocketFileDescritorExists(const String& socketFD)
        {
            if (UnixDomainSocket::IsFileDescriptorForValidSocket(atoi(socketFD.c_str())))
                LOG_INFO(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: Environment variable WAYLAND_SOCKET contains a valid socket file descriptor.");
            else
                LOG_WARN(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: Environment variable WAYLAND_SOCKET does not contain a valid socket file descriptor.");

            return true;
        }
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

        void LogEnvironmentState(const String& waylandDisplayName)
        {
            String xdgPathEnvironmentVar;
            String waylandDisplayEnvironmentVar;
            String waylandSocketEnvironmentVar;
            PlatformEnvironmentVariables::get("XDG_RUNTIME_DIR", xdgPathEnvironmentVar);
            const Bool waylandDisplayFoundInEnvironmentVars = PlatformEnvironmentVariables::get("WAYLAND_DISPLAY", waylandDisplayEnvironmentVar);
            const Bool waylandSocketFoundInEnvironmentVars  = PlatformEnvironmentVariables::get("WAYLAND_SOCKET",  waylandSocketEnvironmentVar);

            LOG_INFO(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: Wayland display set on display config=" << waylandDisplayName
                      << ", XDG_RUNTIME_DIR=" << xdgPathEnvironmentVar
                      << ", WAYLAND_DISPLAY=" << waylandDisplayEnvironmentVar
                      << ", WAYLAND_SOCKET=" << waylandSocketEnvironmentVar);

            if(!waylandDisplayName.empty())
                CheckSocketFileExists(xdgPathEnvironmentVar, waylandDisplayName);
            else if (waylandSocketFoundInEnvironmentVars && waylandDisplayFoundInEnvironmentVars)
                LOG_ERROR(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: Environment variables WAYLAND_DISPLAY and WAYLAND_SOCKET are both set.");
            else if (waylandSocketFoundInEnvironmentVars)
                CheckSocketFileDescritorExists(waylandSocketEnvironmentVar);
            else if (waylandDisplayFoundInEnvironmentVars)
                CheckSocketFileExists(xdgPathEnvironmentVar, waylandDisplayEnvironmentVar);
            else
                CheckXDGRuntimeDir();
        }
    }
}
