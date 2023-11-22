//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"
#include "internal/RendererLib/IRendererSceneUpdater.h"
#include "internal/Components/SceneUpdate.h"
#include "internal/RendererLib/DisplayConfig.h"
#include "internal/RendererLib/PlatformInterface/IBinaryShaderCache.h"

namespace ramses::internal
{
    class RendererSceneUpdaterMock : public IRendererSceneUpdater
    {
    public:
        RendererSceneUpdaterMock();
        ~RendererSceneUpdaterMock() override;

        MOCK_METHOD(void, handleSceneUpdate, (SceneId sceneId, SceneUpdate&& sceneUpdate), (override));
        MOCK_METHOD(void, createDisplayContext, (const DisplayConfig& displayConfig, IBinaryShaderCache*), (override));
        MOCK_METHOD(void, destroyDisplayContext, (), (override));
        MOCK_METHOD(void, handleScenePublished, (SceneId sceneId, EScenePublicationMode mode), (override));
        MOCK_METHOD(void, handleSceneUnpublished, (SceneId sceneId), (override));
        MOCK_METHOD(void, handleSceneReceived, (const SceneInfo& sceneInfo), (override));
        MOCK_METHOD(bool, handleBufferCreateRequest, (OffscreenBufferHandle buffer, uint32_t width, uint32_t height, uint32_t sampleCount, bool isDoubleBuffered, EDepthBufferType depthStencilBufferType), (override));
        MOCK_METHOD(bool, handleDmaBufferCreateRequest, (OffscreenBufferHandle buffer, uint32_t width, uint32_t height, DmaBufferFourccFormat format, DmaBufferUsageFlags dmaBufferUsageFlags, DmaBufferModifiers dmaBufferModifiers), (override));
        MOCK_METHOD(bool, handleBufferDestroyRequest, (OffscreenBufferHandle buffer), (override));
        MOCK_METHOD(bool, handleBufferCreateRequest, (StreamBufferHandle buffer, WaylandIviSurfaceId source), (override));
        MOCK_METHOD(bool, handleBufferDestroyRequest, (StreamBufferHandle buffer), (override));
        MOCK_METHOD(bool, handleExternalBufferCreateRequest, (ExternalBufferHandle), (override));
        MOCK_METHOD(bool, handleExternalBufferDestroyRequest, (ExternalBufferHandle), (override));
        MOCK_METHOD(void, handleSetClearFlags, (OffscreenBufferHandle buffer, ClearFlags), (override));
        MOCK_METHOD(void, handleSetClearColor, (OffscreenBufferHandle buffer, const glm::vec4& clearColor), (override));
        MOCK_METHOD(void, handleSetExternallyOwnedWindowSize, (uint32_t, uint32_t), (override));
        MOCK_METHOD(void, handleReadPixels, (OffscreenBufferHandle buffer, ScreenshotInfo&& screenshotInfo), (override));
        MOCK_METHOD(void, handlePickEvent, (SceneId sceneId, glm::vec2 coordsNormalizedToBufferSize), (override));
        MOCK_METHOD(void, handleSceneDataLinkRequest, (SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId), (override));
        MOCK_METHOD(void, handleBufferToSceneDataLinkRequest, (OffscreenBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId), (override));
        MOCK_METHOD(void, handleBufferToSceneDataLinkRequest, (StreamBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId), (override));
        MOCK_METHOD(void, handleBufferToSceneDataLinkRequest, (ExternalBufferHandle, SceneId consumerSceneId, DataSlotId consumerId), (override));
        MOCK_METHOD(void, handleDataUnlinkRequest, (SceneId consumerSceneId, DataSlotId consumerId), (override));
        MOCK_METHOD(void, setLimitFlushesForceApply, (size_t limitForPendingFlushesForceApply), (override));
        MOCK_METHOD(void, setLimitFlushesForceUnsubscribe, (size_t limitForPendingFlushesForceUnsubscribe), (override));
        MOCK_METHOD(void, setSkippingOfUnmodifiedScenes, (bool enable), (override));
        MOCK_METHOD(void, logRendererInfo, (const RendererCommand::LogInfo& cmd), (const, override));
    };
}
