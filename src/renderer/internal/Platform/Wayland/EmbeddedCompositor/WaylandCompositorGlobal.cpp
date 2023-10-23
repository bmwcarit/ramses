//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandCompositorGlobal.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandCompositorConnection.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandClient.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IEmbeddedCompositor_Wayland.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandDisplay.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandGlobal.h"
#include "internal/PlatformAbstraction/PlatformMath.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"
#include <cassert>

namespace ramses::internal
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
        auto* waylandCompositorConnection = new WaylandCompositorConnection(client, version, id, m_compositor);
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
        auto* waylandCompositorGlobal = static_cast<WaylandCompositorGlobal*>(data);
        WaylandClient waylandClient(client);
        waylandCompositorGlobal->compositorBind(waylandClient, version, id);
    }
}
