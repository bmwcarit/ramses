//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_WAYLANDSURFACE_H
#define RAMSES_WAYLANDSURFACE_H

#include "EmbeddedCompositor_Wayland/IWaylandSurface.h"
#include "RendererLib/RendererLogContext.h"

#include "wayland-server.h"

namespace ramses_internal
{
    class IEmbeddedCompositor_Wayland;
    class IWaylandIVISurface;
    class IWaylandShellSurface;
    class IWaylandClient;
    class IWaylandResource;
    class WaylandCallbackResource;
    class IWaylandBuffer;

    class WaylandSurface: public IWaylandSurface
    {
    public:
        WaylandSurface(IEmbeddedCompositor_Wayland& compositor, IWaylandClient& client, uint32_t version, uint32_t id);
        ~WaylandSurface();

        virtual IWaylandBuffer* getWaylandBuffer() const override;
        virtual bool hasPendingBuffer() const override;
        virtual void setShellSurface(IWaylandShellSurface* shellSurface) override;
        virtual bool hasShellSurface() const override;
        virtual const String& getSurfaceTitle() const override;
        virtual void setIviSurface(IWaylandIVISurface* iviSurface) override;
        virtual bool hasIviSurface() const override;
        virtual WaylandIviSurfaceId getIviSurfaceId() const override;
        virtual uint64_t getNumberOfCommitedFrames() const override;
        virtual void resetNumberOfCommitedFrames() override;
        virtual uint64_t getNumberOfCommitedFramesSinceBeginningOfTime() const override;
        virtual void sendFrameCallbacks(UInt32 time) override;
        virtual void logInfos(RendererLogContext& context) const override;
        virtual void bufferDestroyed(IWaylandBuffer& buffer) override;
        virtual void resourceDestroyed() override;
        virtual void surfaceAttach(IWaylandClient& client, WaylandBufferResource& bufferResource, int x, int y) override;
        virtual void surfaceDetach(IWaylandClient& client) override;
        virtual void surfaceDamage(IWaylandClient& client, int x, int y, int width, int height) override;
        virtual void surfaceFrame(IWaylandClient& client, uint32_t id) override;
        virtual void surfaceSetOpaqueRegion(IWaylandClient& client, IWaylandResource* regionResource) override;
        virtual void surfaceSetInputRegion(IWaylandClient& client, IWaylandResource* regionResource) override;
        virtual void surfaceCommit(IWaylandClient& client) override;
        virtual void surfaceSetBufferTransform(IWaylandClient& client, int32_t transform) override;
        virtual void surfaceSetBufferScale(IWaylandClient& client, int32_t scale) override;
        virtual void surfaceDamageBuffer(IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height) override;

    private:
        void setBufferToSurface(IWaylandBuffer& buffer);
        void unsetBufferFromSurface();
        void setWaylandBuffer(IWaylandBuffer* buffer);

        static void SurfaceDestroyCallback(wl_client* client, wl_resource* surfaceResource);
        static void SurfaceAttachCallback(wl_client*, wl_resource* surfaceResource, wl_resource* bufferResource, int x, int y);
        static void SurfaceDamageCallback(wl_client* client, wl_resource* surfaceResource, int x, int y, int width, int height);
        static void SurfaceFrameCallback(wl_client* client, wl_resource* surfaceResource, uint32_t id);
        static void SurfaceSetOpaqueRegionCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* regionResource);
        static void SurfaceSetInputRegionCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* regionResource);
        static void SurfaceCommitCallback(wl_client* client, wl_resource* surfaceResource);
        static void SurfaceSetBufferTransformCallback(wl_client* client, wl_resource* surfaceResource, int32_t transform);
        static void SurfaceSetBufferScaleCallback(wl_client* client, wl_resource* surfaceResource, int32_t scale);
        static void SurfaceDamageBufferCallback(wl_client *client, wl_resource* surfaceResource, int32_t x, int32_t y, int32_t width, int32_t height);

        static void ResourceDestroyedCallback(wl_resource* surfaceResource);

        IWaylandResource* m_surfaceResource = nullptr;
        std::vector<WaylandCallbackResource*> m_pendingCallbacks;
        std::vector<WaylandCallbackResource*> m_frameCallbacks;
        IWaylandBuffer* m_pendingBuffer = nullptr;
        IWaylandBuffer* m_buffer = nullptr;
        IWaylandIVISurface* m_iviSurface = nullptr;
        IWaylandShellSurface* m_shellSurface = nullptr;
        bool m_removeBufferOnNextCommit = false;
        IEmbeddedCompositor_Wayland& m_compositor;
        UInt64 m_numberOfCommitedFrames = 0;
        UInt64 m_numberOfCommitedFramesSinceBeginningOfTime = 0;

        const struct Surface_Interface : private wl_surface_interface
        {
            Surface_Interface()
            {
                destroy              = SurfaceDestroyCallback;
                attach               = SurfaceAttachCallback;
                damage               = SurfaceDamageCallback;
                frame                = SurfaceFrameCallback;
                set_opaque_region    = SurfaceSetOpaqueRegionCallback;
                set_input_region     = SurfaceSetInputRegionCallback;
                commit               = SurfaceCommitCallback;
                set_buffer_transform = SurfaceSetBufferTransformCallback;
                set_buffer_scale     = SurfaceSetBufferScaleCallback;
#ifdef WL_SURFACE_DAMAGE_BUFFER_SINCE_VERSION
                damage_buffer        = SurfaceDamageBufferCallback;
#endif
            }
        } m_surfaceInterface;
    };
}

#endif
