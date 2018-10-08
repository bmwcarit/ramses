//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "WaylandUtilities/WaylandUtilities.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "Collections/String.h"
#include "Utils/LogMacros.h"
#include "Utils/File.h"
#include "wayland-server.h"
#include "wayland-version.h"

namespace ramses_internal
{
    Bool WaylandUtilities::IsValidSocket(int fileDescriptor)
    {
        if (fileDescriptor < 0)
        {
            return false;
        }

        struct stat buf;
        if (fstat(fileDescriptor, &buf) < 0)
        {
            return false;
        }

        return S_ISSOCK(buf.st_mode);
    }

    Bool WaylandUtilities::IsEnvironmentInProperState()
    {
        String xdgPath;
        String waylandDisplay;
        String waylandSocket;
        const Bool xdgPathFound = PlatformEnvironmentVariables::get("XDG_RUNTIME_DIR", xdgPath);
        const Bool waylandDisplayFound = PlatformEnvironmentVariables::get("WAYLAND_DISPLAY", waylandDisplay);
        const Bool waylandSocketFound  = PlatformEnvironmentVariables::get("WAYLAND_SOCKET",  waylandSocket);

        if (waylandSocketFound && waylandDisplayFound)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandUtilities::IsEnvironmentInProperState Environment variables WAYLAND_DISPLAY and WAYLAND_SOCKET are both set.");
            return false;
        }

        if (waylandSocketFound)
        {
            if (IsValidSocket(atoi(waylandSocket.c_str())))
            {
                return true;
            }
            else
            {
                LOG_ERROR(CONTEXT_RENDERER, "WaylandUtilities::IsEnvironmentInProperState Environment variable WAYLAND_SOCKET does not contain a valid socket file descriptor.");
                return false;
            }
        }

        if (!xdgPathFound)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandUtilities::IsEnvironmentInProperState XDG_RUNTIME_DIR environment variable not set.");
            return false;
        }

        const File xdgDir(xdgPath);
        if (!xdgDir.isDirectory())
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandUtilities::IsEnvironmentInProperState XDG_RUNTIME_DIR does not point to a valid directory.");
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

    Bool WaylandUtilities::DoesWaylandSupportDisplayAddSockedFD()
    {
#if (WAYLAND_VERSION_MAJOR >= 1 && WAYLAND_VERSION_MINOR >= 13)
        return true;
#else
        return false;
#endif
    }

    int WaylandUtilities::DisplayAddSocketFD(wl_display* display, int sock_fd)
    {
#if (WAYLAND_VERSION_MAJOR >= 1 && WAYLAND_VERSION_MINOR >= 13)
        return wl_display_add_socket_fd(display, sock_fd);
#else
        UNUSED(display)
        UNUSED(sock_fd)
        LOG_ERROR(CONTEXT_RENDERER,
                  "WaylandUtilities::DisplayAddSocketFD - Wayland version is less than 1.13, "
                  "wl_display_add_socket_fd not supported!")
        return -1;
#endif
    }
}
