//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENEUPDATERFACADE_H
#define RAMSES_RENDERERSCENEUPDATERFACADE_H

#include "renderer_common_gmock_header.h"
#include "gmock/gmock.h"
#include "RendererLib/RendererSceneUpdater.h"
#include "RendererLib/RendererScenes.h"
#include "RendererResourceManagerMock.h"

namespace ramses_internal
{
    class RendererSceneUpdaterPartialMock : public RendererSceneUpdater
    {
    public:
        RendererSceneUpdaterPartialMock(IPlatform& platform, const Renderer& renderer, const RendererScenes& rendererScenes, const SceneStateExecutor& sceneStateExecutor, const RendererEventCollector& rendererEventCollector, const FrameTimer& frameTimer, const SceneExpirationMonitor& expirationMonitor, IThreadAliveNotifier& notifier, const IRendererResourceCache* rendererResourceCache);
        virtual ~RendererSceneUpdaterPartialMock() override;

        MOCK_METHOD(void, handleSceneUpdate, (SceneId sceneId, SceneUpdate&& update), (override));
        MOCK_METHOD(void, handlePickEvent, (SceneId sceneId, Vector2 coords), (override));
        MOCK_METHOD(std::unique_ptr<IRendererResourceManager>, createResourceManager, (IRenderBackend&, AsyncEffectUploader&, IEmbeddedCompositingManager&, DisplayHandle, bool, uint64_t, IBinaryShaderCache*), (override));
    };

    class RendererSceneUpdaterFacade : public RendererSceneUpdaterPartialMock
    {
    public:
        RendererSceneUpdaterFacade(IPlatform& platform, const Renderer& renderer, const RendererScenes& rendererScenes, const SceneStateExecutor& sceneStateExecutor, const RendererEventCollector& rendererEventCollector, const FrameTimer& frameTimer, const SceneExpirationMonitor& expirationMonitor, IThreadAliveNotifier& notifier, const IRendererResourceCache* rendererResourceCache);
        virtual ~RendererSceneUpdaterFacade() override;

        virtual void handleSceneUpdate(SceneId sceneId, SceneUpdate&& update) override;
        virtual void handlePickEvent(SceneId sceneId, Vector2 coords) override;

        std::unordered_map<DisplayHandle, testing::StrictMock<RendererResourceManagerRefCountMock>*> m_resourceManagerMocks;

    protected:
        virtual std::unique_ptr<IRendererResourceManager> createResourceManager(
            IRenderBackend& renderBackend,
            AsyncEffectUploader& asyncEffectUploader,
            IEmbeddedCompositingManager& embeddedCompositingManager,
            DisplayHandle display,
            bool keepEffectsUploaded,
            uint64_t gpuCacheSize,
            IBinaryShaderCache* binaryShaderCache) override;
    };
}

#endif
