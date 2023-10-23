//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandClient.h"
#include "internal/RendererLib/Types.h"

#include <string>

namespace ramses::internal
{
    class IWaylandShellSurface;
    class WaylandBufferResource;
    class IWaylandBuffer;
    class IWaylandIVISurface;
    class RendererLogContext;
    class WaylandEGLExtensionProcs;

    class IWaylandSurface
    {
    public:
        virtual ~IWaylandSurface() = default;

        virtual void resourceDestroyed() = 0;
        virtual void surfaceAttach(IWaylandClient& client, WaylandBufferResource& bufferResource, int x, int y) = 0;
        virtual void surfaceDetach(IWaylandClient& client) = 0;
        virtual void surfaceDamage(IWaylandClient& client, int x, int y, int width, int height) = 0;
        virtual void surfaceFrame(IWaylandClient& client, uint32_t id) = 0;
        virtual void surfaceSetOpaqueRegion(IWaylandClient& client, INativeWaylandResource* regionResource) = 0;
        virtual void surfaceSetInputRegion(IWaylandClient& client, INativeWaylandResource* regionResource) = 0;
        virtual void surfaceCommit(IWaylandClient& client) = 0;
        virtual void surfaceSetBufferTransform(IWaylandClient& client, int32_t transform) = 0;
        virtual void surfaceSetBufferScale(IWaylandClient& client, int32_t scale) = 0;
        virtual void surfaceDamageBuffer(IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height) = 0;
        virtual void setShellSurface(IWaylandShellSurface* shellSurface) = 0;
        [[nodiscard]] virtual bool hasShellSurface() const = 0;
        virtual void logInfos(RendererLogContext& context, const WaylandEGLExtensionProcs& eglExt) const = 0;
        [[nodiscard]] virtual WaylandIviSurfaceId getIviSurfaceId() const = 0;
        virtual void sendFrameCallbacks(uint32_t time) = 0;
        [[nodiscard]] virtual IWaylandBuffer* getWaylandBuffer() const = 0;
        [[nodiscard]] virtual uint32_t getNumberOfCommitedFrames() const = 0;
        virtual void resetNumberOfCommitedFrames() = 0;
        [[nodiscard]] virtual uint64_t getNumberOfCommitedFramesSinceBeginningOfTime() const = 0;
        [[nodiscard]] virtual bool hasPendingBuffer() const = 0;
        [[nodiscard]] virtual const std::string& getSurfaceTitle() const = 0;
        virtual void bufferDestroyed(IWaylandBuffer& buffer) = 0;
        virtual void setIviSurface(IWaylandIVISurface* iviSurface) = 0;
        [[nodiscard]] virtual bool hasIviSurface() const = 0;
        [[nodiscard]] virtual WaylandClientCredentials getClientCredentials() const = 0;
        virtual bool dispatchBufferTypeChanged() = 0;
    };
}
