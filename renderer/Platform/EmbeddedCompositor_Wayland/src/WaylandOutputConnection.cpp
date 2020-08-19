//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandOutputConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "EmbeddedCompositor_Wayland/IWaylandResource.h"
#include "Utils/LogMacros.h"
#include <wayland-server.h>
#include <wayland-server-protocol.h>

namespace ramses_internal
{
    WaylandOutputConnection::WaylandOutputConnection(const WaylandOutputParams& waylandOutputParams, IWaylandClient& client, uint32_t version, uint32_t id)
        : m_waylandOutputParams(waylandOutputParams)
        , m_clientCredentials(client.getCredentials())
    {
        LOG_DEBUG(CONTEXT_RENDERER, "WaylandOutputConnection::WaylandOutputConnection Connection created");

        m_resource = client.resourceCreate(&wl_output_interface, version, id);
        if (nullptr != m_resource)
        {
            m_resource->setImplementation(&m_outputInterface, this, ResourceDestroyedCallback);
            LOG_INFO(CONTEXT_RENDERER, "WaylandOutputConnection::WaylandOutputConnection(): Output interface is now provided  " << m_clientCredentials);

            sendOutputParams(version);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputConnection::WaylandOutputConnection(): could not create wayland resource  " << m_clientCredentials);
            client.postNoMemory();
        }
    }

    void WaylandOutputConnection::sendOutputParams(uint32_t protocolVersion)
    {
        wl_resource* nativeResource = static_cast<wl_resource*>(m_resource->getWaylandNativeResource());

        //Physical width and height set to zero as per documentation in:
        //https://github.com/wayland-project/wayland/blob/1361da9cd5a719b32d978485a29920429a31ed25/tests/data/example-server.h#L3897
        //https://github.com/wayland-project/weston/blob/2eda978e95aaa18516cfc007aa91c8901fa251c5/libweston/compositor.c#L4954

        //Note about wl_output protocol versions
        //RAMSES EC implementation of wl_output protocol supports up to version 3. Wayland clients are allowed to request any (older) version
        //of the protocol regardless of the highest version supported by the server:
        //Version 1: supports messages "geometry" and "mode" from server to client
        //Version 2: additionally supports "scale" and "done" from server to client
        //Version 3: additionally supports "release" from client to server (messages from server to client identical to version 2)

        const int32_t physicalWidth = 0;
        const int32_t physicalHeight = 0;
        wl_output_send_geometry(nativeResource, 0, 0, physicalWidth, physicalHeight, WL_OUTPUT_SUBPIXEL_UNKNOWN, "", "", WL_OUTPUT_TRANSFORM_NORMAL);
        if(protocolVersion >= WL_OUTPUT_SCALE_SINCE_VERSION)
            wl_output_send_scale(nativeResource, 1);
        if(protocolVersion >= WL_OUTPUT_MODE_SINCE_VERSION)
            wl_output_send_mode(nativeResource, WL_OUTPUT_MODE_CURRENT, static_cast<int32_t>(m_waylandOutputParams.width), static_cast<int32_t>(m_waylandOutputParams.height), 0);
        if(protocolVersion >= WL_OUTPUT_DONE_SINCE_VERSION)
            wl_output_send_done(nativeResource);
    }

    WaylandOutputConnection::~WaylandOutputConnection()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandOutputConnection::~WaylandOutputConnection(): Connection destroyed  " << m_clientCredentials);

        if (m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_outputInterface, this, nullptr);
            delete m_resource;
        }
    }

    void WaylandOutputConnection::resourceDestroyed()
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandOutputConnection::resourceDestroyed");
        assert(nullptr != m_resource);

        // wl_resource is destroyed outside by the Wayland library, so m_resource loses the ownership of the
        // Wayland resource, so that we don't call wl_resource_destroy.
        m_resource->disownWaylandResource();
    }

    void WaylandOutputConnection::ResourceDestroyedCallback(wl_resource* clientResource)
    {
        WaylandOutputConnection* outputConnection =
            static_cast<WaylandOutputConnection*>(wl_resource_get_user_data(clientResource));

        outputConnection->resourceDestroyed();
        delete outputConnection;
    }

    void WaylandOutputConnection::OutputReleaseCallback(wl_client* client, wl_resource *clientResource)
    {
        UNUSED(client);
        WaylandOutputConnection* outputConnection =
            static_cast<WaylandOutputConnection*>(wl_resource_get_user_data(clientResource));
        delete outputConnection;
    }
}
