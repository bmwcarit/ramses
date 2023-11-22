//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/WaylandEnvironmentUtils.h"
#include "internal/Platform/Wayland/UnixDomainSocket.h"
#include "internal/PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "internal/Core/Utils/File.h"
#include "internal/Core/Utils/LoggingUtils.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    namespace
    {
        const std::array EnvironmentVariableNames =
        {
            "XDG_RUNTIME_DIR",
            "WAYLAND_SOCKET",
            "WAYLAND_DISPLAY"
        };

        ENUM_TO_STRING(WaylandEnvironmentVariable, EnvironmentVariableNames, WaylandEnvironmentVariable::WaylandDisplay);

        bool CheckXDGRuntimeDir()
        {
            std::string xdgPathEnvironmentVar;
            const bool xdgPathFoundInEnvironmentVars = PlatformEnvironmentVariables::get("XDG_RUNTIME_DIR", xdgPathEnvironmentVar);

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

        void CheckSocketFileExists(const std::string& xdgRuntimeDir, const std::string& socketFilename)
        {
            const bool socketIsAbsolute = (!socketFilename.empty()) && socketFilename[0] == '/';
            const auto socketFullPath = (socketIsAbsolute ? socketFilename : (xdgRuntimeDir + "/" + socketFilename));

            if (!socketIsAbsolute)
            {
                if(!CheckXDGRuntimeDir())
                    return;
            }

            const File socketFile(socketFullPath);
            if (socketFile.exists())
            {
                LOG_INFO(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: Socket file {} exists.", socketFullPath);
            }
            else
            {
                LOG_WARN(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: Socket file {} does not exist.", socketFullPath);
            }
        }

        bool CheckSocketFileDescritorExists(const std::string& socketFD)
        {
            if (UnixDomainSocket::IsFileDescriptorForValidSocket(static_cast<int>(strtol(socketFD.c_str(), nullptr, 0))))
            {
                LOG_INFO(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: Environment variable WAYLAND_SOCKET contains a valid socket file descriptor.");
            }
            else
            {
                LOG_WARN(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: Environment variable WAYLAND_SOCKET does not contain a valid socket file descriptor.");
            }

            return true;
        }
    }

    namespace WaylandEnvironmentUtils
    {
        void SetVariable(WaylandEnvironmentVariable variableName, const std::string& value)
        {
            PlatformEnvironmentVariables::SetEnvVar(EnumToString(variableName), value);
        }

        void UnsetVariable(WaylandEnvironmentVariable variableName)
        {
            PlatformEnvironmentVariables::UnsetEnvVar(EnumToString(variableName));
        }

        std::string GetVariable(WaylandEnvironmentVariable variableName)
        {
            std::string variable;
            PlatformEnvironmentVariables::get(EnumToString(variableName), variable);
            return variable;
        }

        void LogEnvironmentState(const std::string& waylandDisplayName)
        {
            std::string xdgPathEnvironmentVar;
            std::string waylandDisplayEnvironmentVar;
            std::string waylandSocketEnvironmentVar;
            PlatformEnvironmentVariables::get("XDG_RUNTIME_DIR", xdgPathEnvironmentVar);
            const bool waylandDisplayFoundInEnvironmentVars = PlatformEnvironmentVariables::get("WAYLAND_DISPLAY", waylandDisplayEnvironmentVar);
            const bool waylandSocketFoundInEnvironmentVars  = PlatformEnvironmentVariables::get("WAYLAND_SOCKET",  waylandSocketEnvironmentVar);

            LOG_INFO(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: Wayland display set on display config={}, XDG_RUNTIME_DIR={}, WAYLAND_DISPLAY={}, WAYLAND_SOCKET={}",
                waylandDisplayName, xdgPathEnvironmentVar, waylandDisplayEnvironmentVar, waylandSocketEnvironmentVar);

            if (!waylandDisplayName.empty())
            {
                CheckSocketFileExists(xdgPathEnvironmentVar, waylandDisplayName);
            }
            else if (waylandSocketFoundInEnvironmentVars && waylandDisplayFoundInEnvironmentVars)
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandEnvironmentUtils::LogEnvironmentState: Environment variables WAYLAND_DISPLAY and WAYLAND_SOCKET are both set.");
            }
            else if (waylandSocketFoundInEnvironmentVars)
            {
                CheckSocketFileDescritorExists(waylandSocketEnvironmentVar);
            }
            else if (waylandDisplayFoundInEnvironmentVars)
            {
                CheckSocketFileExists(xdgPathEnvironmentVar, waylandDisplayEnvironmentVar);
            }
            else
            {
                CheckXDGRuntimeDir();
            }
        }
    }
}
