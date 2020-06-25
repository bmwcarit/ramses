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
        RendererSceneUpdaterMock(const Renderer& renderer, const RendererScenes& rendererScenes, const SceneStateExecutor& sceneStateExecutor, const RendererEventCollector& rendererEventCollector, const FrameTimer& frameTimer, const SceneExpirationMonitor& expirationMonitor)
            : RendererSceneUpdater(const_cast<Renderer&>(renderer), const_cast<RendererScenes&>(rendererScenes), const_cast<SceneStateExecutor&>(sceneStateExecutor), const_cast<RendererEventCollector&>(rendererEventCollector), const_cast<FrameTimer&>(frameTimer), const_cast<SceneExpirationMonitor&>(expirationMonitor))
        {
        }

        MOCK_METHOD(void, handleSceneActions, (SceneId sceneId, SceneActionCollection&& actionsForScene), (override));
        MOCK_METHOD(void, handlePickEvent, (SceneId sceneId, Vector2 coords), (override));
    };

    class RendererSceneUpdaterFacade : public RendererSceneUpdaterMock
    {
    public:
        RendererSceneUpdaterFacade(const Renderer& renderer, const RendererScenes& rendererScenes, const SceneStateExecutor& sceneStateExecutor, const RendererEventCollector& rendererEventCollector, const FrameTimer& frameTimer, const SceneExpirationMonitor& expirationMonitor)
            : RendererSceneUpdaterMock(renderer, rendererScenes, sceneStateExecutor, rendererEventCollector, frameTimer, expirationMonitor)
        {
        }

        virtual void handleSceneActions(SceneId sceneId, SceneActionCollection&& actionsForScene) override
        {
            SceneActionCollection copyOfActionsForScene(actionsForScene.copy());
            RendererSceneUpdaterMock::handleSceneActions(sceneId, std::move(copyOfActionsForScene));
            RendererSceneUpdater::handleSceneActions(sceneId, std::move(actionsForScene)); // NOLINT clang-tidy: We really mean to call into RendererSceneUpdater
        }
    };
}

#endif
