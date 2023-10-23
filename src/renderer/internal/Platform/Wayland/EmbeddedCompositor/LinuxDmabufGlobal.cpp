//  -------------------------------------------------------------------------
//  Copyright (C) 2019-2019, Garmin International, Inc. and its affiliates.
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <algorithm>
#include <cassert>

#include <wayland-egl-core.h>
#include <EGL/egl.h>

#include "wayland-client.h"
#include "wayland-server.h"

#include "linux-dmabuf-unstable-v1-server-protocol.h"
#include "internal/Platform/EGL/Context_EGL.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandGlobal.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandClient.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandDisplay.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/LinuxDmabufGlobal.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/LinuxDmabufConnection.h"
#include "internal/Platform/Wayland/WaylandEGLExtensionProcs.h"

namespace ramses::internal
{
    LinuxDmabufGlobal::LinuxDmabufGlobal(IEmbeddedCompositor_Wayland& compositor)
        : m_compositor(compositor)
    {
    }

    LinuxDmabufGlobal::~LinuxDmabufGlobal()
    {
        assert(nullptr == m_waylandGlobal);
    }

    bool LinuxDmabufGlobal::init(WaylandDisplay& serverDisplay, Context_EGL& context)
    {
        WaylandEGLExtensionProcs procs(context.getEglDisplay());

        if (!procs.areDmabufExtensionsSupported())
        {
            return false;
        }

        const int maximumSupportedLinuxDmabufInterfaceVersion = 3;
        const int supportedLinuxDmabufInterfaceVersion = std::min(maximumSupportedLinuxDmabufInterfaceVersion, zwp_linux_dmabuf_v1_interface.version);

        m_waylandGlobal = serverDisplay.createGlobal(&zwp_linux_dmabuf_v1_interface, supportedLinuxDmabufInterfaceVersion, this, GlobalBindCallback);

        return true;
    }

    void LinuxDmabufGlobal::destroy()
    {
        if (nullptr != m_waylandGlobal)
        {
            delete m_waylandGlobal;
            m_waylandGlobal = nullptr;
        }
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    void LinuxDmabufGlobal::globalBind(IWaylandClient& client, uint32_t version, uint32_t id)
    {
        auto* connection = new LinuxDmabufConnection(client, version, id);

        connection->sendFormats();

        // Don't keep track of any bookkeeping on the connection object. It'll automatically be
        // reaped when the client disconnects or when the client destroys it.
    }

    void LinuxDmabufGlobal::GlobalBindCallback(wl_client* client, void* data, [[maybe_unused]] uint32_t version, uint32_t id)
    {
        auto* global = static_cast<LinuxDmabufGlobal*>(data);
        WaylandClient waylandClient(client);
        global->globalBind(waylandClient, version, id);
    }
}
