//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENEUPDATERMOCK_H
#define RAMSES_RENDERERSCENEUPDATERMOCK_H

#include "gmock/gmock.h"
#include "RendererLib/IRendererSceneUpdater.h"
#include "Components/SceneUpdate.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererAPI/IBinaryShaderCache.h"

namespace ramses_internal
{
    class RendererSceneUpdaterMock : public IRendererSceneUpdater
    {
    public:
        RendererSceneUpdaterMock();
        virtual ~RendererSceneUpdaterMock() override;

        MOCK_METHOD(void, handleSceneUpdate, (SceneId sceneId, SceneUpdate&& sceneUpdate), (override));
        MOCK_METHOD(void, createDisplayContext, (const DisplayConfig& displayConfig, IBinaryShaderCache*), (override));
        MOCK_METHOD(void, destroyDisplayContext, (), (override));
        MOCK_METHOD(void, handleScenePublished, (SceneId sceneId, EScenePublicationMode mode), (override));
        MOCK_METHOD(void, handleSceneUnpublished, (SceneId sceneId), (override));
        MOCK_METHOD(void, handleSceneReceived, (const SceneInfo& sceneInfo), (override));
        MOCK_METHOD(bool, handleBufferCreateRequest, (OffscreenBufferHandle buffer, UInt32 width, UInt32 height, UInt32 sampleCount, Bool isDoubleBuffered, ERenderBufferType depthStencilBufferType), (override));
        MOCK_METHOD(bool, handleDmaBufferCreateRequest, (OffscreenBufferHandle buffer, UInt32 width, UInt32 height, DmaBufferFourccFormat format, DmaBufferUsageFlags dmaBufferUsageFlags, DmaBufferModifiers dmaBufferModifiers), (override));
        MOCK_METHOD(bool, handleBufferDestroyRequest, (OffscreenBufferHandle buffer), (override));
        MOCK_METHOD(bool, handleBufferCreateRequest, (StreamBufferHandle buffer, WaylandIviSurfaceId source), (override));
        MOCK_METHOD(bool, handleBufferDestroyRequest, (StreamBufferHandle buffer), (override));
        MOCK_METHOD(bool, setStreamBufferState, (StreamBufferHandle buffer, bool newState), (override));
        MOCK_METHOD(void, handleSetClearFlags, (OffscreenBufferHandle buffer, uint32_t), (override));
        MOCK_METHOD(void, handleSetClearColor, (OffscreenBufferHandle buffer, const Vector4& clearColor), (override));
        MOCK_METHOD(void, handleReadPixels, (OffscreenBufferHandle buffer, ScreenshotInfo&& screenshotInfo), (override));
        MOCK_METHOD(void, handlePickEvent, (SceneId sceneId, Vector2 coordsNormalizedToBufferSize), (override));
        MOCK_METHOD(void, handleSceneDataLinkRequest, (SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId), (override));
        MOCK_METHOD(void, handleBufferToSceneDataLinkRequest, (OffscreenBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId), (override));
        MOCK_METHOD(void, handleBufferToSceneDataLinkRequest, (StreamBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId), (override));
        MOCK_METHOD(void, handleDataUnlinkRequest, (SceneId consumerSceneId, DataSlotId consumerId), (override));
        MOCK_METHOD(void, setLimitFlushesForceApply, (UInt limitForPendingFlushesForceApply), (override));
        MOCK_METHOD(void, setLimitFlushesForceUnsubscribe, (UInt limitForPendingFlushesForceUnsubscribe), (override));
        MOCK_METHOD(void, setSkippingOfUnmodifiedScenes, (bool enable), (override));
        MOCK_METHOD(void, logRendererInfo, (ERendererLogTopic topic, bool verbose, NodeHandle nodeFilter), (const, override));
    };
}

#endif
