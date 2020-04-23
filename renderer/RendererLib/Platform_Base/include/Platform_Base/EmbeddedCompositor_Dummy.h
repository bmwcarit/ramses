//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMBEDDEDCOMPOSITOR_DUMMY_H
#define RAMSES_EMBEDDEDCOMPOSITOR_DUMMY_H

#include "RendererAPI/IEmbeddedCompositor.h"

namespace ramses_internal
{
    class EmbeddedCompositor_Dummy: public IEmbeddedCompositor
    {
    public:
        EmbeddedCompositor_Dummy();
        virtual ~EmbeddedCompositor_Dummy();

        Bool init();

        virtual void handleRequestsFromClients() override;
        virtual Bool hasUpdatedStreamTextureSources() const override;
        virtual StreamTextureSourceIdSet dispatchUpdatedStreamTextureSourceIds() override;
        virtual StreamTextureSourceIdSet dispatchNewStreamTextureSourceIds() override;
        virtual StreamTextureSourceIdSet dispatchObsoleteStreamTextureSourceIds() override;
        virtual void endFrame(Bool notifyClients) override;
        virtual UInt32 uploadCompositingContentForStreamTexture(StreamTextureSourceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter) override;

        virtual Bool isContentAvailableForStreamTexture(StreamTextureSourceId streamTextureSourceId) const override;
        virtual UInt64 getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const override;
        virtual Bool isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        virtual UInt32 getNumberOfCompositorConnections() const override;
        virtual Bool hasSurfaceForStreamTexture(StreamTextureSourceId streamTextureSourceId) const override;
        virtual String getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        virtual void logInfos(RendererLogContext& context) const override;

        virtual Bool isRealCompositor() const override;
    };
}

#endif
