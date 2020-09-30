//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererSceneUpdaterMock.h"
#include "Components/SceneUpdate.h"
#include "RendererAPI/IEmbeddedCompositingManager.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererLib/IResourceUploader.h"
#include "RendererFramework/IResourceProvider.h"

namespace ramses_internal
{
    RendererSceneUpdaterMock::RendererSceneUpdaterMock(
        const Renderer& renderer,
        const RendererScenes& rendererScenes,
        const SceneStateExecutor& sceneStateExecutor,
        const RendererEventCollector& rendererEventCollector,
        const FrameTimer& frameTimer,
        const SceneExpirationMonitor& expirationMonitor,
        const IRendererResourceCache* rendererResourceCache)
        : RendererSceneUpdater(const_cast<Renderer&>(renderer), const_cast<RendererScenes&>(rendererScenes), const_cast<SceneStateExecutor&>(sceneStateExecutor), const_cast<RendererEventCollector&>(rendererEventCollector), const_cast<FrameTimer&>(frameTimer), const_cast<SceneExpirationMonitor&>(expirationMonitor), const_cast<IRendererResourceCache*>(rendererResourceCache))
    {
    }

    RendererSceneUpdaterMock::~RendererSceneUpdaterMock() = default;

    RendererSceneUpdaterFacade::RendererSceneUpdaterFacade(
        const Renderer& renderer,
        const RendererScenes& rendererScenes,
        const SceneStateExecutor& sceneStateExecutor,
        const RendererEventCollector& rendererEventCollector,
        const FrameTimer& frameTimer,
        const SceneExpirationMonitor& expirationMonitor,
        const IRendererResourceCache* rendererResourceCache)
        : RendererSceneUpdaterMock(renderer, rendererScenes, sceneStateExecutor, rendererEventCollector, frameTimer, expirationMonitor, rendererResourceCache)
    {
    }

    void RendererSceneUpdaterFacade::handleSceneActions(SceneId sceneId, SceneUpdate&& update)
    {
        SceneUpdate copyOfActionsForScene;
        copyOfActionsForScene.actions = update.actions.copy();
        copyOfActionsForScene.resources = update.resources;
        copyOfActionsForScene.flushInfos = update.flushInfos.copy();
        RendererSceneUpdaterMock::handleSceneActions(sceneId, std::move(update));
        RendererSceneUpdater::handleSceneActions(sceneId, std::move(copyOfActionsForScene)); // NOLINT clang-tidy: We really mean to call into RendererSceneUpdater
    }

    RendererSceneUpdaterFacade::~RendererSceneUpdaterFacade() = default;

    std::unique_ptr<IRendererResourceManager> RendererSceneUpdaterFacade::createResourceManager(
        IResourceProvider& resourceProvider,
        IResourceUploader& resourceUploader,
        IRenderBackend& renderBackend,
        IEmbeddedCompositingManager& embeddedCompositingManager,
        DisplayHandle display,
        bool keepEffectsUploaded,
        uint64_t gpuCacheSize)
    {
        RendererSceneUpdaterMock::createResourceManager(resourceProvider, resourceUploader, renderBackend, embeddedCompositingManager, display, keepEffectsUploaded, gpuCacheSize);
        testing::StrictMock<RendererResourceManagerMock>* resMgrMock = new testing::StrictMock<RendererResourceManagerMock>;
        assert(!m_resourceManagerMocks.count(display));
        m_resourceManagerMocks[display] = resMgrMock;
        return std::unique_ptr<IRendererResourceManager>{ resMgrMock };
    }
}
