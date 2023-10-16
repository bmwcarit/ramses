//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestRenderer.h"
#include "ramses/renderer/RendererConfig.h"
#include "ramses/client/Scene.h"
#include "internal/RendererLib/PlatformInterface/IRenderBackend.h"
#include "impl/RamsesRendererImpl.h"
#include "impl/RendererSceneControlImpl.h"
#include "RendererAndSceneTestEventHandler.h"

namespace ramses::internal
{
    displayId_t TestRenderer::createDisplay(const ramses::DisplayConfig& displayConfig)
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

    void TestRenderer::destroyDisplay(displayId_t displayId)
    {
        RendererTestUtils::DestroyDisplayImmediate(*m_renderer, displayId);
    }

    displayBufferId_t TestRenderer::getDisplayFramebufferId(displayId_t displayId) const
    {
        return m_renderer->getDisplayFramebuffer(displayId);
    }

    void TestRenderer::setSceneMapping(sceneId_t sceneId, displayId_t display)
    {
        m_sceneControlAPI->setSceneMapping(sceneId, display);
        m_sceneControlAPI->flush();
    }

    bool TestRenderer::getSceneToState(ramses::Scene& scene, ramses::RendererSceneState state)
    {
        setSceneState(scene.getSceneId(), state);
        return waitForSceneStateChange(scene, state);
    }

    void TestRenderer::setSceneState(sceneId_t sceneId, ramses::RendererSceneState state)
    {
        m_sceneControlAPI->setSceneState(sceneId, state);
        m_sceneControlAPI->flush();
    }

    bool TestRenderer::waitForSceneStateChange(ramses::Scene& scene, ramses::RendererSceneState state)
    {
        RendererAndSceneTestEventHandler eventHandler(*m_renderer);
        return eventHandler.waitForSceneState(scene, state);
    }

    void TestRenderer::waitForFlush(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag)
    {
        RendererAndSceneTestEventHandler sceneEventHandler(*m_renderer);
        sceneEventHandler.waitForFlush(sceneId, sceneVersionTag);
    }

    bool TestRenderer::checkScenesExpired(std::initializer_list<sceneId_t> sceneIds)
    {
        RendererAndSceneTestEventHandler sceneEventHandler(*m_renderer);
        for (auto sceneId : sceneIds)
        {
            if (!sceneEventHandler.checkExpiredState(sceneId))
                return false;
        }

        return true;
    }

    bool TestRenderer::checkScenesNotExpired(std::initializer_list<sceneId_t> sceneIds)
    {
        RendererAndSceneTestEventHandler sceneEventHandler(*m_renderer);
        for (auto sceneId : sceneIds)
        {
            if (sceneEventHandler.checkExpiredState(sceneId))
                return false;
        }

        return true;
    }

    bool TestRenderer::waitForStreamSurfaceAvailabilityChange(waylandIviSurfaceId_t streamSource, bool available)
    {
        RendererAndSceneTestEventHandler eventHandler(*m_renderer);
        return eventHandler.waitForStreamSurfaceAvailabilityChange(streamSource, available);
    }

    void TestRenderer::dispatchEvents(IRendererEventHandler& eventHandler, IRendererSceneControlEventHandler& sceneControlEventHandler)
    {
        m_renderer->dispatchEvents(eventHandler);
        m_sceneControlAPI->dispatchEvents(sceneControlEventHandler);
    }

    void TestRenderer::setLoopMode(ELoopMode loopMode)
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

    bool TestRenderer::isRendererThreadEnabled() const
    {
        return m_renderer->isThreadRunning();
    }

    void TestRenderer::doOneLoop()
    {
        assert(!m_renderer->isThreadRunning());
        m_renderer->doOneLoop();
    }

    displayBufferId_t TestRenderer::createOffscreenBuffer(displayId_t displayId, uint32_t width, uint32_t height, bool interruptible, uint32_t sampleCount, EDepthBufferType depthBufferType)
    {
        displayBufferId_t offscreenBufferId;
        if (interruptible)
        {
            offscreenBufferId = m_renderer->createInterruptibleOffscreenBuffer(displayId, width, height, depthBufferType);
        }
        else
        {
            offscreenBufferId = m_renderer->createOffscreenBuffer(displayId, width, height, sampleCount, depthBufferType);
        }
        m_renderer->flush();

        RendererAndSceneTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForOffscreenBufferCreation(offscreenBufferId);

        return offscreenBufferId;
    }

    displayBufferId_t TestRenderer::createDmaOffscreenBuffer(displayId_t displayId, uint32_t width, uint32_t height, uint32_t bufferFourccFormat, uint32_t bufferUsageFlags, uint64_t modifier)
    {
        displayBufferId_t offscreenBufferId;
        offscreenBufferId = m_renderer->createDmaOffscreenBuffer(displayId, width, height, bufferFourccFormat, bufferUsageFlags, modifier);
        m_renderer->flush();

        RendererAndSceneTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForOffscreenBufferCreation(offscreenBufferId);

        return offscreenBufferId;
    }

    void TestRenderer::destroyOffscreenBuffer(displayId_t displayId, displayBufferId_t buffer)
    {
        m_renderer->destroyOffscreenBuffer(displayId, buffer);
        m_renderer->flush();

        RendererAndSceneTestEventHandler eventHandler(*m_renderer);
        eventHandler.waitForOffscreenBufferDestruction(buffer);
    }

    void TestRenderer::assignSceneToDisplayBuffer(sceneId_t sceneId, displayBufferId_t buffer, int32_t renderOrder)
    {
        m_sceneControlAPI->setSceneDisplayBufferAssignment(sceneId, buffer, renderOrder);
        m_sceneControlAPI->flush();
    }

    void TestRenderer::setClearFlags(displayId_t displayId, displayBufferId_t buffer, ClearFlags clearFlags)
    {
        m_renderer->setDisplayBufferClearFlags(displayId, buffer, clearFlags);
        m_renderer->flush();
    }

    void TestRenderer::setClearColor(displayId_t displayId, displayBufferId_t buffer, const glm::vec4& clearColor)
    {
        m_renderer->setDisplayBufferClearColor(displayId, buffer, {clearColor.r, clearColor.g, clearColor.b, clearColor.a});
        m_renderer->flush();
    }

    bool TestRenderer::getDmaOffscreenBufferFDAndStride(displayId_t displayId, displayBufferId_t displayBufferId, int& fd, uint32_t& stride) const
    {
        return m_renderer->getDmaOffscreenBufferFDAndStride(displayId, displayBufferId, fd, stride);
    }

    streamBufferId_t TestRenderer::createStreamBuffer(displayId_t displayId, waylandIviSurfaceId_t source)
    {
        const auto bufferId = m_renderer->createStreamBuffer(displayId, source);
        m_renderer->flush();

        return bufferId;
    }

    void TestRenderer::destroyStreamBuffer(displayId_t displayId, streamBufferId_t buffer)
    {
        m_renderer->impl().destroyStreamBuffer(displayId, buffer);
        m_renderer->flush();
    }

    void TestRenderer::createBufferDataLink(displayBufferId_t providerBuffer, sceneId_t consumerScene, dataConsumerId_t consumerTag)
    {
        m_sceneControlAPI->linkOffscreenBuffer(providerBuffer, consumerScene, consumerTag);
        m_sceneControlAPI->flush();
    }

    void TestRenderer::createBufferDataLink(streamBufferId_t providerBuffer, sceneId_t consumerScene, dataConsumerId_t consumerTag)
    {
        m_sceneControlAPI->impl().linkStreamBuffer(providerBuffer, consumerScene, consumerTag);
        m_sceneControlAPI->flush();
    }

    void TestRenderer::createDataLink(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId)
    {
        m_sceneControlAPI->linkData(providerScene, providerId, consumerScene, consumerId);
        m_sceneControlAPI->flush();
    }

    void TestRenderer::removeDataLink(sceneId_t consumerScene, dataConsumerId_t consumerId)
    {
        m_sceneControlAPI->unlinkData(consumerScene, consumerId);
        m_sceneControlAPI->flush();
    }

    bool TestRenderer::performScreenshotCheck(
        displayId_t displayId,
        displayBufferId_t bufferId,
        uint32_t x, uint32_t y, uint32_t width, uint32_t height,
        const std::string& comparisonImageFile,
        float maxAveragePercentErrorPerPixel,
        bool readPixelsTwice,
        bool saveDiffOnError)
    {
        // In some cases (interruptible OB) taking screenshot needs to be delayed to guarantee that the last state was rendered to framebuffer,
        // reading pixels twice before checking result guarantees that.
        if (readPixelsTwice)
            RendererTestUtils::ReadPixelData(*m_renderer, displayId, bufferId, x, y, width, height);

        return RendererTestUtils::PerformScreenshotTestForDisplay(
            *m_renderer,
            displayId,
            bufferId,
            x,
            y,
            width,
            height,
            comparisonImageFile,
            maxAveragePercentErrorPerPixel,
            saveDiffOnError);
    }

    void TestRenderer::saveScreenshotForDisplay(displayId_t displayId, displayBufferId_t bufferId, uint32_t x, uint32_t y, uint32_t width, uint32_t height, const std::string& imageFile)
    {
        RendererTestUtils::SaveScreenshotForDisplay(
            *m_renderer,
            displayId,
            bufferId,
            x,
            y,
            width,
            height,
            imageFile);
    }

    void TestRenderer::readPixels(displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        m_renderer->readPixels(displayId, {}, x, y, width, height);
    }

    void TestRenderer::setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender)
    {
        m_renderer->setFrameTimerLimits(0u, limitForClientResourcesUpload, limitForOffscreenBufferRender);
    }

    void TestRenderer::setSurfaceVisibility(WaylandIviSurfaceId surfaceId, bool visibility)
    {
        RendererCommands cmds;
        cmds.push_back(RendererCommand::SCSetIviSurfaceVisibility{ surfaceId, visibility });
        m_renderer->impl().pushAndConsumeRendererCommands(cmds);
    }

    IEmbeddedCompositor& TestRenderer::getEmbeddedCompositor(displayId_t displayId)
    {
        return m_renderer->impl().getDisplayDispatcher().getEC(DisplayHandle{ displayId.getValue() });
    }

    IEmbeddedCompositingManager& TestRenderer::getEmbeddedCompositorManager(displayId_t displayId)
    {
        return m_renderer->impl().getDisplayDispatcher().getECManager(DisplayHandle{ displayId.getValue() });
    }

    bool TestRenderer::hasSystemCompositorController() const
    {
        return m_renderer->impl().getDisplayDispatcher().hasSystemCompositorController();
    }
}
