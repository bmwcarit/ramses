//  -------------------------------------------------------------------------
//  Copyright (C) 2019-2019, Garmin International, Inc. and its affiliates.
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <assert.h>

#include <drm_fourcc.h>

#include "wayland-server.h"

#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "EmbeddedCompositor_Wayland/IWaylandResource.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"

#include "EmbeddedCompositor_Wayland/LinuxDmabufConnection.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabufParams.h"
#include "Utils/LogMacros.h"
#include "Utils/Warnings.h"

namespace ramses_internal
{
    struct zwp_linux_dmabuf_v1_interface const LinuxDmabufConnection::m_dmabufInterface =
    {
        .destroy = LinuxDmabufConnection::DmabufDestroyCallback,
        .create_params = LinuxDmabufConnection::DmabufCreateParamsCallback,
    };

    LinuxDmabufConnection::LinuxDmabufConnection(IWaylandClient& client, uint32_t version, uint32_t id)
    {
        m_resource = client.resourceCreate(&zwp_linux_dmabuf_v1_interface, version, id);

        if (m_resource)
        {
            m_resource->setImplementation(&m_dmabufInterface, this, ResourceDestroyedCallback);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "LinuxDmabufConnection::LinuxDmabufConnection(): Could not create wayland resource");
            client.postNoMemory();
        }
    }

    LinuxDmabufConnection::~LinuxDmabufConnection()
    {
        if (nullptr != m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_dmabufInterface, this, nullptr);
            delete m_resource;
        }
    }

    bool LinuxDmabufConnection::wasSuccessfullyInitialized() const
    {
        return nullptr != m_resource;
    }

    void LinuxDmabufConnection::ResourceDestroyedCallback(wl_resource* dmabufConnectionResource)
    {
        LinuxDmabufConnection* dmabufConnection = static_cast<LinuxDmabufConnection*>(wl_resource_get_user_data(dmabufConnectionResource));

        dmabufConnection->resourceDestroyed();
        delete dmabufConnection;
    }

    void LinuxDmabufConnection::resourceDestroyed()
    {
        LOG_TRACE(CONTEXT_RENDERER, "LinuxDmabufConnection::resourceDestroyed");
        assert(nullptr != m_resource);

        // wl_resource is destroyed outside by the Wayland library, so our job here is to abandon
        // ownership of the Wayland resource so that we don't call wl_resource_destroy().
        m_resource->disownWaylandResource();
    }

    void LinuxDmabufConnection::DmabufDestroyCallback(wl_client* /*client*/, wl_resource* dmabufConnectionResource)
    {
        LinuxDmabufConnection* connection = static_cast<LinuxDmabufConnection*>(wl_resource_get_user_data(dmabufConnectionResource));
        delete connection;
    }

    void LinuxDmabufConnection::DmabufCreateParamsCallback(wl_client* /*client*/, wl_resource* dmabufConnectionResource, uint32_t id)
    {
        LinuxDmabufConnection* connection = static_cast<LinuxDmabufConnection*>(wl_resource_get_user_data(dmabufConnectionResource));
        connection->createParams(id);
    }

    void LinuxDmabufConnection::sendFormats()
    {
        // Send the minimum permitted set of fallback formats. A fancier implementation would
        // sniff these out using eglQueryDmaBufFormatsEXT() and eglQueryDmaBufModifiersEXT().
        PUSH_DISABLE_C_STYLE_CAST_WARNING
        static const uint32_t fallbackFormats[] =
        {
            DRM_FORMAT_ARGB8888,
            DRM_FORMAT_XRGB8888,
        };

        // Declare local aliases bracked around exemptions for the parser's use of old-style C casts
        constexpr uint64_t drmFormatModInvalid = DRM_FORMAT_MOD_INVALID;
        constexpr uint32_t drmFormatModLinear = DRM_FORMAT_MOD_LINEAR;
        POP_DISABLE_C_STYLE_CAST_WARNING

        wl_resource* resource = static_cast<wl_resource*>(m_resource->getWaylandNativeResource());

        for (auto format: fallbackFormats)
        {
            // Send the minimum permitted set of modifiers for this format. In the future sniff
            // these out with eglQueryDmaBufModifiersExt().
            static const uint64_t fallbackModifiers[] =
            {
            };

            uint64_t const* modifiers;
            size_t numModifiers;

            if ((sizeof(fallbackModifiers) / sizeof(fallbackModifiers[0])) == 0)
            {
                modifiers = &drmFormatModInvalid;
                numModifiers = 1;
            }
            else
            {
                modifiers = fallbackModifiers;
                numModifiers = sizeof(fallbackModifiers) / sizeof(fallbackModifiers[0]);
            }

            for (size_t i = 0; i < numModifiers; ++i)
            {
                if (m_resource->getVersion() >= ZWP_LINUX_DMABUF_V1_MODIFIER_SINCE_VERSION)
                {
                    uint32_t lo = modifiers[i] & 0xffffffff;
                    uint32_t hi = modifiers[i] >> 32;
                    zwp_linux_dmabuf_v1_send_modifier(resource, format, hi, lo);
                }
                else if (modifiers[i] == drmFormatModLinear || modifiers == &drmFormatModInvalid)
                {
                    zwp_linux_dmabuf_v1_send_format(resource, format);
                }
            }
        }
    }

    void LinuxDmabufConnection::createParams(uint32_t id)
    {
        wl_resource* resource = static_cast<wl_resource*>(m_resource->getWaylandNativeResource());
        WaylandClient client(wl_resource_get_client(resource));

        LinuxDmabufParams* params = new LinuxDmabufParams(client, m_resource->getVersion(), id);

        if (!params->wasSuccessfullyInitialized())
        {
            delete params;
        }
    }
}
