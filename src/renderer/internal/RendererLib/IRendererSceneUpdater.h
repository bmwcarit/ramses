//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/RendererLib/Types.h"
#include "internal/RendererLib/RendererCommands.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include "internal/SceneGraph/SceneAPI/DataSlot.h"
#include "internal/SceneGraph/SceneAPI/RenderState.h"
#include "impl/DataTypesImpl.h"
#include "ramses/renderer/Types.h"

namespace ramses::internal
{
    struct SceneUpdate;
    class DisplayConfigData;
    class IBinaryShaderCache;

    class IRendererSceneUpdater
    {
    public:
        virtual void handleSceneUpdate(SceneId sceneId, SceneUpdate&& sceneUpdate) = 0;
        virtual void createDisplayContext(const DisplayConfigData& displayConfig, IBinaryShaderCache* binaryShaderCache) = 0;
        virtual void destroyDisplayContext() = 0;
        virtual void handleScenePublished(SceneId sceneId, EScenePublicationMode mode) = 0;
        virtual void handleSceneUnpublished(SceneId sceneId) = 0;
        virtual void handleSceneReceived(const SceneInfo& sceneInfo) = 0;
        virtual bool handleBufferCreateRequest(OffscreenBufferHandle buffer, uint32_t width, uint32_t height, uint32_t sampleCount, bool isDoubleBuffered, EDepthBufferType depthStencilBufferType) = 0;
        virtual bool handleDmaBufferCreateRequest(OffscreenBufferHandle buffer, uint32_t width, uint32_t height, DmaBufferFourccFormat dmaBufferFourccFormat, DmaBufferUsageFlags dmaBufferUsageFlags, DmaBufferModifiers dmaBufferModifiers) = 0;
        virtual bool handleBufferDestroyRequest(OffscreenBufferHandle buffer) = 0;
        virtual bool handleBufferCreateRequest(StreamBufferHandle buffer, WaylandIviSurfaceId source) = 0;
        virtual bool handleBufferDestroyRequest(StreamBufferHandle buffer) = 0;
        virtual bool handleExternalBufferCreateRequest(ExternalBufferHandle buffer) = 0;
        virtual bool handleExternalBufferDestroyRequest(ExternalBufferHandle buffer) = 0;
        virtual void handleSetClearFlags(OffscreenBufferHandle buffer, ClearFlags clearFlags) = 0;
        virtual void handleSetClearColor(OffscreenBufferHandle buffer, const glm::vec4& clearColor) = 0;
        virtual void handleSetExternallyOwnedWindowSize(uint32_t width, uint32_t height) = 0;
        virtual void handleReadPixels(OffscreenBufferHandle buffer, ScreenshotInfo&& screenshotInfo) = 0;
        virtual void handlePickEvent(SceneId sceneId, glm::vec2 coordsNormalizedToBufferSize) = 0;
        virtual void handleSceneDataLinkRequest(SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId) = 0;
        virtual void handleBufferToSceneDataLinkRequest(OffscreenBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId) = 0;
        virtual void handleBufferToSceneDataLinkRequest(StreamBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId) = 0;
        virtual void handleBufferToSceneDataLinkRequest(ExternalBufferHandle externalBuffer, SceneId consumerSceneId, DataSlotId consumerId) = 0;
        virtual void handleDataUnlinkRequest(SceneId consumerSceneId, DataSlotId consumerId) = 0;
        virtual void setLimitFlushesForceApply(size_t limitForPendingFlushesForceApply) = 0;
        virtual void setLimitFlushesForceUnsubscribe(size_t limitForPendingFlushesForceUnsubscribe) = 0;
        virtual void setSkippingOfUnmodifiedScenes(bool enable) = 0;
        virtual void logRendererInfo(const RendererCommand::LogInfo& cmd) const = 0;

        virtual ~IRendererSceneUpdater() = default;
    };
}
