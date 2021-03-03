//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-renderer-api/RamsesRenderer.h"
#include "RamsesRendererImpl.h"
#include "RamsesFrameworkImpl.h"
#include "RamsesFrameworkTypesImpl.h"

namespace ramses
{
    RamsesRenderer::RamsesRenderer(RamsesRendererImpl& impl_)
        : StatusObject(impl_)
        , impl(impl_)
    {
    }

    RamsesRenderer::~RamsesRenderer()
    {
        LOG_HL_RENDERER_API_NOARG(LOG_API_VOID);
    }

    status_t RamsesRenderer::doOneLoop()
    {
        const status_t status = impl.doOneLoop();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    ramses::status_t RamsesRenderer::startThread()
    {
        const status_t status = impl.startThread();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    ramses::status_t RamsesRenderer::stopThread()
    {
        const status_t status = impl.stopThread();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    bool RamsesRenderer::isThreadRunning() const
    {
        return impl.isThreadRunning();
    }

    ramses::status_t RamsesRenderer::setMaximumFramerate(float maximumFramerate)
    {
        const status_t status = impl.setMaximumFramerate(maximumFramerate);
        LOG_HL_RENDERER_API1(status, maximumFramerate);
        return status;
    }

    ramses::status_t RamsesRenderer::setMaximumFramerate(RamsesRenderer& renderer, float maximumFramerate, displayId_t displayId)
    {
        const status_t status = renderer.impl.setMaximumFramerate(maximumFramerate, displayId);
        LOG_HL_RENDERER_STATIC_API2(status, maximumFramerate, displayId);
        return status;
    }

    float RamsesRenderer::getMaximumFramerate() const
    {
        return impl.getMaximumFramerate();
    }

    ramses::status_t RamsesRenderer::setLoopMode(ELoopMode loopMode)
    {
        const status_t status = impl.setLoopMode(loopMode);
        LOG_HL_RENDERER_API1(status, loopMode);
        return status;
    }

    ELoopMode RamsesRenderer::getLoopMode() const
    {
        return impl.getLoopMode();
    }

    displayId_t RamsesRenderer::createDisplay(const DisplayConfig& config)
    {
        const displayId_t displayId = impl.createDisplay(config);
        LOG_HL_RENDERER_API1(displayId, LOG_API_GENERIC_OBJECT_STRING(config));
        return displayId;
    }

    status_t RamsesRenderer::destroyDisplay(displayId_t displayId)
    {
        const status_t status = impl.destroyDisplay(displayId);
        LOG_HL_RENDERER_API1(status, displayId);
        return status;
    }

    displayBufferId_t RamsesRenderer::getDisplayFramebuffer(displayId_t displayId) const
    {
        return impl.getDisplayFramebuffer(displayId);
    }

    RendererSceneControl* RamsesRenderer::getSceneControlAPI()
    {
        return impl.getSceneControlAPI();
    }

    DcsmContentControl* RamsesRenderer::createDcsmContentControl()
    {
        auto ret = impl.createDcsmContentControl();
        LOG_HL_RENDERER_API_NOARG(LOG_API_GENERIC_PTR_STRING(ret));
        return ret;
    }

    status_t RamsesRenderer::dispatchEvents(IRendererEventHandler& rendererEventHandler)
    {
        const status_t status = impl.dispatchEvents(rendererEventHandler);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(rendererEventHandler));
        return status;
    }

    displayBufferId_t RamsesRenderer::createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, uint32_t sampleCount)
    {
        const displayBufferId_t bufferId = impl.createOffscreenBuffer(display, width, height, sampleCount, false, EDepthBufferType_DepthStencil);
        LOG_HL_RENDERER_API4(bufferId, display, width, height, sampleCount);
        return bufferId;
    }

    displayBufferId_t RamsesRenderer::createOffscreenBuffer(RamsesRenderer& renderer, displayId_t display, uint32_t width, uint32_t height, uint32_t sampleCount, EDepthBufferType depthBufferType)
    {
        const displayBufferId_t bufferId = renderer.impl.createOffscreenBuffer(display, width, height, sampleCount, false, depthBufferType);
        LOG_HL_RENDERER_STATIC_API5(bufferId, display, width, height, sampleCount, depthBufferType);
        return bufferId;
    }

    status_t RamsesRenderer::destroyOffscreenBuffer(displayId_t display, displayBufferId_t offscreenBuffer)
    {
        const status_t status = impl.destroyOffscreenBuffer(display, offscreenBuffer);
        LOG_HL_RENDERER_API2(status, display, offscreenBuffer);
        return status;
    }

    status_t RamsesRenderer::setDisplayBufferClearFlags(RamsesRenderer& renderer, displayId_t display, displayBufferId_t displayBuffer, uint32_t clearFlags)
    {
        const status_t status = renderer.impl.setDisplayBufferClearFlags(display, displayBuffer, clearFlags);
        LOG_HL_RENDERER_STATIC_API3(status, display, displayBuffer, clearFlags);
        return status;
    }

    status_t RamsesRenderer::setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, float r, float g, float b, float a)
    {
        const status_t status = impl.setDisplayBufferClearColor(display, displayBuffer, r, g, b, a);
        LOG_HL_RENDERER_API6(status, display, displayBuffer, r, g, b, a);
        return status;
    }

    status_t RamsesRenderer::readPixels(displayId_t displayId, displayBufferId_t displayBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        const status_t status = impl.readPixels(displayId, displayBuffer, x, y, width, height);
        LOG_HL_RENDERER_API6(status, displayId, displayBuffer, x, y, width, height);
        return status;
    }

    status_t RamsesRenderer::updateWarpingMeshData(displayId_t displayId, const WarpingMeshData& newWarpingMeshData)
    {
        const status_t status = impl.updateWarpingMeshData(displayId, newWarpingMeshData);
        LOG_HL_RENDERER_API2(status, displayId, LOG_API_GENERIC_OBJECT_STRING(newWarpingMeshData));
        return status;
    }

    status_t RamsesRenderer::flush()
    {
        const status_t result = impl.flush();
        LOG_HL_RENDERER_API_NOARG(result);
        return result;
    }

    status_t RamsesRenderer::setSurfaceVisibility(uint32_t surfaceId, bool visibility)
    {
        const status_t status = impl.systemCompositorSetIviSurfaceVisibility(surfaceId, visibility);
        LOG_HL_RENDERER_API2(status, surfaceId, visibility);
        return status;
    }

    status_t RamsesRenderer::setSurfaceOpacity(uint32_t surfaceId, float opacity)
    {
        const status_t status = impl.systemCompositorSetIviSurfaceOpacity(surfaceId, opacity);
        LOG_HL_RENDERER_API2(status, surfaceId, opacity);
        return status;
    }

    status_t RamsesRenderer::setSurfaceRectangle(uint32_t surfaceId, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        const status_t status = impl.systemCompositorSetIviSurfaceRectangle(surfaceId, x, y, width, height);
        LOG_HL_RENDERER_API5(status, surfaceId, x, y, width, height);
        return status;
    }

    status_t RamsesRenderer::setLayerVisibility(uint32_t layerId, bool visibility)
    {
        const status_t status = impl.systemCompositorSetIviLayerVisibility(layerId, visibility);
        LOG_HL_RENDERER_API2(status, layerId, visibility);
        return status;
    }

    status_t RamsesRenderer::takeSystemCompositorScreenshot(const char* fileName, int32_t screenIviId)
    {
        const status_t status = impl.systemCompositorTakeScreenshot(fileName, screenIviId);
        LOG_HL_RENDERER_API2(status, fileName, screenIviId);
        return status;
    }

    status_t RamsesRenderer::logRendererInfo()
    {
        const status_t status = impl.logRendererInfo();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    status_t RamsesRenderer::setFrameTimerLimits(uint64_t limitForSceneResourcesUpload, uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender)
    {
        const status_t status = impl.setFrameTimerLimits(limitForSceneResourcesUpload, limitForClientResourcesUpload, limitForOffscreenBufferRender);
        LOG_HL_RENDERER_API3(status, limitForSceneResourcesUpload, limitForClientResourcesUpload, limitForOffscreenBufferRender);
        return status;
    }

    status_t RamsesRenderer::setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit)
    {
        const status_t status = impl.setPendingFlushLimits(forceApplyFlushLimit, forceUnsubscribeSceneLimit);
        LOG_HL_RENDERER_API2(status, forceApplyFlushLimit, forceUnsubscribeSceneLimit);
        return status;
    }

    displayBufferId_t RamsesRenderer::createInterruptibleOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height)
    {
        const auto bufferId = impl.createOffscreenBuffer(display, width, height, 0u, true, EDepthBufferType_DepthStencil);
        LOG_HL_RENDERER_API3(bufferId, display, width, height);
        return bufferId;
    }

    displayBufferId_t RamsesRenderer::createInterruptibleOffscreenBuffer(RamsesRenderer& renderer, displayId_t display, uint32_t width, uint32_t height, EDepthBufferType depthBufferType)
    {
        const auto bufferId = renderer.impl.createOffscreenBuffer(display, width, height, 0u, true, depthBufferType);
        LOG_HL_RENDERER_STATIC_API4(bufferId, display, width, height, depthBufferType);
        return bufferId;
    }

    status_t RamsesRenderer::setSkippingOfUnmodifiedBuffers(bool enable)
    {
        const status_t status = impl.setSkippingOfUnmodifiedBuffers(enable);
        LOG_HL_RENDERER_API1(status, enable);
        return status;
    }
}
