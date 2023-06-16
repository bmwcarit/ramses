//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IEMBEDDEDCOMPOSITOR_H
#define RAMSES_IEMBEDDEDCOMPOSITOR_H

#include "RendererAPI/Types.h"

namespace ramses_internal
{
    class RendererLogContext;
    class ITextureUploadingAdapter;

    class IEmbeddedCompositor
    {
    public:
        virtual ~IEmbeddedCompositor() = default;

        virtual void handleRequestsFromClients() = 0;
        [[nodiscard]] virtual bool hasUpdatedStreamTextureSources() const = 0;
        virtual WaylandIviSurfaceIdSet dispatchUpdatedStreamTextureSourceIds() = 0;
        virtual WaylandIviSurfaceIdSet dispatchNewStreamTextureSourceIds() = 0;
        virtual WaylandIviSurfaceIdSet dispatchObsoleteStreamTextureSourceIds() = 0;
        virtual void endFrame(bool notifyClients) = 0;
        virtual uint32_t uploadCompositingContentForStreamTexture(WaylandIviSurfaceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter) = 0;

        [[nodiscard]] virtual bool isContentAvailableForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const = 0;
        [[nodiscard]] virtual uint64_t getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const = 0;
        [[nodiscard]] virtual bool isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const = 0;
        [[nodiscard]] virtual uint32_t getNumberOfCompositorConnections() const = 0;
        [[nodiscard]] virtual bool hasSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const = 0;
        [[nodiscard]] virtual std::string getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const = 0;
        virtual void logInfos(RendererLogContext& context) const = 0;

        [[nodiscard]] virtual bool isRealCompositor() const = 0; //TODO Mohamed: remove this when dummy EC is removed
    };
}

#endif
