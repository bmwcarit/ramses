//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDIVISURFACE_H
#define RAMSES_WAYLANDIVISURFACE_H

#include "EmbeddedCompositor_Wayland/IWaylandIVISurface.h"
#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "ivi-application-server-protocol.h"
#include "wayland-server.h"

namespace ramses_internal
{
    class EmbeddedCompositor_Wayland;
    class INativeWaylandResource;
    class IWaylandSurface;

    class WaylandIVISurface: public IWaylandIVISurface
    {
    public:
        WaylandIVISurface(IWaylandClient& client, INativeWaylandResource& iviApplicationConnectionResource, WaylandIviSurfaceId iviSurfaceId, IWaylandSurface* surface, uint32_t id, EmbeddedCompositor_Wayland& compositor);
        ~WaylandIVISurface();
        bool wasSuccessfullyInitialized() const;
        virtual void surfaceWasDeleted() override;
        virtual void bufferWasSetToSurface(IWaylandBuffer* buffer) override;
        virtual WaylandIviSurfaceId getIviId() const override;
        virtual void resourceDestroyed() override;

    private:
        static void IVISurfaceDestroyCallback(wl_client* client, wl_resource* iviSurfaceResource);
        static void ResourceDestroyedCallback(wl_resource* iviSurfaceResource);

        const struct IVISurface_Interface : private ivi_surface_interface
        {
            IVISurface_Interface()
            {
                destroy = IVISurfaceDestroyCallback;
            }
        } m_iviSurfaceInterface;

        WaylandIviSurfaceId m_iviSurfaceId;
        IWaylandSurface* m_surface = nullptr;
        INativeWaylandResource* m_resource = nullptr;
        EmbeddedCompositor_Wayland& m_compositor;
        const WaylandClientCredentials  m_clientCredentials;
    };
}

#endif
