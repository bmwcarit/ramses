//  -------------------------------------------------------------------------
//  Copyright (C) 2019-2019, Garmin International, Inc. and its affiliates.
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <cassert>

#include <drm_fourcc.h>

#include "wayland-server.h"

#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandClient.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/INativeWaylandResource.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandClient.h"

#include "internal/Platform/Wayland/EmbeddedCompositor/LinuxDmabufConnection.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/LinuxDmabufParams.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/Warnings.h"

namespace ramses::internal
{
    struct zwp_linux_dmabuf_v1_interface const LinuxDmabufConnection::m_dmabufInterface =
    {
        .destroy = LinuxDmabufConnection::DmabufDestroyCallback,
        .create_params = LinuxDmabufConnection::DmabufCreateParamsCallback,
    };

    LinuxDmabufConnection::LinuxDmabufConnection(IWaylandClient& client, uint32_t version, uint32_t id)
        : m_clientCredentials(client.getCredentials())
    {
        m_resource = client.resourceCreate(&zwp_linux_dmabuf_v1_interface, static_cast<int>(version), id);

        if (m_resource)
        {
            LOG_INFO(CONTEXT_RENDERER, "LinuxDmabufConnection::LinuxDmabufConnection(): DMA BUF interface is now provided  " << m_clientCredentials);

            m_resource->setImplementation(&m_dmabufInterface, this, ResourceDestroyedCallback);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "LinuxDmabufConnection::LinuxDmabufConnection(): Could not create wayland resource  " << m_clientCredentials);
            client.postNoMemory();
        }
    }

    LinuxDmabufConnection::~LinuxDmabufConnection()
    {
        LOG_INFO(CONTEXT_RENDERER, "LinuxDmabufConnection::~LinuxDmabufConnection(): Connection destroyed  " << m_clientCredentials);

        if (nullptr != m_resource)
        {
            // Remove ResourceDestroyedCallback
            m_resource->setImplementation(&m_dmabufInterface, this, nullptr);
            m_resource->destroy();
            delete m_resource;
        }
    }

    bool LinuxDmabufConnection::wasSuccessfullyInitialized() const
    {
        return nullptr != m_resource;
    }

    void LinuxDmabufConnection::ResourceDestroyedCallback(wl_resource* dmabufConnectionResource)
    {
        auto* dmabufConnection = static_cast<LinuxDmabufConnection*>(wl_resource_get_user_data(dmabufConnectionResource));

        dmabufConnection->resourceDestroyed();
        delete dmabufConnection;
    }

    void LinuxDmabufConnection::resourceDestroyed()
    {
        LOG_TRACE(CONTEXT_RENDERER, "LinuxDmabufConnection::resourceDestroyed");
        assert(nullptr != m_resource);

        // wl_resource is destroyed outside by the Wayland library, so our job here is to abandon
        // ownership of the Wayland resource so that we don't call wl_resource_destroy().
        delete m_resource;
        m_resource = nullptr;
    }

    void LinuxDmabufConnection::DmabufDestroyCallback(wl_client* /*client*/, wl_resource* dmabufConnectionResource)
    {
        auto* connection = static_cast<LinuxDmabufConnection*>(wl_resource_get_user_data(dmabufConnectionResource));
        delete connection;
    }

    void LinuxDmabufConnection::DmabufCreateParamsCallback(wl_client* /*client*/, wl_resource* dmabufConnectionResource, uint32_t id)
    {
        auto* connection = static_cast<LinuxDmabufConnection*>(wl_resource_get_user_data(dmabufConnectionResource));
        connection->createParams(id);
    }

    void LinuxDmabufConnection::sendFormats()
    {
        // Send the minimum permitted set of fallback formats. A fancier implementation would
        // sniff these out using eglQueryDmaBufFormatsEXT() and eglQueryDmaBufModifiersEXT().
        PUSH_DISABLE_C_STYLE_CAST_WARNING
        const std::array<uint32_t,2> fallbackFormats =
        {
            DRM_FORMAT_ARGB8888,
            DRM_FORMAT_XRGB8888,
        };

        // Declare local aliases bracked around exemptions for the parser's use of old-style C casts
        constexpr uint64_t drmFormatModInvalid = DRM_FORMAT_MOD_INVALID;
        // NOLINTNEXTLINE(hicpp-signed-bitwise)
        constexpr uint32_t drmFormatModLinear = DRM_FORMAT_MOD_LINEAR;
        POP_DISABLE_C_STYLE_CAST_WARNING

        wl_resource* resource = m_resource->getLowLevelHandle();

        for (auto format: fallbackFormats)
        {
            // Send the minimum permitted set of modifiers for this format. In the future sniff
            // these out with eglQueryDmaBufModifiersExt().
            // NOLINTNEXTLINE(modernize-avoid-c-arrays)
            static const uint64_t fallbackModifiers[] =
            {
            };

            uint64_t const* modifiers = nullptr;
            size_t numModifiers = 0;

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
        wl_resource* resource = m_resource->getLowLevelHandle();
        WaylandClient client(wl_resource_get_client(resource));

        auto* params = new LinuxDmabufParams(client, m_resource->getVersion(), id);

        if (!params->wasSuccessfullyInitialized())
        {
            delete params;
        }
    }
}
