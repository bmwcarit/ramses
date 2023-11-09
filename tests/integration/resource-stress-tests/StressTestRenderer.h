//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/RamsesRenderer.h"
#include "RendererAndSceneTestEventHandler.h"

namespace ramses
{
    class RamsesRenderer;
    class RamsesFramework;
}

namespace ramses::internal
{
    class StressTestRenderer
    {
    public:
        StressTestRenderer(ramses::RamsesFramework& framework, const ramses::RendererConfig& config);
        ~StressTestRenderer();

        displayId_t createDisplay(uint32_t offsetX, uint32_t width, uint32_t height, uint32_t displayIndex, const ramses::DisplayConfig& config);
        displayBufferId_t createOffscreenBuffer(displayId_t displayId, uint32_t width, uint32_t height, bool interruptible);
        void startLooping();
        void setFPS(displayId_t display, uint32_t fpsAsInteger);
        void setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender);
        void setSkippingOfUnmodifiedBuffers(bool enabled);

        void setSceneDisplayAndBuffer(sceneId_t sceneId, displayId_t display, displayBufferId_t displayBuffer = {});
        void setSceneState(sceneId_t sceneId, RendererSceneState state);
        void waitForSceneState(ramses::Scene& scene, RendererSceneState state);
        void linkOffscreenBufferToSceneTexture(sceneId_t sceneId, displayBufferId_t offscreenBuffer, dataConsumerId_t consumerTexture);

        void waitForFlush(sceneId_t sceneId, sceneVersionTag_t flushVersion);
        void consumePendingEvents();

    private:
        RamsesFramework& m_framework;
        RamsesRenderer&  m_renderer;
        RendererSceneControl& m_sceneControlAPI;
        RendererAndSceneTestEventHandler m_eventHandler;
    };
}
