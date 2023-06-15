//  -------------------------------------------------------------------------
//  Copyright (C) 2019-2019, Garmin International, Inc. and its affiliates.
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LINUXDMABUFPARAMS_H
#define RAMSES_LINUXDMABUFPARAMS_H

#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "linux-dmabuf-unstable-v1-server-protocol.h"
#include "wayland-server.h"
#include <cstdint>

namespace ramses_internal
{
    class IWaylandClient;
    class INativeWaylandResource;
    class LinuxDmabufBufferData;

    class LinuxDmabufParams
    {
    public:
        LinuxDmabufParams(IWaylandClient& client, uint32_t version, uint32_t id);
        ~LinuxDmabufParams();

        [[nodiscard]] bool wasSuccessfullyInitialized() const;

    private:
        static void ResourceDestroyedCallback(wl_resource* dmabufParamsResource);

        static void DmabufParamsDestroyCallback(wl_client* client, wl_resource* dmabufParamsResource);
        static void DmabufParamsAddCallback(wl_client* client, wl_resource* dmabufParamsResource, int32_t fd, uint32_t plane_idx, uint32_t offset, uint32_t stride, uint32_t modifier_hi, uint32_t modifier_lo);
        static void DmabufParamsCreateCallback(wl_client* client, wl_resource* dmabufParamsResource, int32_t width, int32_t height, uint32_t format, uint32_t flags);
        static void DmabufParamsCreateImmedCallback(wl_client* client, wl_resource* dmabufParamsResource, uint32_t buffer_id, int32_t width, int32_t height, uint32_t format, uint32_t flags);

        void resourceDestroyed();

        void addPlane(int32_t fd, uint32_t index, uint32_t offset, uint32_t stride, uint32_t modifier_hi, uint32_t modifier_lo);
        void createBuffer(IWaylandClient& client, uint32_t bufferId, int32_t width, int32_t height, uint32_t format, uint32_t flags);

        static struct zwp_linux_buffer_params_v1_interface const m_paramsInterface;

        const WaylandClientCredentials  m_clientCredentials;
        INativeWaylandResource* m_resource = nullptr;
        LinuxDmabufBufferData* m_data = nullptr;

    friend class LinuxDmabuf;
    };
}

#endif
