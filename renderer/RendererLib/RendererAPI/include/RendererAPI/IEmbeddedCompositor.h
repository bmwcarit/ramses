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
#include "SceneAPI/WaylandIviSurfaceId.h"

namespace ramses_internal
{
    class RendererLogContext;
    class ITextureUploadingAdapter;

    class IEmbeddedCompositor
    {
    public:
        virtual ~IEmbeddedCompositor() = default;

        virtual void handleRequestsFromClients() = 0;
        virtual Bool hasUpdatedStreamTextureSources() const = 0;
        virtual WaylandIviSurfaceIdSet dispatchUpdatedStreamTextureSourceIds() = 0;
        virtual WaylandIviSurfaceIdSet dispatchNewStreamTextureSourceIds() = 0;
        virtual WaylandIviSurfaceIdSet dispatchObsoleteStreamTextureSourceIds() = 0;
        virtual void endFrame(Bool notifyClients) = 0;
        virtual UInt32 uploadCompositingContentForStreamTexture(WaylandIviSurfaceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter) = 0;

        virtual Bool isContentAvailableForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const = 0;
        virtual UInt64 getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const = 0;
        virtual Bool isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const = 0;
        virtual UInt32 getNumberOfCompositorConnections() const = 0;
        virtual Bool hasSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const = 0;
        virtual String getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const = 0;
        virtual void logInfos(RendererLogContext& context) const = 0;

        virtual Bool isRealCompositor() const = 0; //TODO Mohamed: remove this when dummy EC is removed
    };
}

#endif
