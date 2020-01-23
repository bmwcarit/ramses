//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestRenderer.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "RendererAPI/IRenderBackend.h"
#include "RamsesRendererImpl.h"
#include "RendererTestEventHandler.h"
#include "ramses-renderer-api/WarpingMeshData.h"
#include "RendererLib/FrameProfileRenderer.h"

namespace ramses_internal
{
    ramses::displayId_t TestRenderer::createDisplay(const ramses::DisplayConfig& displayConfig)
    {
        assert(nullptr != m_renderer);
        return RendererTestUtils::CreateDisplayImmediate(*m_renderer, displayConfig);
    }

    void TestRenderer::initializeRendererWithFramework(ramses::RamsesFramework& ramsesFramework, const ramses::RendererConfig& rendererConfig)
    {
        assert(nullptr == m_renderer);
        m_renderer = ramsesFramework.createRenderer(rendererConfig);
    }

    bool TestRenderer::isRendererInitialized() const
    {
        return nullptr != m_renderer;
    }

    void TestRenderer::destroyRendererWithFramework(ramses::RamsesFramework& ramsesFramework)
    {
        if (m_renderer)
        {
            ramsesFramework.destroyRenderer(*m_renderer);
            m_renderer = nullptr;
        }
    }

    void TestRenderer::flushRenderer()
    {
        assert(nullptr != m_renderer);
        m_renderer->flush();
    }

    void TestRenderer::destroyDisplay(ramses::displayId_t displayId)
    {
        assert(nullptr != m_renderer);
        RendererTestUtils::DestroyDisplayImmediate(*m_renderer, displayId);
    }

    ramses::displayBufferId_t TestRenderer::getDisplayFramebufferId(ramses::displayId_t displayId) const
    {
        assert(nullptr != m_renderer);
        return m_renderer->getDisplayFramebuffer(displayId);
    }

    void TestRenderer::subscribeScene(ramses::sceneId_t sceneId, bool blockUntilSubscription)
    {
        assert(nullptr != m_renderer);
        m_renderer->subscribeScene(sceneId);
        m_renderer->flush();

        if (blockUntilSubscription)
        {
            waitForSubscription(sceneId);
        }
    }

    void TestRenderer::hideUnmapAndUnsubscribeScene(ramses::sceneId_t sceneId)
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

    void TestRenderer::waitForSubscription(ramses::sceneId_t sceneId)
    {
        assert(nullptr != m_renderer);
        RendererTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForSubscription(sceneId);
    }

    void TestRenderer::unsubscribeScene(ramses::sceneId_t sceneId)
    {
        assert(nullptr != m_renderer);
        m_renderer->unsubscribeScene(sceneId);
        m_renderer->flush();

        RendererTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForUnsubscribed(sceneId);
    }

    void TestRenderer::mapScene(ramses::displayId_t displayId, ramses::sceneId_t sceneId)
    {
        assert(nullptr != m_renderer);
        m_renderer->mapScene(displayId, sceneId);
        m_renderer->flush();

        RendererTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForMapped(sceneId);
    }

    void TestRenderer::unmapScene(ramses::sceneId_t sceneId)
    {
        assert(nullptr != m_renderer);
        m_renderer->unmapScene(sceneId);
        m_renderer->flush();
        waitForUnmapped(sceneId);
    }

    void TestRenderer::waitForUnmapped(ramses::sceneId_t sceneId)
    {
        assert(nullptr != m_renderer);
        RendererTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForUnmapped(sceneId);
    }

    bool TestRenderer::showScene(ramses::sceneId_t sceneId)
    {
        assert(nullptr != m_renderer);
        m_renderer->showScene(sceneId);
        m_renderer->flush();

        RendererTestEventHandler eventHandler(*m_renderer);
        return eventHandler.waitForShown(sceneId);
    }

    void TestRenderer::hideScene(ramses::sceneId_t sceneId)
    {
        assert(nullptr != m_renderer);
        m_renderer->hideScene(sceneId);
        m_renderer->flush();

        RendererTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForHidden(sceneId);
    }

    void TestRenderer::hideAndUnmapScene(ramses::sceneId_t sceneId)
    {
        assert(nullptr != m_renderer);
        m_renderer->hideScene(sceneId);
        m_renderer->unmapScene(sceneId);
        m_renderer->flush();

        RendererTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForHidden(sceneId);
        eventHandler.waitForUnmapped(sceneId);
    }

    void TestRenderer::waitForPublication(ramses::sceneId_t sceneId)
    {
        assert(nullptr != m_renderer);
        RendererTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForPublication(sceneId);
    }

    void TestRenderer::waitForUnpublished(ramses::sceneId_t sceneId)
    {
        assert(nullptr != m_renderer);
        RendererTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForUnpublished(sceneId);
    }

    void TestRenderer::waitForNamedFlush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag)
    {
        assert(nullptr != m_renderer);
        RendererTestEventHandler sceneEventHandler(*m_renderer);
        sceneEventHandler.waitForNamedFlush(sceneId, sceneVersionTag, true);
    }

    bool TestRenderer::consumeEventsAndCheckExpiredScenes(std::initializer_list<ramses::sceneId_t> sceneIds)
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

    bool TestRenderer::consumeEventsAndCheckRecoveredScenes(std::initializer_list<ramses::sceneId_t> sceneIds)
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

    bool TestRenderer::waitForStreamSurfaceAvailabilityChange(ramses::streamSource_t streamSource, bool available)
    {
        assert(nullptr != m_renderer);
        RendererTestEventHandler eventHandler(*m_renderer);
        return eventHandler.waitForStreamSurfaceAvailabilityChange(streamSource, available);
    }

    void TestRenderer::dispatchRendererEvents(ramses::IRendererEventHandler& eventHandler)
    {
        assert(nullptr != m_renderer);
        m_renderer->dispatchEvents(eventHandler);
    }

    void TestRenderer::setLoopMode(ramses::ELoopMode loopMode)
    {
        assert(nullptr != m_renderer);
        m_renderer->setLoopMode(loopMode);
    }

    void TestRenderer::startRendererThread()
    {
        assert(nullptr != m_renderer);
        m_renderer->startThread();
    }

    void TestRenderer::stopRendererThread()
    {
        assert(nullptr != m_renderer);
        m_renderer->stopThread();
    }

    void TestRenderer::doOneLoop()
    {
        assert(nullptr != m_renderer);
        m_renderer->doOneLoop();
    }

    ramses::displayBufferId_t TestRenderer::createOffscreenBuffer(ramses::displayId_t displayId, uint32_t width, uint32_t height, bool interruptible)
    {
        assert(nullptr != m_renderer);
        ramses::displayBufferId_t offscreenBufferId;
        if (interruptible)
            offscreenBufferId = m_renderer->createInterruptibleOffscreenBuffer(displayId, width, height);
        else
            offscreenBufferId = m_renderer->createOffscreenBuffer(displayId, width, height);
        m_renderer->flush();

        RendererTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForOffscreenBufferCreation(offscreenBufferId);

        return offscreenBufferId;
    }

    void TestRenderer::destroyOffscreenBuffer(ramses::displayId_t displayId, ramses::displayBufferId_t buffer)
    {
        assert(nullptr != m_renderer);
        m_renderer->destroyOffscreenBuffer(displayId, buffer);
        m_renderer->flush();

        RendererTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForOffscreenBufferDestruction(buffer);
    }

    void TestRenderer::assignSceneToDisplayBuffer(ramses::sceneId_t sceneId, ramses::displayBufferId_t buffer, int32_t renderOrder)
    {
        assert(nullptr != m_renderer);
        m_renderer->assignSceneToDisplayBuffer(sceneId, buffer, renderOrder);
        m_renderer->flush();
    }

    void TestRenderer::setClearColor(ramses::displayId_t displayId, ramses::displayBufferId_t buffer, const ramses_internal::Vector4& clearColor)
    {
        assert(nullptr != m_renderer);
        m_renderer->setBufferClearColor(displayId, buffer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        m_renderer->flush();
    }

    void TestRenderer::createBufferDataLink(ramses::displayBufferId_t providerBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag)
    {
        assert(nullptr != m_renderer);
        m_renderer->linkOffscreenBufferToSceneData(providerBuffer, consumerScene, consumerTag);
        m_renderer->flush();
    }

    void TestRenderer::createDataLink(ramses::sceneId_t providerScene, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId)
    {
        assert(nullptr != m_renderer);
        m_renderer->linkData(providerScene, providerId, consumerScene, consumerId);
        m_renderer->flush();
    }

    void TestRenderer::removeDataLink(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId)
    {
        assert(nullptr != m_renderer);
        m_renderer->unlinkData(consumerScene, consumerId);
        m_renderer->flush();
    }

    void TestRenderer::updateWarpingMeshData(ramses::displayId_t displayId, const ramses::WarpingMeshData& warpingMeshData)
    {
        assert(nullptr != m_renderer);
        m_renderer->updateWarpingMeshData(displayId, warpingMeshData);
        m_renderer->flush();
    }

    bool TestRenderer::performScreenshotCheck(ramses::displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const String& comparisonImageFile, float maxAveragePercentErrorPerPixel, bool readPixelsTwice)
    {
        assert(nullptr != m_renderer);

        // In some cases (interruptible OB) taking screenshot needs to be delayed to guarantee that the last state was rendered to framebuffer,
        // reading pixels twice before checking result guarantees that.
        if (readPixelsTwice)
            RendererTestUtils::ReadPixelData(*m_renderer, displayId, x, y, width, height);

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

    void TestRenderer::saveScreenshotForDisplay(ramses::displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const String& imageFile)
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

    void TestRenderer::toggleRendererFrameProfiler()
    {
        assert(nullptr != m_renderer);
        FrameProfileRenderer::ForAllFrameProfileRenderer(
            m_renderer->impl.getRenderer().getRenderer(),
            [](FrameProfileRenderer& renderer) { renderer.enable(!renderer.isEnabled()); });
    }

    IEmbeddedCompositingManager& TestRenderer::getEmbeddedCompositorManager(ramses::displayId_t displayId)
    {
        assert(nullptr != m_renderer);
        IDisplayController& displayController = m_renderer->impl.getRenderer().getRenderer().getDisplayController(DisplayHandle(displayId.getValue()));
        return displayController.getEmbeddedCompositingManager();
    }

    void TestRenderer::setSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility)
    {
        assert(nullptr != m_renderer);
        m_renderer->impl.getRenderer().getRenderer().systemCompositorSetIviSurfaceVisibility(surfaceId, visibility);
    }

    void TestRenderer::readPixels(ramses::displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        assert(nullptr != m_renderer);
        m_renderer->readPixels(displayId, x, y, width, height);
    }

    IEmbeddedCompositor& TestRenderer::getEmbeddedCompositor(ramses::displayId_t displayId)
    {
        assert(nullptr != m_renderer);
        return m_renderer->impl.getRenderer().getRenderer().getDisplayController(DisplayHandle(displayId.getValue())).getRenderBackend().getEmbeddedCompositor();
    }

    void TestRenderer::setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForSceneActionsApply, uint64_t limitForOffscreenBufferRender)
    {
        m_renderer->setFrameTimerLimits(0u, limitForClientResourcesUpload, limitForSceneActionsApply, limitForOffscreenBufferRender);
    }

    bool TestRenderer::hasSystemCompositorController() const
    {
        assert(nullptr != m_renderer);
        return m_renderer->impl.getRenderer().getRenderer().hasSystemCompositorController();
    }
}
