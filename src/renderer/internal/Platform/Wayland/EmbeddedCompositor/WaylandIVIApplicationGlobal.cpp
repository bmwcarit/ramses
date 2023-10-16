//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandIVIApplicationGlobal.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandDisplay.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandClient.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandIVIApplicationConnection.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandGlobal.h"
#include "internal/PlatformAbstraction/PlatformMath.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"
#include <cassert>

namespace ramses::internal
{
    WaylandIVIApplicationGlobal::WaylandIVIApplicationGlobal(EmbeddedCompositor_Wayland& compositor)
        : m_compositor(compositor)
    {
    }

    WaylandIVIApplicationGlobal::~WaylandIVIApplicationGlobal()
    {
        assert(nullptr == m_waylandGlobal);
    }

    bool WaylandIVIApplicationGlobal::init(WaylandDisplay& serverDisplay)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandIVIApplicationGlobal::init");

        // RAMSES currently supports the ivi-application interface up to version 1.
        // For supporting newer versions, before increasing the number here, take care that newly introduced callbacks
        // are ALL handled in WaylandIVIApplicationConnection::IVIApplication_Interface.
        const int maximumSupportedIVIApplicationInterfaceVersion = 1;

        const int supportedIVIApplicationInterfaceVersion = std::min(maximumSupportedIVIApplicationInterfaceVersion, ivi_application_interface.version);
        m_waylandGlobal = serverDisplay.createGlobal(&ivi_application_interface, supportedIVIApplicationInterfaceVersion, this, IVIApplicationBindCallback);

        if (nullptr == m_waylandGlobal)
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandIVIApplicationGlobal::init(): Failed to create global!");
            return false;
        }

        return true;
    }

    void WaylandIVIApplicationGlobal::destroy()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandIVIApplicationGlobal::destroy");

        delete m_waylandGlobal;
        m_waylandGlobal = nullptr;
    }

    void WaylandIVIApplicationGlobal::iviApplicationBind(IWaylandClient& client, uint32_t version, uint32_t id)
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandIVIApplicationGlobal::iviApplicationBind");

        auto* waylandIVIApplicationConnection = new WaylandIVIApplicationConnection(client, version, id, m_compositor);
        // Registers callback for destruction, when the corresponding resource is destroyed

        if (!waylandIVIApplicationConnection->wasSuccessfullyInitialized())
        {
            delete waylandIVIApplicationConnection;
        }
    }

    void WaylandIVIApplicationGlobal::IVIApplicationBindCallback(wl_client* client, void* data, [[maybe_unused]] uint32_t version, uint32_t id)
    {
        auto* shell = static_cast<WaylandIVIApplicationGlobal*>(data);
        WaylandClient waylandClient(client);
        shell->iviApplicationBind(waylandClient, version, id);
    }
}

