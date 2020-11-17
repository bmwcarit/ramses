//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCOMMANDS_H
#define RAMSES_RENDERERCOMMANDS_H

#include "RendererLib/RendererCommandContainer.h"
#include "RendererLogger.h"
#include "SceneAPI/Handles.h"
#include "Scene/EScenePublicationMode.h"

namespace ramses_internal
{
    class WarpingMeshData;
    class DisplayConfig;
    class IResourceUploader;

    class RendererCommands
    {
    public:
        void publishScene(SceneId sceneId, EScenePublicationMode mode);
        void unpublishScene(SceneId sceneId);
        void receiveScene(const SceneInfo& sceneInfo);

        void setSceneState(SceneId sceneId, RendererSceneState state);
        void setSceneMapping(SceneId sceneId, DisplayHandle display);
        void setSceneDisplayBufferAssignment(SceneId sceneId, OffscreenBufferHandle displayBuffer, int32_t sceneRenderOrder);

        void subscribeScene(SceneId sceneId);
        void unsubscribeScene(SceneId sceneId, bool indirect);
        void mapSceneToDisplay(SceneId sceneId, DisplayHandle displayHandle);
        void unmapScene(SceneId sceneId);
        void showScene(SceneId sceneId);
        void hideScene(SceneId sceneId);

        void enqueueActionsForScene(SceneId sceneId, SceneUpdate&& sceneUpdate);

        void createDisplay(const DisplayConfig& displayConfig, IResourceUploader& resourceUploader, DisplayHandle handle);
        void destroyDisplay(DisplayHandle handle);

        void updateWarpingData(DisplayHandle displayHandle, const WarpingMeshData& warpingData);
        void readPixels(DisplayHandle displayHandle, OffscreenBufferHandle obHandle, const String& filename, Bool fullScreen, UInt32 x, UInt32 y, UInt32 width, UInt32 height, Bool sendViaDLT = false);
        void setClearColor(DisplayHandle displayHandle, OffscreenBufferHandle obHandle, const Vector4& color);

        void linkSceneData(const SceneId providerSceneId, DataSlotId providerDataSlotId, SceneId consumerSceneId, DataSlotId consumerDataSlotId);
        void linkBufferToSceneData(OffscreenBufferHandle providerBuffer, SceneId consumerSceneId, DataSlotId consumerDataSlotId);
        void unlinkSceneData(const SceneId consumerSceneId, DataSlotId consumerDataSlotId);

        void createOffscreenBuffer(OffscreenBufferHandle buffer, DisplayHandle display, UInt32 width, UInt32 height, UInt32 sampleCount, Bool interruptible);
        void destroyOffscreenBuffer(OffscreenBufferHandle buffer, DisplayHandle display);
        void assignSceneToDisplayBuffer(SceneId sceneId, OffscreenBufferHandle buffer, Int32 sceneRenderOrder);

        void logStatistics      ();
        void logRendererInfo    (ERendererLogTopic topic, Bool verbose, NodeHandle nodeHandleFilter);

        void systemCompositorControllerListIviSurfaces();
        void systemCompositorControllerSetIviSurfaceVisibility(WaylandIviSurfaceId surfaceId, Bool visibility);
        void systemCompositorControllerSetIviSurfaceOpacity(WaylandIviSurfaceId surfaceId, Float opacity);
        void systemCompositorControllerSetIviSurfaceDestRectangle(WaylandIviSurfaceId surfaceId, Int32 x, Int32 y, Int32 width, Int32 height);
        void systemCompositorControllerSetIviLayerVisibility(WaylandIviLayerId layerId, Bool visibility);
        void systemCompositorControllerScreenshot(const String& fileName, int32_t screenIviId);
        void systemCompositorControllerAddIviSurfaceToIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId);
        void systemCompositorControllerRemoveIviSurfaceFromIviLayer(WaylandIviSurfaceId surfaceId, WaylandIviLayerId layerId);
        void systemCompositorControllerDestroyIviSurface(WaylandIviSurfaceId surfaceId);

        void confirmationEcho(const String& text);

        void toggleFrameProfilerVisibility(Bool setVisibleInsteadOfToggle);
        void setFrameProfilerTimingGraphHeight(UInt32 height);
        void setFrameProfilerCounterGraphHeight(UInt32 height);
        void setFrameProfilerFilteredRegionFlags(UInt32 flags);

        void setSkippingOfUnmodifiedBuffers(Bool enable);
        void handlePickEvent(SceneId sceneId, Vector2 coordsNormalizedToBufferSize);
        void setFrameTimerLimits(UInt64 limitForSceneResourcesUpload, UInt64 limitForClientResourcesUploadMicrosec, UInt64 limitForOffscreenBufferRenderMicrosec);
        void setForceApplyPendingFlushesLimit(UInt maximumPendingFlushes);
        void setForceUnsubscribeLimits(UInt maximumPendingFlushes);

        const RendererCommandContainer& getCommands() const;
        void swapCommandContainer(RendererCommandContainer& commandContainer);
        void clear();

    private:
        RendererCommandContainer m_commands;
    };
}

#endif
