//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDREGION_H
#define RAMSES_WAYLANDREGION_H

#include "EmbeddedCompositor_Wayland/IWaylandRegion.h"
#include "RendererAPI/Types.h"
#include "RendererLib/RendererLogContext.h"

#include "wayland-server.h"

namespace ramses_internal
{

    class IEmbeddedCompositor_Wayland;
    class IWaylandClient;
    class INativeWaylandResource;

    class WaylandRegion: public IWaylandRegion
    {
    public:

        WaylandRegion(IEmbeddedCompositor_Wayland& compositor, IWaylandClient& client, uint32_t version, uint32_t id);
        ~WaylandRegion();

        virtual void regionAdd(IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height) override;
        virtual void regionSubtract(IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height) override;
        virtual void resourceDestroyed() override;

    private:
        static void RegionDestroyCallback(wl_client* client, wl_resource* regionResource);
        static void RegionAddCallback(wl_client* client, wl_resource* regionResource, int32_t x, int32_t y, int32_t width, int32_t height);
        static void RegionSubtractCallback(wl_client* client, wl_resource* regionResource, int32_t x, int32_t y, int32_t width, int32_t height);

        static void ResourceDestroyedCallback(wl_resource* regionResource);

        INativeWaylandResource* m_resource = nullptr;
        IEmbeddedCompositor_Wayland& m_compositor;

        const struct Region_Interface : private wl_region_interface
        {
            Region_Interface()
            {
                destroy  = RegionDestroyCallback;
                add      = RegionAddCallback;
                subtract = RegionSubtractCallback;
            }
        } m_regionInterface;
    };
}

#endif
