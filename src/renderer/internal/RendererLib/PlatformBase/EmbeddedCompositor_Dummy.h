//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/PlatformInterface/IEmbeddedCompositor.h"

namespace ramses::internal
{
    class EmbeddedCompositor_Dummy : public IEmbeddedCompositor
    {
    public:
        EmbeddedCompositor_Dummy();

        void handleRequestsFromClients() override;
        [[nodiscard]] bool hasUpdatedStreamTextureSources() const override;
        WaylandIviSurfaceIdSet dispatchUpdatedStreamTextureSourceIds() override;
        WaylandIviSurfaceIdSet dispatchNewStreamTextureSourceIds() override;
        WaylandIviSurfaceIdSet dispatchObsoleteStreamTextureSourceIds() override;
        void endFrame(bool notifyClients) override;
        uint32_t uploadCompositingContentForStreamTexture(WaylandIviSurfaceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter) override;

        [[nodiscard]] bool isContentAvailableForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;
        [[nodiscard]] uint64_t getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const override;
        [[nodiscard]] bool isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        [[nodiscard]] uint32_t getNumberOfCompositorConnections() const override;
        [[nodiscard]] bool hasSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;
        [[nodiscard]] std::string getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        void logInfos(RendererLogContext& context) const override;
        void logPeriodicInfo(StringOutputStream& sos) const override;

        [[nodiscard]] bool isRealCompositor() const override;
    };
}
