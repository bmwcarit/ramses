//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandDisplay.h"
#include "EmbeddedCompositor_Wayland/WaylandCompositorConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandGlobal.h"
#include "PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "WaylandUtilities/WaylandUtilities.h"
#include "Utils/LogMacros.h"
#include <grp.h>
#include <errno.h>
#include <unistd.h>

namespace ramses_internal
{
    WaylandDisplay::WaylandDisplay()
    {
    }

    bool WaylandDisplay::init(const String& socketName,  const String& socketGroupName, int socketFD)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandDisplay::init");

        m_display = wl_display_create();
        if (nullptr == m_display)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandDisplay::init Failed to create wayland display!");
            return false;
        }

        if (!addSocketToDisplay(socketName, socketGroupName, socketFD))
        {
            return false;
        }

        wl_display_init_shm(m_display);

        return true;
    }

    WaylandDisplay::~WaylandDisplay()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandDisplay::~WaylandDisplay");

        if (nullptr != m_display)
        {
            wl_display_destroy(m_display);
        }
    }

    bool WaylandDisplay::addSocketToDisplay(const String& socketName, const String& socketGroupName, int socketFD)
    {
        const Bool socketNameProvided = socketName.getLength() > 0u;
        const Bool socketFDProvided   = socketFD >= 0;

        if (socketFDProvided && !socketNameProvided)
        {
            return addSocketToDisplayWithFD(socketFD);
        }
        else if (socketNameProvided && !socketFDProvided)
        {
            return addSocketToDisplayWithName(socketName, socketGroupName);
        }
        else if (socketNameProvided && socketFDProvided)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "WaylandDisplay::addSocketToDisplay(): Failed to add wayland display because "
                      "contradicting configuration paramters are set (both socket file descriptor and socket name are "
                      "set)!");
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "WaylandDisplay::addSocketToDisplay(): Failed to add wayland display because neither "
                      "socket file descriptor nor socket name is set!");
        }

        return false;
    }

    bool WaylandDisplay::addSocketToDisplayWithFD(int socketFD)
    {
        if (WaylandUtilities::DisplayAddSocketFD(m_display, socketFD) >= 0)
        {
            LOG_DEBUG(CONTEXT_RENDERER,
                      "WaylandDisplay::addSocketToDisplayWithFD(): Added wayland display on embedded compositor socket "
                      "provided by "
                      "systemd");
            return true;
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "WaylandDisplay::addSocketToDisplayWithFD(): Failed to add wayland display on embedded compositor socket "
                      "provided by RendererConfig::setWaylandSocketEmbeddedFD()");
            return false;
        }

    }

    bool WaylandDisplay::addSocketToDisplayWithName(const String& socketName, const String& socketGroupName)
    {
        if (wl_display_add_socket(m_display, socketName.c_str()) < 0)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "WaylandDisplay::addSocketToDisplayWithName(): Failed to add wayland display on embedded "
                      "compositor socket :"
                          << socketName << " !");
            return false;
        }

        if (!applyPermissionsGroupToEmbeddedCompositingSocket(socketName, socketGroupName))
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "WaylandDisplay::addSocketToDisplayWithName(): Failed to set permissions on embedded compositor socket :"
                          << socketName << " !");
            return false;
        }

        LOG_INFO(CONTEXT_RENDERER,
                 "WaylandDisplay::addSocketToDisplayWithName(): Added wayland display on embedded compositor socket :"
                     << socketName << " !");

        return true;
    }

    bool WaylandDisplay::applyPermissionsGroupToEmbeddedCompositingSocket(const String& socketName, const String& socketGroupName)
    {
        if (socketGroupName.getLength() > 0u)
        {
            group  permissionGroup;
            group* permissionGroupResult = nullptr;
            char   bufferForStringFields[10000];

            const int status = getgrnam_r(socketGroupName.c_str(),
                                          &permissionGroup,
                                          bufferForStringFields,
                                          sizeof(bufferForStringFields),
                                          &permissionGroupResult);
            if (0 != status)
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "WaylandDisplay::applyPermissionsGroupToEmbeddedCompositingSocket(): Could not "
                          "get group file entry for group name :"
                              << socketGroupName << ". Error :" << status);
                return false;
            }
            if (nullptr == permissionGroupResult)
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "WaylandDisplay::applyPermissionsGroupToEmbeddedCompositingSocket(): Could not "
                          "find group for socket permissions with group name :"
                              << socketGroupName);
                return false;
            }

            const gid_t groupID = permissionGroup.gr_gid;
            String      XDGRuntimeDir;
            PlatformEnvironmentVariables::get("XDG_RUNTIME_DIR", XDGRuntimeDir);
            const String fullSocketFilePath     = XDGRuntimeDir + String("/") + socketName;
            const String fullSocketLockFilePath = fullSocketFilePath + String(".lock");
            const int    chownStatusSocket      = chown(fullSocketFilePath.c_str(), -1, groupID);
            if (0 != chownStatusSocket)
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "WaylandDisplay::applyPermissionsGroupToEmbeddedCompositingSocket(): Could not "
                          "set group :"
                              << socketGroupName << " permission on :" << fullSocketFilePath
                              << ". Error:" << strerror(errno));
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER,
                     "WaylandDisplay::applyPermissionsGroupToEmbeddedCompositingSocket(): Successfully set "
                     "group :"
                         << socketGroupName << " permission on socket :" << fullSocketFilePath);

            // .lock file
            const int chownStatusLockfile = chown(fullSocketLockFilePath.c_str(), -1, groupID);
            if (0 != chownStatusLockfile)
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "WaylandDisplay::::applyPermissionsGroupToEmbeddedCompositingSocket(): Could not "
                          "set group :"
                              << socketGroupName << " permission on :" << fullSocketLockFilePath
                              << ". Error:" << strerror(errno));
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER,
                     "WaylandDisplay::::applyPermissionsGroupToEmbeddedCompositingSocket(): Successfully "
                     "set group :"
                         << socketGroupName << " permission on :" << fullSocketLockFilePath);
        }
        return true;
    }

    wl_display* WaylandDisplay::get() const
    {
        return m_display;
    }

    IWaylandGlobal* WaylandDisplay::createGlobal(const wl_interface *interface, int version, void *data, wl_global_bind_func_t bind)
    {
        wl_global* global = wl_global_create(m_display, interface, version, data, bind);
        if (nullptr != global)
        {
            return new WaylandGlobal(global);
        }
        else
        {
            return nullptr;
        }
    }

    void WaylandDisplay::dispatchEventLoop()
    {
        wl_event_loop* loop = wl_display_get_event_loop(m_display);
        // This dispatches events of the wayland clients, which lead to the callbacks of WaylandCallbacks
        // beeing called, which in turn leads m_updatedStreamTextureSourceIds to be filled with the stream texture id's
        // of those surfaces, which need a texture update.
        wl_event_loop_dispatch(loop, 0u);
    }

    void WaylandDisplay::flushClients()
    {
        wl_display_flush_clients(m_display);
    }
}
