//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandShellGlobal.h"
#include "EmbeddedCompositor_Wayland/WaylandShellConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandDisplay.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "EmbeddedCompositor_Wayland/IWaylandGlobal.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "Utils/LogMacros.h"
#include "assert.h"

namespace ramses_internal
{
    WaylandShellGlobal::WaylandShellGlobal()
    {
    }

    WaylandShellGlobal::~WaylandShellGlobal()
    {
        assert(nullptr == m_waylandGlobal);
    }

    bool WaylandShellGlobal::init(WaylandDisplay& serverDisplay)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellGlobal::init");

        // RAMSES currently supports the shell interface up to version 1.
        // For supporting newer versions, before increasing the number here, take care that newly introduced callbacks
        // are ALL handled in WaylandShellConnection::Shell_Interface.
        const int maximumSupportedShellInterfaceVersion = 1;

        const int supportedShellInterfaceVersion = std::min(maximumSupportedShellInterfaceVersion, wl_shell_interface.version);
        m_waylandGlobal = serverDisplay.createGlobal(&wl_shell_interface, supportedShellInterfaceVersion, this, ShellBindCallback);

        if (nullptr == m_waylandGlobal)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandShellGlobal::init(): Failed to create global!");
            return false;
        }

        return true;
    }

    void WaylandShellGlobal::destroy()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandShellGlobal::deinit");

        delete m_waylandGlobal;
        m_waylandGlobal = nullptr;
    }

    void WaylandShellGlobal::shellBind(IWaylandClient& client, uint32_t version, uint32_t id)
    {
        WaylandShellConnection* waylandShellConnection = new WaylandShellConnection(client, version, id);
        // Registers callback for destruction, when the corresponding resource is destroyed

        if (!waylandShellConnection->wasSuccessfullyInitialized())
        {
            delete waylandShellConnection;
        }
    }

    void WaylandShellGlobal::ShellBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id)
    {
        UNUSED(version)
        WaylandShellGlobal* shell = static_cast<WaylandShellGlobal*>(data);
        WaylandClient waylandClient(client);
        shell->shellBind(waylandClient, version, id);
    }
}

