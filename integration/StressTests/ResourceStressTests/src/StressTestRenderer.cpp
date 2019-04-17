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
        : m_renderer(framework, config)
        , m_eventHandler(m_renderer, 0)
    {
    }

    ramses::displayId_t StressTestRenderer::createDisplay(uint32_t offsetX, uint32_t width, uint32_t height, uint32_t displayIndex, int32_t argc, const char* argv[])
    {
        ramses::DisplayConfig displayConfig(argc, argv);
        displayConfig.setWindowRectangle(offsetX + 50, 50, width, height);

        const auto iviSurfaceId = displayConfig.getWaylandIviSurfaceID();
        displayConfig.setWaylandIviSurfaceID((InvalidWaylandIviSurfaceId.getValue() != iviSurfaceId? iviSurfaceId : 10000)+ displayIndex);

        if(InvalidWaylandIviLayerId.getValue() == displayConfig.getWaylandIviLayerID())
            displayConfig.setWaylandIviLayerID(2);

        ramses::displayId_t displayId = m_renderer.createDisplay(displayConfig);
        m_renderer.flush();
        m_eventHandler.waitForDisplayCreation(displayId);
        return displayId;
    }

    ramses::offscreenBufferId_t StressTestRenderer::createOffscreenBuffer(ramses::displayId_t displayId, uint32_t width, uint32_t height, bool interruptable)
    {
        ramses::offscreenBufferId_t offscreenBufferId;
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

    void StressTestRenderer::setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForSceneActionsApply, uint64_t limitForOffscreenBufferRender)
    {
        m_renderer.setFrameTimerLimits(0u, limitForClientResourcesUpload, limitForSceneActionsApply, limitForOffscreenBufferRender);
    }

    void StressTestRenderer::setSkippingOfUnmodifiedBuffers(bool enabled)
    {
        m_renderer.setSkippingOfUnmodifiedBuffers(enabled);
    }

    void StressTestRenderer::subscribeMapShowScene(ramses::displayId_t displayId, ramses::sceneId_t sceneId)
    {
        subscribeScene(sceneId);
        mapScene(displayId, sceneId);
        showScene(sceneId);
    }

    void StressTestRenderer::subscribeScene(ramses::sceneId_t sceneId)
    {
        m_eventHandler.waitForPublication(sceneId);

        m_renderer.subscribeScene(sceneId);
        m_renderer.flush();
        m_eventHandler.waitForSubscription(sceneId);
    }

    void StressTestRenderer::mapScene(ramses::displayId_t displayId, ramses::sceneId_t sceneId)
    {
        m_renderer.mapScene(displayId, sceneId);
        m_renderer.flush();

        m_eventHandler.waitForMapped(sceneId);
    }

    void StressTestRenderer::showScene(ramses::sceneId_t sceneId)
    {
        m_renderer.showScene(sceneId);
        m_renderer.flush();
        m_eventHandler.waitForShown(sceneId);
    }

    void StressTestRenderer::showSceneOnOffscreenBuffer(ramses::sceneId_t sceneId, ramses::offscreenBufferId_t offscreenBuffer)
    {
        m_renderer.assignSceneToOffscreenBuffer(sceneId, offscreenBuffer);
        showScene(sceneId);
    }

    void StressTestRenderer::hideAndUnmapScene(ramses::sceneId_t sceneId)
    {
        m_renderer.hideScene(sceneId);
        m_renderer.flush();
        m_eventHandler.waitForHidden(sceneId);

        m_renderer.unmapScene(sceneId);
        m_renderer.flush();
        m_eventHandler.waitForUnmapped(sceneId);
    }

    void StressTestRenderer::waitForNamedFlush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t flushName)
    {
        m_eventHandler.waitForNamedFlush(sceneId, flushName, true);
    }

    void StressTestRenderer::linkOffscreenBufferToSceneTexture(ramses::sceneId_t sceneId, ramses::offscreenBufferId_t offscreenBuffer, ramses::dataConsumerId_t consumerTexture)
    {
        m_renderer.linkOffscreenBufferToSceneData(offscreenBuffer, sceneId, consumerTexture);
        m_renderer.flush();

        m_eventHandler.waitForOffscreenBufferLink(offscreenBuffer);
    }

    void StressTestRenderer::consumePendingEvents()
    {
        m_eventHandler.consumePendingEvents();
    }

}
