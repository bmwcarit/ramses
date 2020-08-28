//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "StressTestRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"

namespace ramses_internal
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

    ramses::displayId_t StressTestRenderer::createDisplay(uint32_t offsetX, uint32_t width, uint32_t height, uint32_t displayIndex, int32_t argc, const char* argv[])
    {
        ramses::DisplayConfig displayConfig(argc, argv);
        displayConfig.setWindowRectangle(offsetX + 50, 50, width, height);

        const auto iviSurfaceId = displayConfig.getWaylandIviSurfaceID();
        displayConfig.setWaylandIviSurfaceID(ramses::waylandIviSurfaceId_t((iviSurfaceId.isValid() ? iviSurfaceId.getValue() : 10000) + displayIndex));

        if(!displayConfig.getWaylandIviLayerID().isValid())
            displayConfig.setWaylandIviLayerID(ramses::waylandIviLayerId_t(2));

        ramses::displayId_t displayId = m_renderer.createDisplay(displayConfig);
        m_renderer.flush();
        m_eventHandler.waitForDisplayCreation(displayId);
        return displayId;
    }

    ramses::displayBufferId_t StressTestRenderer::createOffscreenBuffer(ramses::displayId_t displayId, uint32_t width, uint32_t height, bool interruptable)
    {
        ramses::displayBufferId_t offscreenBufferId;
        if (interruptable)
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

    void StressTestRenderer::setFPS(uint32_t fpsAsInteger)
    {
        const float fpsAsFloatBecauseWhyNot = static_cast<float>(fpsAsInteger);
        m_renderer.setMaximumFramerate(fpsAsFloatBecauseWhyNot);
    }

    void StressTestRenderer::setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender)
    {
        m_renderer.setFrameTimerLimits(0u, limitForClientResourcesUpload, limitForOffscreenBufferRender);
    }

    void StressTestRenderer::setSkippingOfUnmodifiedBuffers(bool enabled)
    {
        m_renderer.setSkippingOfUnmodifiedBuffers(enabled);
    }

    void StressTestRenderer::setSceneDisplayAndBuffer(ramses::sceneId_t sceneId, ramses::displayId_t display, ramses::displayBufferId_t displayBuffer)
    {
        m_sceneControlAPI.setSceneMapping(sceneId, display);
        m_sceneControlAPI.setSceneDisplayBufferAssignment(sceneId, displayBuffer);
        m_sceneControlAPI.flush();
    }

    void StressTestRenderer::setSceneState(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        m_sceneControlAPI.setSceneState(sceneId, state);
        m_sceneControlAPI.flush();
    }

    void StressTestRenderer::waitForSceneState(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        m_eventHandler.waitForSceneState(sceneId, state);
    }

    void StressTestRenderer::waitForFlush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t flushVersion)
    {
        m_eventHandler.waitForFlush(sceneId, flushVersion);
    }

    void StressTestRenderer::linkOffscreenBufferToSceneTexture(ramses::sceneId_t sceneId, ramses::displayBufferId_t offscreenBuffer, ramses::dataConsumerId_t consumerTexture)
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
