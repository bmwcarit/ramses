//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Platform_Base/EmbeddedCompositor_Dummy.h"
#include "RendererLib/RendererLogContext.h"
#include "RendererAPI/ITextureUploadingAdapter.h"
#include "RendererAPI/Types.h"
#include "Utils/ThreadLocalLogForced.h"

namespace ramses_internal
{
    EmbeddedCompositor_Dummy::EmbeddedCompositor_Dummy()
    {
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::EmbeddedCompositor_Dummy:  created EmbeddedCompositor_Dummy");
    }

    bool EmbeddedCompositor_Dummy::init()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::EmbeddedCompositor_Dummy:");
                return true;
    }

    void EmbeddedCompositor_Dummy::endFrame(bool)
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::endFrame");
    }

    UInt32 EmbeddedCompositor_Dummy::uploadCompositingContentForStreamTexture(WaylandIviSurfaceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter)
    {
        UNUSED(textureUploadingAdapter);
        UNUSED(textureHandle);
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

    UInt32 EmbeddedCompositor_Dummy::getNumberOfCompositorConnections() const
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
