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
#include "EmbeddedCompositor_Wayland/IWaylandClient.h"
#include "wayland-server.h"

namespace ramses_internal
{
    class IEmbeddedCompositor_Wayland;
    class IWaylandIVISurface;
    class IWaylandShellSurface;
    class WaylandCallbackResource;
    class IWaylandBuffer;

    class WaylandSurface: public IWaylandSurface
    {
    public:
        WaylandSurface(IEmbeddedCompositor_Wayland& compositor, IWaylandClient& client, int version, uint32_t id);
        ~WaylandSurface() override;

        [[nodiscard]] IWaylandBuffer* getWaylandBuffer() const override;
        [[nodiscard]] bool hasPendingBuffer() const override;
        void setShellSurface(IWaylandShellSurface* shellSurface) override;
        [[nodiscard]] bool hasShellSurface() const override;
        [[nodiscard]] const std::string& getSurfaceTitle() const override;
        void setIviSurface(IWaylandIVISurface* iviSurface) override;
        [[nodiscard]] bool hasIviSurface() const override;
        [[nodiscard]] WaylandIviSurfaceId getIviSurfaceId() const override;
        [[nodiscard]] uint32_t getNumberOfCommitedFrames() const override;
        void resetNumberOfCommitedFrames() override;
        [[nodiscard]] uint64_t getNumberOfCommitedFramesSinceBeginningOfTime() const override;
        void sendFrameCallbacks(uint32_t time) override;
        void logInfos(RendererLogContext& context) const override;
        void bufferDestroyed(IWaylandBuffer& buffer) override;
        void resourceDestroyed() override;
        void surfaceAttach(IWaylandClient& client, WaylandBufferResource& bufferResource, int x, int y) override;
        void surfaceDetach(IWaylandClient& client) override;
        void surfaceDamage(IWaylandClient& client, int x, int y, int width, int height) override;
        void surfaceFrame(IWaylandClient& client, uint32_t id) override;
        void surfaceSetOpaqueRegion(IWaylandClient& client, INativeWaylandResource* regionResource) override;
        void surfaceSetInputRegion(IWaylandClient& client, INativeWaylandResource* regionResource) override;
        void surfaceCommit(IWaylandClient& client) override;
        void surfaceSetBufferTransform(IWaylandClient& client, int32_t transform) override;
        void surfaceSetBufferScale(IWaylandClient& client, int32_t scale) override;
        void surfaceDamageBuffer(IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height) override;
        [[nodiscard]] WaylandClientCredentials getClientCredentials() const override;
        bool dispatchBufferTypeChanged() override;

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

        INativeWaylandResource* m_surfaceResource = nullptr;
        std::vector<WaylandCallbackResource*> m_pendingCallbacks;
        std::vector<WaylandCallbackResource*> m_frameCallbacks;
        IWaylandBuffer* m_pendingBuffer = nullptr;
        IWaylandBuffer* m_buffer = nullptr;
        IWaylandIVISurface* m_iviSurface = nullptr;
        IWaylandShellSurface* m_shellSurface = nullptr;
        bool m_removeBufferOnNextCommit = false;
        IEmbeddedCompositor_Wayland& m_compositor;
        uint32_t m_numberOfCommitedFrames = 0;
        uint64_t m_numberOfCommitedFramesSinceBeginningOfTime = 0;
        const WaylandClientCredentials m_clientCredentials;

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

        bool m_bufferTypeChanged = false;
    };
}

#endif
