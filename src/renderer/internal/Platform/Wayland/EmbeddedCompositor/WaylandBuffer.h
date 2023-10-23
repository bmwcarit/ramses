//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#pragma once

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandBufferResource.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandBuffer.h"
#include <wayland-server.h>

#include <cstdint>

namespace ramses::internal
{
    class IEmbeddedCompositor_Wayland;

    class WaylandBuffer: public IWaylandBuffer
    {

    public:
        WaylandBuffer(WaylandBufferResource& bufferResource, IEmbeddedCompositor_Wayland& compositor);
        ~WaylandBuffer() override;

        [[nodiscard]] WaylandBufferResource& getResource() const override;
        void reference() override;
        void release() override;
        [[nodiscard]] bool isSharedMemoryBuffer() const override;
        void logInfos(RendererLogContext& context, const WaylandEGLExtensionProcs& eglExt) const override;

    private:

        static void ClientBufferDestroyed(wl_listener* listener, void* data);

        WaylandBufferResource& m_bufferResource;

        wl_listener m_destroyListener{};

        /** Reference counter.
         * The counter is increased each time the buffer is attached to a surface
         * and is decreased when it's detached.
         * This accounting is used for notifying the clients with a
         * wl_buffer.release event when this buffer is no more attached to any
         * surface.
         * Furthermore it is valid for client to create a wl_buffer, attach it to
         * a surface and then destroy it right after wl_surface.commit.
         * So we will keep this buffer unitl the reference counter
         * drops to zero. */
        int m_refCount{0};

        IEmbeddedCompositor_Wayland& m_compositor;
    };

}
