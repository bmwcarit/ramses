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
#include "RendererResourceManagerMock.h"

namespace ramses_internal
{
    class RendererSceneUpdaterMock : public RendererSceneUpdater
    {
    public:
        RendererSceneUpdaterMock(const Renderer& renderer, const RendererScenes& rendererScenes, const SceneStateExecutor& sceneStateExecutor, const RendererEventCollector& rendererEventCollector, const FrameTimer& frameTimer, const SceneExpirationMonitor& expirationMonitor, const IRendererResourceCache* rendererResourceCache);
        virtual ~RendererSceneUpdaterMock();

        MOCK_METHOD(void, handleSceneActions, (SceneId sceneId, SceneUpdate&& update), (override));
        MOCK_METHOD(void, handlePickEvent, (SceneId sceneId, Vector2 coords), (override));
        MOCK_METHOD(std::unique_ptr<IRendererResourceManager>, createResourceManager, (IResourceProvider&, IResourceUploader&, IRenderBackend&, IEmbeddedCompositingManager&, DisplayHandle, bool, uint64_t), (override));
    };

    class RendererSceneUpdaterFacade : public RendererSceneUpdaterMock
    {
    public:
        RendererSceneUpdaterFacade(const Renderer& renderer, const RendererScenes& rendererScenes, const SceneStateExecutor& sceneStateExecutor, const RendererEventCollector& rendererEventCollector, const FrameTimer& frameTimer, const SceneExpirationMonitor& expirationMonitor, const IRendererResourceCache* rendererResourceCache);
        virtual ~RendererSceneUpdaterFacade();

        virtual void handleSceneActions(SceneId sceneId, SceneUpdate&& update) override;

        std::unordered_map<DisplayHandle, testing::StrictMock<RendererResourceManagerMock>*> m_resourceManagerMocks;

    protected:
        virtual std::unique_ptr<IRendererResourceManager> createResourceManager(
            IResourceProvider& resourceProvider,
            IResourceUploader& resourceUploader,
            IRenderBackend& renderBackend,
            IEmbeddedCompositingManager& embeddedCompositingManager,
            DisplayHandle display,
            bool keepEffectsUploaded,
            uint64_t gpuCacheSize) override;
    };
}

#endif
