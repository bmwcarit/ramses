//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-renderer-api/RamsesRenderer.h"
#include "RamsesRendererImpl.h"

namespace ramses
{
    RamsesRenderer::RamsesRenderer(RamsesFramework& framework, const RendererConfig& config)
        : StatusObject(*new RamsesRendererImpl(framework, config))
        , impl(static_cast<RamsesRendererImpl&>(StatusObject::impl))
    {
        LOG_HL_RENDERER_API2(LOG_API_VOID, LOG_API_GENERIC_OBJECT_STRING(framework), LOG_API_GENERIC_OBJECT_STRING(config));
    }

    status_t RamsesRenderer::subscribeScene(sceneId_t sceneId)
    {
        const status_t status = impl.subscribeScene(sceneId);
        LOG_HL_RENDERER_API1(status, sceneId);
        return status;
    }

    status_t RamsesRenderer::unsubscribeScene(sceneId_t sceneId)
    {
        const status_t status = impl.unsubscribeScene(sceneId);
        LOG_HL_RENDERER_API1(status, sceneId);
        return status;
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

    status_t RamsesRenderer::linkData(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId)
    {
        const status_t status = impl.linkData(providerScene, providerId, consumerScene, consumerId);
        LOG_HL_RENDERER_API4(status, providerScene, providerId, consumerScene, consumerId);
        return status;
    }

    status_t RamsesRenderer::linkOffscreenBufferToSceneData(offscreenBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const status_t status = impl.linkOffscreenBufferToSceneData(offscreenBufferId, consumerSceneId, consumerDataSlotId);
        LOG_HL_RENDERER_API3(status, offscreenBufferId, consumerSceneId, consumerDataSlotId);
        return status;
    }

    status_t RamsesRenderer::unlinkData(sceneId_t consumerScene, dataConsumerId_t consumerId)
    {
        const status_t status = impl.unlinkData(consumerScene, consumerId);
        LOG_HL_RENDERER_API2(status, consumerScene, consumerId);
        return status;
    }

    status_t RamsesRenderer::dispatchEvents(IRendererEventHandler& rendererEventHandler)
    {
        const status_t status = impl.dispatchEvents(rendererEventHandler);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(rendererEventHandler));
        return status;
    }

    status_t RamsesRenderer::mapScene(displayId_t displayId, sceneId_t sceneId, int32_t sceneRenderOrder)
    {
        const status_t status = impl.mapScene(displayId, sceneId, sceneRenderOrder);
        LOG_HL_RENDERER_API3(status, displayId, sceneId, sceneRenderOrder);
        return status;
    }

    status_t RamsesRenderer::unmapScene(sceneId_t sceneId)
    {
        const status_t status = impl.unmapScene(sceneId);
        LOG_HL_RENDERER_API1(status, sceneId);
        return status;
    }

    status_t RamsesRenderer::showScene(sceneId_t sceneId)
    {
        const status_t status = impl.showScene(sceneId);
        LOG_HL_RENDERER_API1(status, sceneId);
        return status;
    }

    status_t RamsesRenderer::hideScene(sceneId_t sceneId)
    {
        const status_t status = impl.hideScene(sceneId);
        LOG_HL_RENDERER_API1(status, sceneId);
        return status;
    }

    offscreenBufferId_t RamsesRenderer::createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height)
    {
        const offscreenBufferId_t bufferId = impl.createOffscreenBuffer(display, width, height, false);
        LOG_HL_RENDERER_API3(bufferId, display, width, height);
        return bufferId;
    }

    status_t RamsesRenderer::destroyOffscreenBuffer(displayId_t display, offscreenBufferId_t offscreenBuffer)
    {
        const status_t status = impl.destroyOffscreenBuffer(display, offscreenBuffer);
        LOG_HL_RENDERER_API2(status, display, offscreenBuffer);
        return status;
    }

    status_t RamsesRenderer::assignSceneToOffscreenBuffer(sceneId_t sceneId, offscreenBufferId_t offscreenBuffer)
    {
        const status_t status = impl.assignSceneToOffscreenBuffer(sceneId, offscreenBuffer);
        LOG_HL_RENDERER_API2(status, sceneId, offscreenBuffer);
        return status;
    }

    status_t RamsesRenderer::assignSceneToFramebuffer(sceneId_t sceneId)
    {
        const status_t status = impl.assignSceneToFramebuffer(sceneId);
        LOG_HL_RENDERER_API1(status, sceneId);
        return status;
    }

    status_t RamsesRenderer::readPixels(displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        const status_t status = impl.readPixels(displayId, x, y, width, height);
        LOG_HL_RENDERER_API5(status, displayId, x, y, width, height);
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

    status_t RamsesRenderer::setFrameTimerLimits(uint64_t limitForSceneResourcesUpload, uint64_t limitForClientResourcesUpload, uint64_t limitForSceneActionsApply, uint64_t limitForOffscreenBufferRender)
    {
        const status_t status = impl.setFrameTimerLimits(limitForSceneResourcesUpload, limitForClientResourcesUpload, limitForSceneActionsApply, limitForOffscreenBufferRender);
        LOG_HL_RENDERER_API4(status, limitForSceneResourcesUpload, limitForClientResourcesUpload, limitForSceneActionsApply, limitForOffscreenBufferRender);
        return status;
    }

    status_t RamsesRenderer::setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit)
    {
        const status_t status = impl.setPendingFlushLimits(forceApplyFlushLimit, forceUnsubscribeSceneLimit);
        LOG_HL_RENDERER_API2(status, forceApplyFlushLimit, forceUnsubscribeSceneLimit);
        return status;
    }

    offscreenBufferId_t RamsesRenderer::createInterruptibleOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height)
    {
        const offscreenBufferId_t bufferId = impl.createOffscreenBuffer(display, width, height, true);
        LOG_HL_RENDERER_API3(bufferId, display, width, height);
        return bufferId;
    }

    status_t RamsesRenderer::setSkippingOfUnmodifiedBuffers(bool enable)
    {
        const status_t status = impl.setSkippingOfUnmodifiedBuffers(enable);
        LOG_HL_RENDERER_API1(status, enable);
        return status;
    }

}
