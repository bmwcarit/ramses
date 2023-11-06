//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/PlatformBase/EmbeddedCompositor_Dummy.h"
#include "internal/RendererLib/RendererLogContext.h"
#include "internal/RendererLib/PlatformInterface/ITextureUploadingAdapter.h"
#include "internal/RendererLib/Types.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    EmbeddedCompositor_Dummy::EmbeddedCompositor_Dummy()
    {
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::EmbeddedCompositor_Dummy:  created EmbeddedCompositor_Dummy");
    }

    void EmbeddedCompositor_Dummy::endFrame(bool /*notifyClients*/)
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::endFrame");
    }

    uint32_t EmbeddedCompositor_Dummy::uploadCompositingContentForStreamTexture(WaylandIviSurfaceId streamTextureSourceId, [[maybe_unused]] DeviceResourceHandle textureHandle, [[maybe_unused]] ITextureUploadingAdapter& textureUploadingAdapter)
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::uploadCompositingContentForStreamTexture: " << streamTextureSourceId.getValue());
        return 0;
    }

    WaylandIviSurfaceIdSet EmbeddedCompositor_Dummy::dispatchUpdatedStreamTextureSourceIds()
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::dispatchUpdatedStreamTextureSourceIds:");
        return {};
    }

    bool EmbeddedCompositor_Dummy::isContentAvailableForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::isContentAvailableForStreamTexture: streamTextureSourceId: " << streamTextureSourceId.getValue());
        return false;
    }

    uint64_t EmbeddedCompositor_Dummy::getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime: waylandSurfaceId: " << waylandSurfaceId.getValue());
        return 0;
    }

    bool EmbeddedCompositor_Dummy::isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const
    {
        LOG_TRACE(CONTEXT_RENDERER,
                  "EmbeddedCompositor_Dummy::isBufferAttachedToWaylandIviSurface: waylandSurfaceId: "
                      << waylandSurfaceId.getValue());
        return false;
    }

    uint32_t EmbeddedCompositor_Dummy::getNumberOfCompositorConnections() const
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::getNumberOfCompositorConnections");
        return 0;
    }

    bool EmbeddedCompositor_Dummy::hasSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::hasSurfaceForStreamTexture: streamTextureSourceId: " << streamTextureSourceId.getValue());
        return false;
    }

    std::string EmbeddedCompositor_Dummy::getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::getTitleOfWaylandIviSurface: streamTextureSourceId: " << waylandSurfaceId.getValue());
        return {};
    }

    void EmbeddedCompositor_Dummy::logInfos(RendererLogContext& context) const
    {
        context << "[EmbeddedCompositor_Dummy]" << RendererLogContext::NewLine;
        context << "No embedded compositor information available." << RendererLogContext::NewLine;
    }

    void EmbeddedCompositor_Dummy::logPeriodicInfo(StringOutputStream& /*sos*/) const
    {
    }

    bool EmbeddedCompositor_Dummy::isRealCompositor() const
    {
        return false;
    }

    void EmbeddedCompositor_Dummy::handleRequestsFromClients()
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::handleRequestsFromClients");
    }

    bool EmbeddedCompositor_Dummy::hasUpdatedStreamTextureSources() const
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::hasUpdatedStreamTextureSources");
        return false;
    }

    WaylandIviSurfaceIdSet EmbeddedCompositor_Dummy::dispatchNewStreamTextureSourceIds()
    {
        return {};
    }

    WaylandIviSurfaceIdSet EmbeddedCompositor_Dummy::dispatchObsoleteStreamTextureSourceIds()
    {
        return {};
    }
}
