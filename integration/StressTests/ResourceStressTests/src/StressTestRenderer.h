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
#include "RendererTestEventHandler.h"


namespace ramses
{
    class RamsesRenderer;
    class RamsesFramework;
}

namespace ramses_internal
{
    // TODO Violin this can be merged with the other sandwich tests
    class StressTestRenderer
    {
    public:
        StressTestRenderer(ramses::RamsesFramework& framework, const ramses::RendererConfig& config);

        ramses::displayId_t createDisplay(uint32_t offsetX, uint32_t width, uint32_t height, int32_t argc = 0, const char* argv[] = 0);
        ramses::offscreenBufferId_t createOffscreenBuffer(ramses::displayId_t displayId, uint32_t width, uint32_t height, bool interruptable);
        void startLooping();
        void setFPS(uint32_t fpsAsInteger);
        void setFrameTimerLimits(uint64_t limitForClientResourcesUpload, uint64_t limitForSceneActionsApply, uint64_t limitForOffscreenBufferRender);
        void setSkippingOfUnmodifiedBuffers(bool enabled);

        void subscribeMapShowScene(ramses::displayId_t displayId, ramses::sceneId_t sceneId);

        void subscribeScene(ramses::sceneId_t sceneId);
        void mapScene(ramses::displayId_t displayId, ramses::sceneId_t sceneId);
        void showScene(ramses::sceneId_t sceneId);
        void showSceneOnOffscreenBuffer(ramses::sceneId_t sceneId, ramses::offscreenBufferId_t offscreenBuffer);

        void hideAndUnmapScene(ramses::sceneId_t sceneId);

        void linkOffscreenBufferToSceneTexture(ramses::sceneId_t sceneId, ramses::offscreenBufferId_t offscreenBuffer, ramses::dataConsumerId_t consumerTexture);

        void waitForNamedFlush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t flushName);
        void consumePendingEvents();

    private:
        ramses::RamsesRenderer  m_renderer;
        RendererTestEventHandler m_eventHandler;
    };
}

#endif
