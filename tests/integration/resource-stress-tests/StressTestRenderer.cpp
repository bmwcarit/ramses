//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StressTestRenderer.h"
#include "ramses/renderer/DisplayConfig.h"

namespace ramses::internal
{
    StressTestRenderer::StressTestRenderer(ramses::RamsesFramework& framework, const ramses::RendererConfig& config)
        : m_framework(framework)
        , m_renderer(*framework.createRenderer(config))
        , m_sceneControlAPI(*m_renderer.getSceneControlAPI())
        , m_eventHandler(m_renderer, std::chrono::milliseconds{ 0 })
    {
    }

    StressTestRenderer::~StressTestRenderer()
    {
        m_framework.destroyRenderer(m_renderer);
    }

    displayId_t StressTestRenderer::createDisplay(uint32_t offsetX, uint32_t width, uint32_t height, uint32_t displayIndex, const ramses::DisplayConfig& config)
    {
        ramses::DisplayConfig displayConfig(config);
        displayConfig.setWindowRectangle(offsetX + 50, 50, width, height);

        const auto iviSurfaceId = displayConfig.getWaylandIviSurfaceID();
        displayConfig.setWaylandIviSurfaceID(waylandIviSurfaceId_t((iviSurfaceId.isValid() ? iviSurfaceId.getValue() : 10000) + displayIndex));

        if(!displayConfig.getWaylandIviLayerID().isValid())
            displayConfig.setWaylandIviLayerID(waylandIviLayerId_t(2));

        displayId_t displayId = m_renderer.createDisplay(displayConfig);
        m_renderer.flush();
        m_eventHandler.waitForDisplayCreation(displayId);
        return displayId;
    }

    displayBufferId_t StressTestRenderer::createOffscreenBuffer(displayId_t displayId, uint32_t width, uint32_t height, bool interruptible)
    {
        displayBufferId_t offscreenBufferId;
        if (interruptible)
        {
            offscreenBufferId = m_renderer.createInterruptibleOffscreenBuffer(displayId, width, height);
        }
        else
        {
            offscreenBufferId = m_renderer.createOffscreenBuffer(displayId, width, height);
        }

        m_renderer.flush();
        m_eventHandler.waitForOffscreenBufferCreation(offscreenBufferId);
        return offscreenBufferId;
    }

    void StressTestRenderer::startLooping()
    {
        m_renderer.startThread();
    }

    void StressTestRenderer::setFPS(displayId_t display, uint32_t fpsAsInteger)
    {
        const auto fpsAsFloatBecauseWhyNot = static_cast<float>(fpsAsInteger);
        m_renderer.setFramerateLimit(display, fpsAsFloatBecauseWhyNot);
    }

    void StressTestRenderer::setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender)
    {
        m_renderer.setFrameTimerLimits(0u, limitForClientResourcesUpload, limitForOffscreenBufferRender);
    }

    void StressTestRenderer::setSkippingOfUnmodifiedBuffers(bool enabled)
    {
        m_renderer.setSkippingOfUnmodifiedBuffers(enabled);
    }

    void StressTestRenderer::setSceneDisplayAndBuffer(sceneId_t sceneId, displayId_t display, displayBufferId_t displayBuffer)
    {
        m_sceneControlAPI.setSceneMapping(sceneId, display);
        m_sceneControlAPI.setSceneDisplayBufferAssignment(sceneId, displayBuffer);
        m_sceneControlAPI.flush();
    }

    void StressTestRenderer::setSceneState(sceneId_t sceneId, RendererSceneState state)
    {
        m_sceneControlAPI.setSceneState(sceneId, state);
        m_sceneControlAPI.flush();
    }

    void StressTestRenderer::waitForSceneState(ramses::Scene& scene, RendererSceneState state)
    {
        m_eventHandler.waitForSceneState(scene, state);
    }

    void StressTestRenderer::waitForFlush(sceneId_t sceneId, sceneVersionTag_t flushVersion)
    {
        m_eventHandler.waitForFlush(sceneId, flushVersion);
    }

    void StressTestRenderer::linkOffscreenBufferToSceneTexture(sceneId_t sceneId, displayBufferId_t offscreenBuffer, dataConsumerId_t consumerTexture)
    {
        m_sceneControlAPI.linkOffscreenBuffer(offscreenBuffer, sceneId, consumerTexture);
        m_sceneControlAPI.flush();

        m_eventHandler.waitForOffscreenBufferLink(offscreenBuffer);
    }

    void StressTestRenderer::consumePendingEvents()
    {
        m_eventHandler.consumePendingEvents();
    }

}
