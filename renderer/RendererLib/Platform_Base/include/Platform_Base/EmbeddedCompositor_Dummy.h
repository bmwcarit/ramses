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
    class EmbeddedCompositor_Dummy : public IEmbeddedCompositor
    {
    public:
        EmbeddedCompositor_Dummy();

        Bool init();

        void handleRequestsFromClients() override;
        [[nodiscard]] Bool hasUpdatedStreamTextureSources() const override;
        WaylandIviSurfaceIdSet dispatchUpdatedStreamTextureSourceIds() override;
        WaylandIviSurfaceIdSet dispatchNewStreamTextureSourceIds() override;
        WaylandIviSurfaceIdSet dispatchObsoleteStreamTextureSourceIds() override;
        void endFrame(Bool notifyClients) override;
        UInt32 uploadCompositingContentForStreamTexture(WaylandIviSurfaceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter) override;

        [[nodiscard]] Bool isContentAvailableForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;
        [[nodiscard]] UInt64 getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const override;
        [[nodiscard]] Bool isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        [[nodiscard]] UInt32 getNumberOfCompositorConnections() const override;
        [[nodiscard]] Bool hasSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;
        [[nodiscard]] String getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        void logInfos(RendererLogContext& context) const override;

        [[nodiscard]] Bool isRealCompositor() const override;
    };
}

#endif
