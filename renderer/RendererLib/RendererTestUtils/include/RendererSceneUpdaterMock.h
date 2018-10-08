//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENEUPDATERMOCK_H
#define RAMSES_RENDERERSCENEUPDATERMOCK_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererLib/RendererSceneUpdater.h"
#include "RendererLib/RendererScenes.h"

namespace ramses_internal
{
    class RendererSceneUpdaterMock : public RendererSceneUpdater
    {
    public:
        RendererSceneUpdaterMock(const Renderer& renderer, const RendererScenes& rendererScenes, const SceneStateExecutor& sceneStateExecutor, const RendererEventCollector& rendererEventCollector, const FrameTimer& frameTimer, const LatencyMonitor& latencyMonitor)
            : RendererSceneUpdater(const_cast<Renderer&>(renderer), const_cast<RendererScenes&>(rendererScenes), const_cast<SceneStateExecutor&>(sceneStateExecutor), const_cast<RendererEventCollector&>(rendererEventCollector), const_cast<FrameTimer&>(frameTimer), const_cast<LatencyMonitor&>(latencyMonitor))
        {
        }

        MOCK_METHOD2(handleSceneActions, void(SceneId sceneId, SceneActionCollection& actionsForScene));
    };

    class RendererSceneUpdaterFacade : public RendererSceneUpdaterMock
    {
    public:
        RendererSceneUpdaterFacade(const Renderer& renderer, const RendererScenes& rendererScenes, const SceneStateExecutor& sceneStateExecutor, const RendererEventCollector& rendererEventCollector, const FrameTimer& frameTimer, const LatencyMonitor& latencyMonitor)
            : RendererSceneUpdaterMock(renderer, rendererScenes, sceneStateExecutor, rendererEventCollector, frameTimer, latencyMonitor)
        {
        }

        virtual void handleSceneActions(SceneId sceneId, SceneActionCollection& actionsForScene) override
        {
            RendererSceneUpdaterMock::handleSceneActions(sceneId, actionsForScene);
            RendererSceneUpdater::handleSceneActions(sceneId, actionsForScene);
        }
    };
}

#endif
