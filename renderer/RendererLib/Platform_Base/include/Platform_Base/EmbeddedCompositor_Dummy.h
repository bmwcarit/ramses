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
#include "SceneAPI/WaylandIviSurfaceId.h"

namespace ramses_internal
{
    class EmbeddedCompositor_Dummy : public IEmbeddedCompositor
    {
    public:
        EmbeddedCompositor_Dummy();

        Bool init();

        virtual void handleRequestsFromClients() override;
        [[nodiscard]] virtual Bool hasUpdatedStreamTextureSources() const override;
        virtual WaylandIviSurfaceIdSet dispatchUpdatedStreamTextureSourceIds() override;
        virtual WaylandIviSurfaceIdSet dispatchNewStreamTextureSourceIds() override;
        virtual WaylandIviSurfaceIdSet dispatchObsoleteStreamTextureSourceIds() override;
        virtual void endFrame(Bool notifyClients) override;
        virtual UInt32 uploadCompositingContentForStreamTexture(WaylandIviSurfaceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter) override;

        [[nodiscard]] virtual Bool isContentAvailableForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;
        [[nodiscard]] virtual UInt64 getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const override;
        [[nodiscard]] virtual Bool isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        [[nodiscard]] virtual UInt32 getNumberOfCompositorConnections() const override;
        [[nodiscard]] virtual Bool hasSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;
        [[nodiscard]] virtual String getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        virtual void logInfos(RendererLogContext& context) const override;

        [[nodiscard]] virtual Bool isRealCompositor() const override;
    };
}

#endif
