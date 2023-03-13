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

    ramses::status_t RamsesRenderer::setFramerateLimit(displayId_t displayId, float fpsLimit)
    {
        const status_t status = impl.setFramerateLimit(displayId, fpsLimit);
        LOG_HL_RENDERER_API2(status, displayId, fpsLimit);
        return status;
    }

    float RamsesRenderer::getFramerateLimit(displayId_t displayId) const
    {
        return impl.getFramerateLimit(displayId);
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

    status_t RamsesRenderer::dispatchEvents(IRendererEventHandler& rendererEventHandler)
    {
        const status_t status = impl.dispatchEvents(rendererEventHandler);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(rendererEventHandler));
        return status;
    }

    status_t RamsesRenderer::setExternallyOwnedWindowSize(displayId_t display, uint32_t width, uint32_t height)
    {
        const status_t status = impl.setExternallyOwnedWindowSize(display, width, height);
        LOG_HL_RENDERER_API3(status, display, width, height);
        return status;
    }

    displayBufferId_t RamsesRenderer::createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, uint32_t sampleCount, EDepthBufferType depthBufferType)
    {
        const displayBufferId_t bufferId = impl.createOffscreenBuffer(display, width, height, sampleCount, false, depthBufferType);
        LOG_HL_RENDERER_API5(bufferId, display, width, height, sampleCount, depthBufferType);
        return bufferId;
    }

    status_t RamsesRenderer::destroyOffscreenBuffer(displayId_t display, displayBufferId_t offscreenBuffer)
    {
        const status_t status = impl.destroyOffscreenBuffer(display, offscreenBuffer);
        LOG_HL_RENDERER_API2(status, display, offscreenBuffer);
        return status;
    }

    externalBufferId_t RamsesRenderer::createExternalBuffer(displayId_t display)
    {
        const auto bufferId = impl.createExternalBuffer(display);
        LOG_HL_RENDERER_API1(bufferId, display);
        return bufferId;
    }

    status_t RamsesRenderer::destroyExternalBuffer(displayId_t display, externalBufferId_t externalBuffer)
    {
        const auto status = impl.destroyExternalBuffer(display, externalBuffer);
        LOG_HL_RENDERER_API2(status, display, externalBuffer);
        return status;
    }

    streamBufferId_t RamsesRenderer::createStreamBuffer(displayId_t display, ramses::waylandIviSurfaceId_t surfaceId)
    {
        return impl.createStreamBuffer(display, surfaceId);
    }

    status_t RamsesRenderer::destroyStreamBuffer(displayId_t display, ramses::streamBufferId_t bufferId)
    {
        return impl.destroyStreamBuffer(display, bufferId);
    }

    status_t RamsesRenderer::setDisplayBufferClearFlags(displayId_t display, displayBufferId_t displayBuffer, uint32_t clearFlags)
    {
        const status_t status = impl.setDisplayBufferClearFlags(display, displayBuffer, clearFlags);
        LOG_HL_RENDERER_API3(status, display, displayBuffer, clearFlags);
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

    displayBufferId_t RamsesRenderer::createInterruptibleOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, EDepthBufferType depthBufferType)
    {
        const auto bufferId = impl.createOffscreenBuffer(display, width, height, 0u, true, depthBufferType);
        LOG_HL_RENDERER_API4(bufferId, display, width, height, depthBufferType);
        return bufferId;
    }

    displayBufferId_t RamsesRenderer::createDmaOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, uint32_t bufferFourccFormat, uint32_t usageFlags, uint64_t modifier)
    {
        ramses_internal::DmaBufferFourccFormat bufferFormat { bufferFourccFormat };
        ramses_internal::DmaBufferUsageFlags bufferUsage { usageFlags };
        ramses_internal::DmaBufferModifiers bufferModifer { modifier };
        const auto bufferId = impl.createDmaOffscreenBuffer(display, width, height, bufferFormat, bufferUsage, bufferModifer);
        LOG_HL_RENDERER_API6(bufferId, display, width, height, bufferFourccFormat, usageFlags, modifier);
        return bufferId;
    }

    status_t RamsesRenderer::getDmaOffscreenBufferFDAndStride(displayId_t display, displayBufferId_t displayBufferId, int& fd, uint32_t& stride) const
    {
        return impl.getDmaOffscreenBufferFDAndStride(display, displayBufferId, fd, stride);
    }

    status_t RamsesRenderer::setSkippingOfUnmodifiedBuffers(bool enable)
    {
        const status_t status = impl.setSkippingOfUnmodifiedBuffers(enable);
        LOG_HL_RENDERER_API1(status, enable);
        return status;
    }
}
