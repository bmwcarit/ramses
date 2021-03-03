//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererSceneUpdaterFacade.h"
#include "Components/SceneUpdate.h"
#include "RendererAPI/IEmbeddedCompositingManager.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererLib/IResourceUploader.h"
#include "Watchdog/IThreadAliveNotifier.h"

namespace ramses_internal
{
    RendererSceneUpdaterPartialMock::RendererSceneUpdaterPartialMock(
        IPlatform& platform,
        const Renderer& renderer,
        const RendererScenes& rendererScenes,
        const SceneStateExecutor& sceneStateExecutor,
        const RendererEventCollector& rendererEventCollector,
        const FrameTimer& frameTimer,
        const SceneExpirationMonitor& expirationMonitor,
        IThreadAliveNotifier& notifier,
        const IRendererResourceCache* rendererResourceCache)
        : RendererSceneUpdater(platform, const_cast<Renderer&>(renderer), const_cast<RendererScenes&>(rendererScenes), const_cast<SceneStateExecutor&>(sceneStateExecutor), const_cast<RendererEventCollector&>(rendererEventCollector), const_cast<FrameTimer&>(frameTimer), const_cast<SceneExpirationMonitor&>(expirationMonitor), const_cast<IThreadAliveNotifier&>(notifier), const_cast<IRendererResourceCache*>(rendererResourceCache))
    {
    }

    RendererSceneUpdaterPartialMock::~RendererSceneUpdaterPartialMock() = default;

    RendererSceneUpdaterFacade::RendererSceneUpdaterFacade(
        IPlatform& platform,
        const Renderer& renderer,
        const RendererScenes& rendererScenes,
        const SceneStateExecutor& sceneStateExecutor,
        const RendererEventCollector& rendererEventCollector,
        const FrameTimer& frameTimer,
        const SceneExpirationMonitor& expirationMonitor,
        IThreadAliveNotifier& notifier,
        const IRendererResourceCache* rendererResourceCache)
        : RendererSceneUpdaterPartialMock(platform, renderer, rendererScenes, sceneStateExecutor, rendererEventCollector, frameTimer, expirationMonitor, notifier, rendererResourceCache)
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

    void RendererSceneUpdaterFacade::handlePickEvent(SceneId sceneId, Vector2 coords)
    {
        RendererSceneUpdaterPartialMock::handlePickEvent(sceneId, coords);
        RendererSceneUpdater::handlePickEvent(sceneId, coords); // NOLINT clang-tidy: We really mean to call into RendererSceneUpdater
    }

    RendererSceneUpdaterFacade::~RendererSceneUpdaterFacade() = default;

    std::unique_ptr<IRendererResourceManager> RendererSceneUpdaterFacade::createResourceManager(
        IRenderBackend& renderBackend,
        AsyncEffectUploader& asyncEffectUploader,
        IEmbeddedCompositingManager& embeddedCompositingManager,
        DisplayHandle display,
        bool keepEffectsUploaded,
        uint64_t gpuCacheSize,
        IBinaryShaderCache* binaryShaderCache)
    {
        RendererSceneUpdaterPartialMock::createResourceManager(renderBackend, asyncEffectUploader, embeddedCompositingManager, display, keepEffectsUploaded, gpuCacheSize, binaryShaderCache);
        testing::StrictMock<RendererResourceManagerRefCountMock>* resMgrMock = new testing::StrictMock<RendererResourceManagerRefCountMock>;
        assert(!m_resourceManagerMocks.count(display));
        m_resourceManagerMocks[display] = resMgrMock;
        return std::unique_ptr<IRendererResourceManager>{ resMgrMock };
    }
}
