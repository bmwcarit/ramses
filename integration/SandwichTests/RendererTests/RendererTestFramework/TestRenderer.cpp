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
#include "RendererAndSceneTestEventHandler.h"
#include "ramses-renderer-api/WarpingMeshData.h"
#include "RendererLib/FrameProfileRenderer.h"

namespace ramses_internal
{
    ramses::displayId_t TestRenderer::createDisplay(const ramses::DisplayConfig& displayConfig)
    {
        return RendererTestUtils::CreateDisplayImmediate(*m_renderer, displayConfig);
    }

    void TestRenderer::initializeRendererWithFramework(ramses::RamsesFramework& ramsesFramework, const ramses::RendererConfig& rendererConfig)
    {
        m_renderer = ramsesFramework.createRenderer(rendererConfig);
        assert(m_renderer != nullptr);
        m_sceneControlAPI = m_renderer->getSceneControlAPI();
        assert(m_sceneControlAPI != nullptr);
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
            m_sceneControlAPI = nullptr;
        }
    }

    void TestRenderer::flushRenderer()
    {
        m_renderer->flush();
    }

    void TestRenderer::destroyDisplay(ramses::displayId_t displayId)
    {
        RendererTestUtils::DestroyDisplayImmediate(*m_renderer, displayId);
    }

    ramses::displayBufferId_t TestRenderer::getDisplayFramebufferId(ramses::displayId_t displayId) const
    {
        return m_renderer->getDisplayFramebuffer(displayId);
    }

    void TestRenderer::setSceneMapping(ramses::sceneId_t sceneId, ramses::displayId_t display)
    {
        m_sceneControlAPI->setSceneMapping(sceneId, display);
        m_sceneControlAPI->flush();
    }

    bool TestRenderer::getSceneToState(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        setSceneState(sceneId, state);
        return waitForSceneStateChange(sceneId, state);
    }

    void TestRenderer::setSceneState(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        m_sceneControlAPI->setSceneState(sceneId, state);
        m_sceneControlAPI->flush();
    }

    bool TestRenderer::waitForSceneStateChange(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        ramses::RendererAndSceneTestEventHandler eventHandler(*m_renderer);
        return eventHandler.waitForSceneState(sceneId, state);
    }

    void TestRenderer::waitForFlush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag)
    {
        ramses::RendererAndSceneTestEventHandler sceneEventHandler(*m_renderer);
        sceneEventHandler.waitForFlush(sceneId, sceneVersionTag);
    }

    bool TestRenderer::checkScenesExpired(std::initializer_list<ramses::sceneId_t> sceneIds)
    {
        ramses::RendererAndSceneTestEventHandler sceneEventHandler(*m_renderer);
        for (auto sceneId : sceneIds)
        {
            if (!sceneEventHandler.checkExpiredState(sceneId))
                return false;
        }

        return true;
    }

    bool TestRenderer::checkScenesNotExpired(std::initializer_list<ramses::sceneId_t> sceneIds)
    {
        ramses::RendererAndSceneTestEventHandler sceneEventHandler(*m_renderer);
        for (auto sceneId : sceneIds)
        {
            if (sceneEventHandler.checkExpiredState(sceneId))
                return false;
        }

        return true;
    }

    bool TestRenderer::waitForStreamSurfaceAvailabilityChange(ramses::streamSource_t streamSource, bool available)
    {
        ramses::RendererAndSceneTestEventHandler eventHandler(*m_renderer);
        return eventHandler.waitForStreamSurfaceAvailabilityChange(streamSource, available);
    }

    void TestRenderer::dispatchEvents(ramses::IRendererEventHandler& eventHandler, ramses::IRendererSceneControlEventHandler& sceneControlEventHandler)
    {
        m_renderer->dispatchEvents(eventHandler);
        m_sceneControlAPI->dispatchEvents(sceneControlEventHandler);
    }

    void TestRenderer::setLoopMode(ramses::ELoopMode loopMode)
    {
        m_renderer->setLoopMode(loopMode);
    }

    void TestRenderer::startRendererThread()
    {
        m_renderer->startThread();
    }

    void TestRenderer::stopRendererThread()
    {
        m_renderer->stopThread();
    }

    void TestRenderer::doOneLoop()
    {
        m_renderer->doOneLoop();
    }

    ramses::displayBufferId_t TestRenderer::createOffscreenBuffer(ramses::displayId_t displayId, uint32_t width, uint32_t height, bool interruptible)
    {
        ramses::displayBufferId_t offscreenBufferId;
        if (interruptible)
            offscreenBufferId = m_renderer->createInterruptibleOffscreenBuffer(displayId, width, height);
        else
            offscreenBufferId = m_renderer->createOffscreenBuffer(displayId, width, height);
        m_renderer->flush();

        ramses::RendererAndSceneTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForOffscreenBufferCreation(offscreenBufferId);

        return offscreenBufferId;
    }

    void TestRenderer::destroyOffscreenBuffer(ramses::displayId_t displayId, ramses::displayBufferId_t buffer)
    {
        m_renderer->destroyOffscreenBuffer(displayId, buffer);
        m_renderer->flush();

        ramses::RendererAndSceneTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForOffscreenBufferDestruction(buffer);
    }

    void TestRenderer::assignSceneToDisplayBuffer(ramses::sceneId_t sceneId, ramses::displayBufferId_t buffer, int32_t renderOrder)
    {
        m_sceneControlAPI->setSceneDisplayBufferAssignment(sceneId, buffer, renderOrder);
        m_sceneControlAPI->flush();
    }

    void TestRenderer::setClearColor(ramses::displayId_t displayId, ramses::displayBufferId_t buffer, const ramses_internal::Vector4& clearColor)
    {
        m_sceneControlAPI->setDisplayBufferClearColor(displayId, buffer, clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        m_sceneControlAPI->flush();
    }

    void TestRenderer::createBufferDataLink(ramses::displayBufferId_t providerBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTag)
    {
        m_sceneControlAPI->linkOffscreenBuffer(providerBuffer, consumerScene, consumerTag);
        m_sceneControlAPI->flush();
    }

    void TestRenderer::createDataLink(ramses::sceneId_t providerScene, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId)
    {
        m_sceneControlAPI->linkData(providerScene, providerId, consumerScene, consumerId);
        m_sceneControlAPI->flush();
    }

    void TestRenderer::removeDataLink(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId)
    {
        m_sceneControlAPI->unlinkData(consumerScene, consumerId);
        m_sceneControlAPI->flush();
    }

    void TestRenderer::updateWarpingMeshData(ramses::displayId_t displayId, const ramses::WarpingMeshData& warpingMeshData)
    {
        m_renderer->updateWarpingMeshData(displayId, warpingMeshData);
        m_renderer->flush();
    }

    bool TestRenderer::performScreenshotCheck(ramses::displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const String& comparisonImageFile, float maxAveragePercentErrorPerPixel, bool readPixelsTwice)
    {
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
        FrameProfileRenderer::ForAllFrameProfileRenderer(
            m_renderer->impl.getRenderer().getRenderer(),
            [](FrameProfileRenderer& renderer) { renderer.enable(!renderer.isEnabled()); });
    }

    IEmbeddedCompositingManager& TestRenderer::getEmbeddedCompositorManager(ramses::displayId_t displayId)
    {
        IDisplayController& displayController = m_renderer->impl.getRenderer().getRenderer().getDisplayController(DisplayHandle(displayId.getValue()));
        return displayController.getEmbeddedCompositingManager();
    }

    void TestRenderer::setSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility)
    {
        m_renderer->impl.getRenderer().getRenderer().systemCompositorSetIviSurfaceVisibility(surfaceId, visibility);
    }

    void TestRenderer::readPixels(ramses::displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        m_renderer->readPixels(displayId, x, y, width, height);
    }

    IEmbeddedCompositor& TestRenderer::getEmbeddedCompositor(ramses::displayId_t displayId)
    {
        return m_renderer->impl.getRenderer().getRenderer().getDisplayController(DisplayHandle(displayId.getValue())).getRenderBackend().getEmbeddedCompositor();
    }

    void TestRenderer::setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender)
    {
        m_renderer->setFrameTimerLimits(0u, limitForClientResourcesUpload, limitForOffscreenBufferRender);
    }

    bool TestRenderer::hasSystemCompositorController() const
    {
        return m_renderer->impl.getRenderer().getRenderer().hasSystemCompositorController();
    }
}
