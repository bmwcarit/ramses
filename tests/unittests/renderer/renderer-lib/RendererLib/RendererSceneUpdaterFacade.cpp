//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererSceneUpdaterFacade.h"
#include "internal/Components/SceneUpdate.h"

namespace ramses::internal
{
    RendererSceneUpdaterPartialMock::RendererSceneUpdaterPartialMock(
        DisplayHandle display,
        IPlatform& platform,
        const Renderer& renderer,
        const RendererScenes& rendererScenes,
        const SceneStateExecutor& sceneStateExecutor,
        const RendererEventCollector& rendererEventCollector,
        const FrameTimer& frameTimer,
        const SceneExpirationMonitor& expirationMonitor,
        IThreadAliveNotifier& notifier)
        : RendererSceneUpdater(display,
                               platform,
                               const_cast<Renderer&>(renderer),
                               const_cast<RendererScenes&>(rendererScenes),
                               const_cast<SceneStateExecutor&>(sceneStateExecutor),
                               const_cast<RendererEventCollector&>(rendererEventCollector),
                               const_cast<FrameTimer&>(frameTimer),
                               const_cast<SceneExpirationMonitor&>(expirationMonitor),
                               const_cast<IThreadAliveNotifier&>(notifier),
                               EFeatureLevel_Latest)
    {
    }

    RendererSceneUpdaterPartialMock::~RendererSceneUpdaterPartialMock() = default;

    RendererSceneUpdaterFacade::RendererSceneUpdaterFacade(
        DisplayHandle display,
        IPlatform& platform,
        const Renderer& renderer,
        const RendererScenes& rendererScenes,
        const SceneStateExecutor& sceneStateExecutor,
        const RendererEventCollector& rendererEventCollector,
        const FrameTimer& frameTimer,
        const SceneExpirationMonitor& expirationMonitor,
        IThreadAliveNotifier& notifier)
        : RendererSceneUpdaterPartialMock(display, platform, renderer, rendererScenes, sceneStateExecutor, rendererEventCollector, frameTimer, expirationMonitor, notifier)
    {
    }

    void RendererSceneUpdaterFacade::handleSceneUpdate(SceneId sceneId, SceneUpdate&& update)
    {
        SceneUpdate copyOfActionsForScene;
        copyOfActionsForScene.actions = update.actions.copy();
        copyOfActionsForScene.resources = update.resources;
        copyOfActionsForScene.flushInfos = update.flushInfos.copy();
        RendererSceneUpdaterPartialMock::handleSceneUpdate(sceneId, std::move(update));
        RendererSceneUpdater::handleSceneUpdate(sceneId, std::move(copyOfActionsForScene)); // NOLINT clang-tidy: We really mean to call into RendererSceneUpdater
    }

    void RendererSceneUpdaterFacade::handlePickEvent(SceneId sceneId, glm::vec2 coords)
    {
        RendererSceneUpdaterPartialMock::handlePickEvent(sceneId, coords);
        RendererSceneUpdater::handlePickEvent(sceneId, coords); // NOLINT clang-tidy: We really mean to call into RendererSceneUpdater
    }

    void RendererSceneUpdaterFacade::setForceMapTimeout(std::chrono::milliseconds timeout)
    {
        m_maximumWaitingTimeToForceMap = timeout;
    }

    RendererSceneUpdaterFacade::~RendererSceneUpdaterFacade() = default;

    std::unique_ptr<IRendererResourceManager> RendererSceneUpdaterFacade::createResourceManager(
        IRenderBackend& renderBackend,
        IEmbeddedCompositingManager& embeddedCompositingManager,
        const DisplayConfigData& displayConfig,
        IBinaryShaderCache* binaryShaderCache)
    {
        RendererSceneUpdaterPartialMock::createResourceManager(renderBackend, embeddedCompositingManager, displayConfig, binaryShaderCache);
        auto* resMgrMock = new testing::StrictMock<RendererResourceManagerRefCountMock>;
        assert(m_resourceManagerMock == nullptr);
        m_resourceManagerMock = resMgrMock;
        return std::unique_ptr<IRendererResourceManager>{ resMgrMock };
    }
}
