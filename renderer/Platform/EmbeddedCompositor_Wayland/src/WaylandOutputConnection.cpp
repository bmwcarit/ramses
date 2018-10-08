//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandOutputConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandOutputResource.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    WaylandOutputConnection::WaylandOutputConnection(
        IWaylandClient& client, uint32_t version, uint32_t id, int32_t width, int32_t height, int32_t refresh)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandOutputConnection::WaylandOutputConnection Connection created");

        m_resource = client.outputResourceCreate(&wl_output_interface, version, id);
        if (m_resource)
        {
            m_resource->setImplementation(&m_outputInterface, this, ResourceDestroyedCallback);

            // Sends information about geometry. This is currently just some fake data.
            m_resource->outputSendGeometry(0,
                                           0,
                                           width / 6,
                                           height / 6,
                                           WL_OUTPUT_SUBPIXEL_UNKNOWN,
                                           "unknown",
                                           "unknown",
                                           WL_OUTPUT_TRANSFORM_NORMAL);

            // Sends current mode of output of parent compositor.
            m_resource->outputSendMode(WL_OUTPUT_MODE_CURRENT | WL_OUTPUT_MODE_PREFERRED, width, height, refresh);

            if (version >= WL_OUTPUT_SCALE_SINCE_VERSION)
            {
                m_resource->outputSendScale(1);
            }

            if (version >= WL_OUTPUT_DONE_SINCE_VERSION)
            {
                m_resource->outputSendDone();
            }
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandOutputConnection::WaylandOutputConnection(): Could not create wayland resource");
            client.postNoMemory();
        }
    }

    WaylandOutputConnection::~WaylandOutputConnection()
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandOutputConnection::~WaylandOutputConnection Connection destroyed");

        if (nullptr != m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_outputInterface, this, nullptr);
            delete m_resource;
        }
    }

    bool WaylandOutputConnection::wasSuccessfullyInitialized() const
    {
        return nullptr != m_resource;
    }

    void WaylandOutputConnection::resourceDestroyed()
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandOutputConnection::resourceDestroyed");
        assert(nullptr != m_resource);

        // wl_resource is destroyed outside by the Wayland library, so m_resource loses the ownership of the
        // Wayland resource, so that we don't call wl_resource_destroy.
        m_resource->disownWaylandResource();
    }


    void WaylandOutputConnection::ResourceDestroyedCallback(wl_resource* outputConnectionResource)
    {
        WaylandOutputConnection* outputConnection =
            static_cast<WaylandOutputConnection*>(wl_resource_get_user_data(outputConnectionResource));

        outputConnection->resourceDestroyed();
        delete outputConnection;
    }

    void WaylandOutputConnection::OutputReleaseCallback(wl_client* client, wl_resource* outputConnectionResource)
    {
        UNUSED(client)
        WaylandOutputConnection* outputConnection =
                    static_cast<WaylandOutputConnection*>(wl_resource_get_user_data(outputConnectionResource));

        delete outputConnection;
    }
}
