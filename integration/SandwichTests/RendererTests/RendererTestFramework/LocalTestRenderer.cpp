//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "LocalTestRenderer.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "RendererAPI/IRenderBackend.h"
#include "RamsesRendererImpl.h"
#include "RendererTestEventHandler.h"
#include "ramses-renderer-api/WarpingMeshData.h"
#include "RendererLib/FrameProfileRenderer.h"

ramses::displayId_t LocalTestRenderer::createDisplay(const ramses::DisplayConfig& displayConfig)
{
    assert(nullptr != m_renderer);
    return RendererTestUtils::CreateDisplayImmediate(*m_renderer, displayConfig);
}

void LocalTestRenderer::initializeRendererWithFramework(ramses::RamsesFramework& ramsesFramework, const ramses::RendererConfig& rendererConfig)
{
    assert(nullptr == m_renderer);
    m_renderer.reset(new ramses::RamsesRenderer(ramsesFramework, rendererConfig));
}

bool LocalTestRenderer::isRendererInitialized() const
{
    return nullptr != m_renderer;
}

void LocalTestRenderer::destroyRenderer()
{
    m_renderer.reset(nullptr);
}

void LocalTestRenderer::flushRenderer()
{
    assert(nullptr != m_renderer);
    m_renderer->flush();
}

void LocalTestRenderer::destroyDisplay(ramses::displayId_t displayId)
{
    assert(nullptr != m_renderer);
    RendererTestUtils::DestroyDisplayImmediate(*m_renderer, displayId);
}

void LocalTestRenderer::subscribeScene(ramses::sceneId_t sceneId, bool blockUntilSubscription)
{
    assert(nullptr != m_renderer);
    m_renderer->subscribeScene(sceneId);
    m_renderer->flush();

    if (blockUntilSubscription)
    {
        waitForSubscription(sceneId);
    }
}

void LocalTestRenderer::hideUnmapAndUnsubscribeScene(ramses::sceneId_t sceneId)
{
    assert(nullptr != m_renderer);
    RendererTestEventHandler eventHandler(*m_renderer);

    m_renderer->hideScene(sceneId);
    m_renderer->flush();
    eventHandler.waitForHidden(sceneId);

    m_renderer->unmapScene(sceneId);
    m_renderer->flush();
    eventHandler.waitForUnmapped(sceneId);

    m_renderer->unsubscribeScene(sceneId);
    m_renderer->flush();
    eventHandler.waitForUnsubscribed(sceneId);
}

void LocalTestRenderer::waitForSubscription(ramses::sceneId_t sceneId)
{
    assert(nullptr != m_renderer);
    RendererTestEventHandler eventHandler(*m_renderer);
    eventHandler.waitForSubscription(sceneId);
}

void LocalTestRenderer::unsubscribeScene(ramses::sceneId_t sceneId)
{
    assert(nullptr != m_renderer);
    m_renderer->unsubscribeScene(sceneId);
    m_renderer->flush();

    RendererTestEventHandler eventHandler(*m_renderer);
    eventHandler.waitForUnsubscribed(sceneId);
}

void LocalTestRenderer::mapScene(ramses::displayId_t displayId, ramses::sceneId_t sceneId, int32_t sceneRenderOrder)
{
    assert(nullptr != m_renderer);
    m_renderer->mapScene(displayId, sceneId, sceneRenderOrder);
    m_renderer->flush();

    RendererTestEventHandler eventHandler(*m_renderer);
    eventHandler.waitForMapped(sceneId);
}

void LocalTestRenderer::unmapScene(ramses::sceneId_t sceneId)
{
    assert(nullptr != m_renderer);
    m_renderer->unmapScene(sceneId);
    m_renderer->flush();
    waitForUnmapped(sceneId);
}

void LocalTestRenderer::waitForUnmapped(ramses::sceneId_t sceneId)
{
    assert(nullptr != m_renderer);
    RendererTestEventHandler eventHandler(*m_renderer);
    eventHandler.waitForUnmapped(sceneId);
}

bool LocalTestRenderer::showScene(ramses::sceneId_t sceneId)
{
    assert(nullptr != m_renderer);
    m_renderer->showScene(sceneId);
    m_renderer->flush();

    RendererTestEventHandler eventHandler(*m_renderer);
    return eventHandler.waitForShown(sceneId);
}

void LocalTestRenderer::hideScene(ramses::sceneId_t sceneId)
{
    assert(nullptr != m_renderer);
    m_renderer->hideScene(sceneId);
    m_renderer->flush();

    RendererTestEventHandler eventHandler(*m_renderer);
    eventHandler.waitForHidden(sceneId);
}

void LocalTestRenderer::hideAndUnmapScene(ramses::sceneId_t sceneId)
{
    assert(nullptr != m_renderer);
    m_renderer->hideScene(sceneId);
    m_renderer->unmapScene(sceneId);
    m_renderer->flush();

    RendererTestEventHandler eventHandler(*m_renderer);
    eventHandler.waitForHidden(sceneId);
    eventHandler.waitForUnmapped(sceneId);
}

void LocalTestRenderer::waitForPublication(ramses::sceneId_t sceneId)
{
    assert(nullptr != m_renderer);
    RendererTestEventHandler eventHandler(*m_renderer);
    eventHandler.waitForPublication(sceneId);
}

void LocalTestRenderer::waitForUnpublished(ramses::sceneId_t sceneId)
{
    assert(nullptr != m_renderer);
    RendererTestEventHandler eventHandler(*m_renderer);
    eventHandler.waitForUnpublished(sceneId);
}

void LocalTestRenderer::waitForNamedFlush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag)
{
    assert(nullptr != m_renderer);
    RendererTestEventHandler sceneEventHandler(*m_renderer);
    sceneEventHandler.waitForNamedFlush(sceneId, sceneVersionTag, true);
}

bool LocalTestRenderer::consumeEventsAndCheckExpiredScenes(std::initializer_list<ramses::sceneId_t> sceneIds)
{
    assert(nullptr != m_renderer);
    RendererTestEventHandler sceneEventHandler(*m_renderer);
    for (auto sceneId : sceneIds)
    {
        if (!sceneEventHandler.checkAndConsumeExpiredScenesEvents(sceneId))
            return false;
    }

    return true;
}

bool LocalTestRenderer::consumeEventsAndCheckRecoveredScenes(std::initializer_list<ramses::sceneId_t> sceneIds)
{
    assert(nullptr != m_renderer);
    RendererTestEventHandler sceneEventHandler(*m_renderer);
    for (auto sceneId : sceneIds)
    {
        if (!sceneEventHandler.checkAndConsumeRecoveredScenesEvents(sceneId))
            return false;
    }

    return true;
}

bool LocalTestRenderer::waitForStreamSurfaceAvailabilityChange(ramses::streamSource_t streamSource, bool available)
{
    assert(nullptr != m_renderer);
    RendererTestEventHandler eventHandler(*m_renderer);
    return eventHandler.waitForStreamSurfaceAvailabilityChange(streamSource, available);
}

void LocalTestRenderer::dispatchRendererEvents(ramses::IRendererEventHandler& eventHandler)
{
    assert(nullptr != m_renderer);
    m_renderer->dispatchEvents(eventHandler);
}

void LocalTestRenderer::setLoopMode(ramses::ELoopMode loopMode)
{
    assert(nullptr != m_renderer);
    m_renderer->setLoopMode(loopMode);
}

void LocalTestRenderer::startRendererThread()
{
    assert(nullptr != m_renderer);
    m_renderer->startThread();
}

void LocalTestRenderer::stopRendererThread()
{
    assert(nullptr != m_renderer);
    m_renderer->stopThread();
}

void LocalTestRenderer::doOneLoop()
{
    assert(nullptr != m_renderer);
    m_renderer->doOneLoop();
}

ramses::offscreenBufferId_t LocalTestRenderer::createOffscreenBuffer(ramses::displayId_t displayId, uint32_t width, uint32_t height, bool interruptible)
{
    assert(nullptr != m_renderer);
    ramses::offscreenBufferId_t offscreenBufferId;
    if (interruptible)
        offscreenBufferId = m_renderer->createInterruptibleOffscreenBuffer(displayId, width, height);
    else
        offscreenBufferId = m_renderer->createOffscreenBuffer(displayId, width, height);
    m_renderer->flush();

    RendererTestEventHandler eventHandler(*m_renderer);
    eventHandler.waitForOffscreenBufferCreation(offscreenBufferId);

    return offscreenBufferId;
}

void LocalTestRenderer::destroyOffscreenBuffer(ramses::displayId_t displayId, ramses::offscreenBufferId_t buffer)
{
    assert(nullptr != m_renderer);
    m_renderer->destroyOffscreenBuffer(displayId, buffer);
    m_renderer->flush();

    RendererTestEventHandler eventHandler(*m_renderer);
    eventHandler.waitForOffscreenBufferDestruction(buffer);
}

void LocalTestRenderer::assignSceneToOffscreenBuffer(ramses::sceneId_t sceneId, ramses::offscreenBufferId_t buffer)
{
    assert(nullptr != m_renderer);
    m_renderer->assignSceneToOffscreenBuffer(sceneId, buffer);
    m_renderer->flush();
}

void LocalTestRenderer::assignSceneToFramebuffer(ramses::sceneId_t sceneId)
{
    assert(nullptr != m_renderer);
    m_renderer->assignSceneToFramebuffer(sceneId);
    m_renderer->flush();
}

void LocalTestRenderer::createBufferDataLink(ramses::offscreenBufferId_t providerBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag)
{
    assert(nullptr != m_renderer);
    m_renderer->linkOffscreenBufferToSceneData(providerBuffer, consumerScene, consumerTag);
    m_renderer->flush();
}

void LocalTestRenderer::createDataLink(ramses::sceneId_t providerScene, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId)
{
    assert(nullptr != m_renderer);
    m_renderer->linkData(providerScene, providerId, consumerScene, consumerId);
    m_renderer->flush();
}

void LocalTestRenderer::removeDataLink(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId)
{
    assert(nullptr != m_renderer);
    m_renderer->unlinkData(consumerScene, consumerId);
    m_renderer->flush();
}

void LocalTestRenderer::updateWarpingMeshData(ramses::displayId_t displayId, const ramses::WarpingMeshData& warpingMeshData)
{
    assert(nullptr != m_renderer);
    m_renderer->updateWarpingMeshData(displayId, warpingMeshData);
    m_renderer->flush();
}

bool LocalTestRenderer::performScreenshotCheck(ramses::displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const ramses_internal::String& comparisonImageFile, float maxAveragePercentErrorPerPixel)
{
    assert(nullptr != m_renderer);
    return RendererTestUtils::PerformScreenshotTestForDisplay(
        *m_renderer,
        displayId,
        x,
        y,
        width,
        height,
        comparisonImageFile,
        maxAveragePercentErrorPerPixel);
}

void LocalTestRenderer::saveScreenshotForDisplay(ramses::displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const ramses_internal::String& imageFile)
{
    assert(nullptr != m_renderer);
    RendererTestUtils::SaveScreenshotForDisplay(
        *m_renderer,
        displayId,
        x,
        y,
        width,
        height,
        imageFile);
}

void LocalTestRenderer::toggleRendererFrameProfiler()
{
    assert(nullptr != m_renderer);
    ramses_internal::FrameProfileRenderer::ForAllFrameProfileRenderer(
        m_renderer->impl.getRenderer().getRenderer(),
        [](ramses_internal::FrameProfileRenderer& renderer) { renderer.enable(!renderer.isEnabled()); });
}

ramses_internal::IEmbeddedCompositingManager& LocalTestRenderer::getEmbeddedCompositorManager(ramses::displayId_t displayId)
{
    assert(nullptr != m_renderer);
    ramses_internal::IDisplayController& displayController = m_renderer->impl.getRenderer().getRenderer().getDisplayController(ramses_internal::DisplayHandle(displayId));
    return displayController.getEmbeddedCompositingManager();
}

void LocalTestRenderer::setSurfaceVisibility(ramses_internal::WaylandIviSurfaceId surfaceId, bool visibility)
{
    assert(nullptr != m_renderer);
    m_renderer->impl.getRenderer().getRenderer().systemCompositorSetIviSurfaceVisibility(surfaceId, visibility);
}

void LocalTestRenderer::readPixels(ramses::displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{
    assert(nullptr != m_renderer);
    m_renderer->readPixels(displayId, x, y, width, height);
}

ramses_internal::IEmbeddedCompositor& LocalTestRenderer::getEmbeddedCompositor(ramses::displayId_t displayId)
{
    assert(nullptr != m_renderer);
    return m_renderer->impl.getRenderer().getRenderer().getDisplayController(ramses_internal::DisplayHandle(displayId)).getRenderBackend().getEmbeddedCompositor();
}

void LocalTestRenderer::setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForSceneActionsApply, uint64_t limitForOffscreenBufferRender)
{
    m_renderer->setFrameTimerLimits(0u, limitForClientResourcesUpload, limitForSceneActionsApply, limitForOffscreenBufferRender);
}

bool LocalTestRenderer::hasSystemCompositorController() const
{
    assert(nullptr != m_renderer);
    return m_renderer->impl.getRenderer().getRenderer().hasSystemCompositorController();
}
