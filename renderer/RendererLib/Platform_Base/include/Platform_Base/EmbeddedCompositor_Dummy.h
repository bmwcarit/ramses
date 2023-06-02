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

        bool init();

        void handleRequestsFromClients() override;
        [[nodiscard]] bool hasUpdatedStreamTextureSources() const override;
        WaylandIviSurfaceIdSet dispatchUpdatedStreamTextureSourceIds() override;
        WaylandIviSurfaceIdSet dispatchNewStreamTextureSourceIds() override;
        WaylandIviSurfaceIdSet dispatchObsoleteStreamTextureSourceIds() override;
        void endFrame(bool notifyClients) override;
        UInt32 uploadCompositingContentForStreamTexture(WaylandIviSurfaceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter) override;

        [[nodiscard]] bool isContentAvailableForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;
        [[nodiscard]] uint64_t getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const override;
        [[nodiscard]] bool isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        [[nodiscard]] UInt32 getNumberOfCompositorConnections() const override;
        [[nodiscard]] bool hasSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;
        [[nodiscard]] std::string getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        void logInfos(RendererLogContext& context) const override;

        [[nodiscard]] bool isRealCompositor() const override;
    };
}

#endif
