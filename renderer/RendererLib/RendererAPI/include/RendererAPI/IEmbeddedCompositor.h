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

        virtual ~IEmbeddedCompositor()
        {
        }

        virtual void handleRequestsFromClients() = 0;
        virtual Bool hasUpdatedStreamTextureSources() const = 0;
        virtual StreamTextureSourceIdSet dispatchUpdatedStreamTextureSourceIds() = 0;
        virtual StreamTextureSourceIdSet dispatchNewStreamTextureSourceIds() = 0;
        virtual StreamTextureSourceIdSet dispatchObsoleteStreamTextureSourceIds() = 0;
        virtual void endFrame(Bool notifyClients) = 0;
        virtual UInt32 uploadCompositingContentForStreamTexture(StreamTextureSourceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter) = 0;

        virtual Bool isContentAvailableForStreamTexture(StreamTextureSourceId streamTextureSourceId) const = 0;
        virtual UInt64 getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const = 0;
        virtual Bool isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const = 0;
        virtual UInt32 getNumberOfCompositorConnections() const = 0;
        virtual Bool hasSurfaceForStreamTexture(StreamTextureSourceId streamTextureSourceId) const = 0;
        virtual String getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const = 0;
        virtual void logInfos(RendererLogContext& context) const = 0;

        virtual Bool isRealCompositor() const = 0; //TODO Mohamed: remove this when dummy EC is removed
    };
}

#endif
