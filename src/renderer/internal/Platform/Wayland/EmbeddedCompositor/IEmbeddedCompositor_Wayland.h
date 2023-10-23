//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

namespace ramses::internal
{
    class IWaylandBuffer;
    class IWaylandSurface;
    class WaylandBufferResource;
    class IWaylandCompositorConnection;
    class IWaylandRegion;

    class IEmbeddedCompositor_Wayland
    {
    public:
        virtual ~IEmbeddedCompositor_Wayland() = default;

        virtual void handleBufferDestroyed(IWaylandBuffer& buffer) = 0;
        virtual void addWaylandSurface(IWaylandSurface& waylandSurface) = 0;
        virtual void removeWaylandSurface(IWaylandSurface& waylandSurface) = 0;
        virtual IWaylandBuffer& getOrCreateBuffer(WaylandBufferResource& bufferResource) = 0;
        virtual void addWaylandCompositorConnection(IWaylandCompositorConnection& waylandCompositorConnection) = 0;
        virtual void removeWaylandCompositorConnection(IWaylandCompositorConnection& waylandCompositorConnection) = 0;
        virtual void addWaylandRegion(IWaylandRegion& waylandRegion) = 0;
        virtual void removeWaylandRegion(IWaylandRegion& waylandRegion) = 0;
    };
}
