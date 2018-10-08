//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Dummy/EmbeddedCompositor_Dummy.h"
#include "RendererLib/RendererLogContext.h"
#include "Utils/LogMacros.h"
#include "RendererAPI/ITextureUploadingAdapter.h"
#include "RendererAPI/Types.h"

namespace ramses_internal
{

    EmbeddedCompositor_Dummy::EmbeddedCompositor_Dummy()
    {
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::EmbeddedCompositor_Dummy:  created EmbeddedCompositor_Dummy")
    }

    EmbeddedCompositor_Dummy::~EmbeddedCompositor_Dummy()
    {
    }

    Bool EmbeddedCompositor_Dummy::init()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::EmbeddedCompositor_Dummy:");
                return true;
    }

    void EmbeddedCompositor_Dummy::endFrame(Bool)
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::endFrame");
    }

    UInt32 EmbeddedCompositor_Dummy::uploadCompositingContentForStreamTexture(StreamTextureSourceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter)
    {
        UNUSED(textureUploadingAdapter)
        UNUSED(textureHandle)
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::uploadCompositingContentForStreamTexture: " << streamTextureSourceId.getValue());
        return 0;
    }

    StreamTextureSourceIdSet EmbeddedCompositor_Dummy::dispatchUpdatedStreamTextureSourceIds()
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::dispatchUpdatedStreamTextureSourceIds:");
        return StreamTextureSourceIdSet();
    }

    Bool EmbeddedCompositor_Dummy::isContentAvailableForStreamTexture(StreamTextureSourceId streamTextureSourceId) const
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::isContentAvailableForStreamTexture: streamTextureSourceId: " << streamTextureSourceId.getValue());
        return false;
    }

    UInt64 EmbeddedCompositor_Dummy::getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime: waylandSurfaceId: " << waylandSurfaceId.getValue());
        return 0;
    }

    Bool EmbeddedCompositor_Dummy::isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const
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

    Bool EmbeddedCompositor_Dummy::hasSurfaceForStreamTexture(StreamTextureSourceId streamTextureSourceId) const
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::hasSurfaceForStreamTexture: streamTextureSourceId: " << streamTextureSourceId.getValue());
        return false;
    }

    String EmbeddedCompositor_Dummy::getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::getTitleOfWaylandIviSurface: streamTextureSourceId: " << waylandSurfaceId.getValue());
        return String();
    }

    void EmbeddedCompositor_Dummy::getOutputResolution(Int32& width, Int32& height) const
    {
        UNUSED(width)
        UNUSED(height)
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::getOutputResolution");
    }

    void EmbeddedCompositor_Dummy::logInfos(RendererLogContext& context) const
    {
        context << "[EmbeddedCompositor_Dummy]" << RendererLogContext::NewLine;
        context << "No embedded compositor information available." << RendererLogContext::NewLine;
    }

    Bool EmbeddedCompositor_Dummy::isRealCompositor() const
    {
        return false;
    }

    void EmbeddedCompositor_Dummy::handleRequestsFromClients()
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::handleRequestsFromClients");
    }

    Bool EmbeddedCompositor_Dummy::hasUpdatedStreamTextureSources() const
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Dummy::hasUpdatedStreamTextureSources");
        return false;
    }

    StreamTextureSourceIdSet EmbeddedCompositor_Dummy::dispatchNewStreamTextureSourceIds()
    {
        return {};
    }

    StreamTextureSourceIdSet EmbeddedCompositor_Dummy::dispatchObsoleteStreamTextureSourceIds()
    {
        return {};
    }
}
