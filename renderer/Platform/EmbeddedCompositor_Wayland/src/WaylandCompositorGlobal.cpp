//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandCompositorGlobal.h"
#include "EmbeddedCompositor_Wayland/WaylandCompositorConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "EmbeddedCompositor_Wayland/IEmbeddedCompositor_Wayland.h"
#include "EmbeddedCompositor_Wayland/IWaylandDisplay.h"
#include "EmbeddedCompositor_Wayland/IWaylandGlobal.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "Utils/ThreadLocalLogForced.h"
#include <cassert>

namespace ramses_internal
{
    WaylandCompositorGlobal::WaylandCompositorGlobal(IEmbeddedCompositor_Wayland& compositor)
        : m_compositor(compositor)
    {
    }

    bool WaylandCompositorGlobal::init(IWaylandDisplay& serverDisplay)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandCompositorGlobal::init");

        // RAMSES currently supports the wayland compositor interface up to version 4.
        // For supporting newer versions, before increasing the number here, take care that newly introduced callbacks
        // are ALL handled in WaylandCompositorConnection::Compositor_Interface.
        const int maximumSupportedCompositorInterfaceVersion = 4;

        const int supportedCompositorInterfaceVersion = std::min(maximumSupportedCompositorInterfaceVersion,  wl_compositor_interface.version);
        m_waylandGlobal = serverDisplay.createGlobal(&wl_compositor_interface, supportedCompositorInterfaceVersion, this, CompositorBindCallback);

        if (nullptr == m_waylandGlobal)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandCompositorGlobal::init(): Failed to create global!");
            return false;
        }

        return true;
    }

    WaylandCompositorGlobal::~WaylandCompositorGlobal()
    {
        assert(nullptr == m_waylandGlobal);
    }

    void WaylandCompositorGlobal::destroy()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandCompositorGlobal::~WaylandCompositorGlobal");

        delete m_waylandGlobal;
        m_waylandGlobal = nullptr;
    }

    void WaylandCompositorGlobal::compositorBind(IWaylandClient& client, uint32_t version, uint32_t id)
    {
        WaylandCompositorConnection* waylandCompositorConnection = new WaylandCompositorConnection(client, version, id, m_compositor);
        if (waylandCompositorConnection->wasSuccessfullyInitialized())
        {
            m_compositor.addWaylandCompositorConnection(*waylandCompositorConnection);
        }
        else
        {
            delete waylandCompositorConnection;
        }
    }

    void WaylandCompositorGlobal::CompositorBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id)
    {
        WaylandCompositorGlobal* waylandCompositorGlobal = static_cast<WaylandCompositorGlobal*>(data);
        WaylandClient waylandClient(client);
        waylandCompositorGlobal->compositorBind(waylandClient, version, id);
    }
}
