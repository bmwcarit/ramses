//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_STRESSTESTRENDERER_H
#define RAMSES_STRESSTESTRENDERER_H

#include "ramses-renderer-api/RamsesRenderer.h"
#include "RendererAndSceneTestEventHandler.h"

namespace ramses
{
    class RamsesRenderer;
    class RamsesFramework;
}

namespace ramses_internal
{
    class StressTestRenderer
    {
    public:
        StressTestRenderer(ramses::RamsesFramework& framework, const ramses::RendererConfig& config);
        ~StressTestRenderer();

        ramses::displayId_t createDisplay(uint32_t offsetX, uint32_t width, uint32_t height, uint32_t displayIndex, const ramses::DisplayConfig& config);
        ramses::displayBufferId_t createOffscreenBuffer(ramses::displayId_t displayId, uint32_t width, uint32_t height, bool interruptable);
        void startLooping();
        void setFPS(ramses::displayId_t display, uint32_t fpsAsInteger);
        void setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender);
        void setSkippingOfUnmodifiedBuffers(bool enabled);

        void setSceneDisplayAndBuffer(ramses::sceneId_t sceneId, ramses::displayId_t display, ramses::displayBufferId_t displayBuffer = {});
        void setSceneState(ramses::sceneId_t sceneId, ramses::RendererSceneState state);
        void waitForSceneState(ramses::sceneId_t sceneId, ramses::RendererSceneState state);
        void linkOffscreenBufferToSceneTexture(ramses::sceneId_t sceneId, ramses::displayBufferId_t offscreenBuffer, ramses::dataConsumerId_t consumerTexture);

        void waitForFlush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t flushVersion);
        void consumePendingEvents();

    private:
        ramses::RamsesFramework& m_framework;
        ramses::RamsesRenderer&  m_renderer;
        ramses::RendererSceneControl& m_sceneControlAPI;
        ramses::RendererAndSceneTestEventHandler m_eventHandler;
    };
}

#endif
