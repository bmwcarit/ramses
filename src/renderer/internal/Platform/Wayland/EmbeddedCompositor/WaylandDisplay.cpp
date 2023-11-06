//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandDisplay.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandCompositorConnection.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandGlobal.h"
#include "internal/PlatformAbstraction/PlatformEnvironmentVariables.h"
#include "internal/Core/Utils/LogMacros.h"
#include <grp.h>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>

namespace ramses::internal
{
    WaylandDisplay::WaylandDisplay() = default;

    bool WaylandDisplay::init(const std::string& socketName,  const std::string& socketGroupName, uint32_t socketPermissions, int socketFD)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandDisplay::init");

        m_display = wl_display_create();
        if (nullptr == m_display)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandDisplay::init Failed to create wayland display!");
            return false;
        }

        if (!addSocketToDisplay(socketName, socketGroupName, socketPermissions, socketFD))
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

    bool WaylandDisplay::addSocketToDisplay(const std::string& socketName, const std::string& socketGroupName, uint32_t socketPermissions, int socketFD)
    {
        const bool socketNameProvided = !socketName.empty();
        const bool socketFDProvided   = socketFD >= 0;

        if (socketFDProvided && !socketNameProvided)
        {
            return addSocketToDisplayWithFD(socketFD);
        }
        if (socketNameProvided && !socketFDProvided)
        {
            return addSocketToDisplayWithName(socketName, socketGroupName, socketPermissions);
        }
        if (socketNameProvided && socketFDProvided)
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
        const int result = wl_display_add_socket_fd(m_display, socketFD);

        constexpr int failureCode = -1;
        if(failureCode != result)
        {
            LOG_DEBUG(CONTEXT_RENDERER,
                      "WaylandDisplay::addSocketToDisplayWithFD(): Added wayland display on embedded compositor socket "
                      "provided by "
                      "systemd");
            return true;
        }
        LOG_ERROR(CONTEXT_RENDERER,
                  "WaylandDisplay::addSocketToDisplayWithFD(): Failed to add wayland display on embedded compositor socket "
                  "provided by RendererConfig::setWaylandEmbeddedCompositingSocketFD()");
        return false;
    }

    bool WaylandDisplay::addSocketToDisplayWithName(const std::string& socketName, const std::string& socketGroupName, uint32_t socketPermissions)
    {
        if (wl_display_add_socket(m_display, socketName.c_str()) < 0)
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "WaylandDisplay::addSocketToDisplayWithName(): Failed to add wayland display on embedded "
                      "compositor socket :"
                          << socketName << " !");
            return false;
        }

        const std::string socketFullPath = GetSocketFullPath(socketName);
        if (!ApplyGroupToEmbeddedCompositingSocket(socketFullPath, socketGroupName))
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "WaylandDisplay::addSocketToDisplayWithName(): Failed to set group '" << socketGroupName << "' on embedded compositor socket :"
                          << socketName << " !");
            return false;
        }

        if (!ApplyPermissionsToEmbeddedCompositingSocket(socketFullPath, socketPermissions))
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "WaylandDisplay::addSocketToDisplayWithName(): Failed to set permissions " << socketPermissions << " on embedded compositor socket :"
                          << socketName << " !");
            return false;
        }

        LOG_INFO(CONTEXT_RENDERER,
                 "WaylandDisplay::addSocketToDisplayWithName(): Added wayland display on embedded compositor socket :"
                     << socketName << " !");

        return true;
    }

    std::string WaylandDisplay::GetSocketFullPath(const std::string& socketName)
    {
        std::string XDGRuntimeDir;
        PlatformEnvironmentVariables::get("XDG_RUNTIME_DIR", XDGRuntimeDir);
        return std::string(fmt::format("{}/{}", XDGRuntimeDir, socketName));
    }

    bool WaylandDisplay::ApplyGroupToEmbeddedCompositingSocket(const std::string& socketFullPath, const std::string& socketGroupName)
    {
        if (!socketGroupName.empty())
        {
            group  permissionGroup{};
            group* permissionGroupResult = nullptr;
            std::array<char, 10000> bufferForStringFields; // NOLINT(cppcoreguidelines-pro-type-member-init)

            const int status = getgrnam_r(socketGroupName.c_str(),
                                          &permissionGroup,
                                          bufferForStringFields.data(),
                                          bufferForStringFields.size(),
                                          &permissionGroupResult);
            if (0 != status)
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "WaylandDisplay::ApplyGroupToEmbeddedCompositingSocket(): Could not "
                          "get group file entry for group name :"
                              << socketGroupName << ". Error :" << status);
                return false;
            }
            if (nullptr == permissionGroupResult)
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "WaylandDisplay::ApplyGroupToEmbeddedCompositingSocket(): Could not "
                          "find group for socket with group name :"
                              << socketGroupName);
                return false;
            }

            const gid_t groupID = permissionGroup.gr_gid;
            const std::string fullSocketLockFilePath = socketFullPath + ".lock";
            const int    chownStatusSocket      = chown(socketFullPath.c_str(), -1, groupID);
            if (0 != chownStatusSocket)
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "WaylandDisplay::ApplyGroupToEmbeddedCompositingSocket(): Could not "
                          "set group :"
                              << socketGroupName << " on :" << socketFullPath
                              << ". Error:" << strerror(errno));
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER,
                     "WaylandDisplay::ApplyGroupToEmbeddedCompositingSocket(): Successfully set "
                     "group :"
                         << socketGroupName << " on socket :" << socketFullPath);

            // .lock file
            const int chownStatusLockfile = chown(fullSocketLockFilePath.c_str(), -1, groupID);
            if (0 != chownStatusLockfile)
            {
                LOG_ERROR(CONTEXT_RENDERER,
                          "WaylandDisplay::::ApplyGroupToEmbeddedCompositingSocket(): Could not "
                          "set group :"
                              << socketGroupName << " on :" << fullSocketLockFilePath
                              << ". Error:" << strerror(errno));
                return false;
            }
            LOG_INFO(CONTEXT_RENDERER,
                     "WaylandDisplay::::ApplyGroupToEmbeddedCompositingSocket(): Successfully "
                     "set group :"
                         << socketGroupName << " on :" << fullSocketLockFilePath);
        }
        return true;
    }

    bool WaylandDisplay::ApplyPermissionsToEmbeddedCompositingSocket(const std::string& socketFullPath, uint32_t socketPermissions)
    {
        // if none given use default ug+rw
        if (socketPermissions == 0)
            socketPermissions = 0660;

        if (0 != ::chmod(socketFullPath.c_str(), socketPermissions))
        {
            LOG_ERROR(CONTEXT_RENDERER,
                      "WaylandDisplay::ApplyPermissionsToEmbeddedCompositingSocket(): Could not set permissions: "
                      << socketPermissions << " on :" << socketFullPath
                      << ". Error:" << strerror(errno));
            return false;
        }
        LOG_INFO(CONTEXT_RENDERER,
                 "WaylandDisplay::ApplyPermissionsToEmbeddedCompositingSocket(): Successfully set permissions: "
                 << socketPermissions << " on socket :" << socketFullPath);
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
        return nullptr;
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
