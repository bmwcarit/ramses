//  -------------------------------------------------------------------------
//  Copyright (C) 2019-2019, Garmin International, Inc. and its affiliates.
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <algorithm>
#include <assert.h>

#include <wayland-egl-core.h>
#include <EGL/egl.h>

#include "wayland-client.h"
#include "wayland-server.h"

#include "linux-dmabuf-unstable-v1-server-protocol.h"
#include "Context_EGL/Context_EGL.h"
#include "EmbeddedCompositor_Wayland/IWaylandGlobal.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "EmbeddedCompositor_Wayland/WaylandDisplay.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabufGlobal.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabufConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandEGLExtensionProcs.h"

namespace ramses_internal
{
    LinuxDmabufGlobal::LinuxDmabufGlobal(IEmbeddedCompositor_Wayland& compositor)
        : m_compositor(compositor)
    {
    }

    LinuxDmabufGlobal::~LinuxDmabufGlobal()
    {
        assert(nullptr == m_waylandGlobal);
    }

    bool LinuxDmabufGlobal::init(WaylandDisplay& serverDisplay, IContext& context)
    {
        EGLDisplay eglDisplay;

        Context_EGL* contextEgl = dynamic_cast<Context_EGL*>(&context);

        if (!contextEgl)
        {
            // Running in a test case
            return false;
        }

        eglDisplay = contextEgl->getEglDisplay();
        assert(eglDisplay != EGL_NO_DISPLAY);

        WaylandEGLExtensionProcs procs(eglDisplay);

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

    void LinuxDmabufGlobal::globalBind(IWaylandClient& client, uint32_t version, uint32_t id)
    {
        LinuxDmabufConnection* connection = new LinuxDmabufConnection(client, version, id);

        connection->sendFormats();

        // Don't keep track of any bookkeeping on the connection object. It'll automatically be
        // reaped when the client disconnects or when the client destroys it.
    }

    void LinuxDmabufGlobal::GlobalBindCallback(wl_client *client, void* data, uint32_t version, uint32_t id)
    {
        UNUSED(version)
        LinuxDmabufGlobal* global = static_cast<LinuxDmabufGlobal*>(data);
        WaylandClient waylandClient(client);
        global->globalBind(waylandClient, version, id);
    }
}
