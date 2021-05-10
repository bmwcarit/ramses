//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRENDERERSCENEUPDATER_H
#define RAMSES_IRENDERERSCENEUPDATER_H

#include "RendererAPI/Types.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/WaylandIviSurfaceId.h"
#include "SceneAPI/TextureEnums.h"
#include "SceneAPI/DataSlot.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector4.h"

namespace ramses_internal
{
    struct SceneUpdate;
    class DisplayConfig;
    class IBinaryShaderCache;

    class IRendererSceneUpdater
    {
    public:
        virtual void handleSceneUpdate(SceneId sceneId, SceneUpdate&& sceneUpdate) = 0;
        virtual void createDisplayContext(const DisplayConfig& displayConfig, IBinaryShaderCache* binaryShaderCache) = 0;
        virtual void destroyDisplayContext() = 0;
        virtual void handleScenePublished(SceneId sceneId, EScenePublicationMode mode) = 0;
        virtual void handleSceneUnpublished(SceneId sceneId) = 0;
        virtual void handleSceneReceived(const SceneInfo& sceneInfo) = 0;
        virtual bool handleBufferCreateRequest(OffscreenBufferHandle buffer, UInt32 width, UInt32 height, UInt32 sampleCount, Bool isDoubleBuffered, ERenderBufferType depthStencilBufferType) = 0;
        virtual bool handleBufferDestroyRequest(OffscreenBufferHandle buffer) = 0;
        virtual bool handleBufferCreateRequest(StreamBufferHandle buffer, WaylandIviSurfaceId source) = 0;
        virtual bool handleBufferDestroyRequest(StreamBufferHandle buffer) = 0;
        virtual bool setStreamBufferState(StreamBufferHandle buffer, bool newState) = 0;
        virtual void handleSetClearFlags(OffscreenBufferHandle buffer, uint32_t clearFlags) = 0;
        virtual void handleSetClearColor(OffscreenBufferHandle buffer, const Vector4& clearColor) = 0;
        virtual void handleReadPixels(OffscreenBufferHandle buffer, ScreenshotInfo&& screenshotInfo) = 0;
        virtual void handlePickEvent(SceneId sceneId, Vector2 coordsNormalizedToBufferSize) = 0;
        virtual void handleSceneDataLinkRequest(SceneId providerSceneId, DataSlotId providerId, SceneId consumerSceneId, DataSlotId consumerId) = 0;
        virtual void handleBufferToSceneDataLinkRequest(OffscreenBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId) = 0;
        virtual void handleBufferToSceneDataLinkRequest(StreamBufferHandle buffer, SceneId consumerSceneId, DataSlotId consumerId) = 0;
        virtual void handleDataUnlinkRequest(SceneId consumerSceneId, DataSlotId consumerId) = 0;
        virtual void setLimitFlushesForceApply(UInt limitForPendingFlushesForceApply) = 0;
        virtual void setLimitFlushesForceUnsubscribe(UInt limitForPendingFlushesForceUnsubscribe) = 0;
        virtual void setSkippingOfUnmodifiedScenes(bool enable) = 0;
        virtual void logRendererInfo(ERendererLogTopic topic, bool verbose, NodeHandle nodeFilter) const = 0;

        virtual ~IRendererSceneUpdater() = default;
    };
}

#endif
