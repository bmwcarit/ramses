//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandIVIApplicationGlobal.h"
#include "EmbeddedCompositor_Wayland/WaylandDisplay.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "EmbeddedCompositor_Wayland/WaylandIVIApplicationConnection.h"
#include "EmbeddedCompositor_Wayland/IWaylandGlobal.h"
#include "PlatformAbstraction/PlatformMath.h"
#include "Utils/LogMacros.h"
#include <cassert>

namespace ramses_internal
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

        WaylandIVIApplicationConnection* waylandIVIApplicationConnection = new WaylandIVIApplicationConnection(client, version, id, m_compositor);
        // Registers callback for destruction, when the corresponding resource is destroyed

        if (!waylandIVIApplicationConnection->wasSuccessfullyInitialized())
        {
            delete waylandIVIApplicationConnection;
        }
    }

    void WaylandIVIApplicationGlobal::IVIApplicationBindCallback(wl_client* client, void* data, uint32_t version, uint32_t id)
    {
        UNUSED(version)
        WaylandIVIApplicationGlobal* shell = static_cast<WaylandIVIApplicationGlobal*>(data);
        WaylandClient waylandClient(client);
        shell->iviApplicationBind(waylandClient, version, id);
    }
}

