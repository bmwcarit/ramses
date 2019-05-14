//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCOMMANDBUFFER_H
#define RAMSES_RENDERERCOMMANDBUFFER_H

#include "RendererLib/RendererCommands.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "RendererLogger.h"
#include "SceneAPI/Handles.h"

namespace ramses_internal
{
    class WarpingMeshData;
    class DisplayConfig;
    class IResourceProvider;
    class IResourceUploader;

    class RendererCommandBuffer : public RendererCommands
    {
    public:
        RendererCommandBuffer();

        // overwrite methods from RendererCommands
        void publishScene(SceneId sceneId, const Guid& clientID, EScenePublicationMode mode);
        void unpublishScene(SceneId sceneId);
        void receiveScene(const SceneInfo& sceneInfo);
        void subscribeScene(SceneId sceneId);
        void unsubscribeScene(SceneId sceneId, bool indirect);
        void enqueueActionsForScene(SceneId sceneId, SceneActionCollection&& newActions);

        void createDisplay(const DisplayConfig& displayConfig, IResourceProvider& resourceProvider, IResourceUploader& resourceUploader, DisplayHandle handle);
        void destroyDisplay(DisplayHandle handle);

        void mapSceneToDisplay(SceneId sceneId, DisplayHandle displayHandle, Int32 sceneRenderOrder);
        void unmapScene(SceneId sceneId);

        void updateWarpingData(DisplayHandle displayHandle, const WarpingMeshData& warpingData);
        void readPixels(DisplayHandle displayHandle, const String& filename, Bool fullScreen, UInt32 x, UInt32 y, UInt32 width, UInt32 height, Bool sendViaDLT = false);
        void setClearColor(DisplayHandle displayHandle, const Vector4& color);

        void linkSceneData(SceneId providerSceneId, DataSlotId providerDataSlotId, SceneId consumerSceneId, DataSlotId consumerDataSlotId);
        void unlinkSceneData(SceneId consumerSceneId, DataSlotId consumerDataSlotId);

        void moveView       (const Vector3& offset);
        void setViewPosition(const Vector3& position);
        void rotateView     (const Vector3& rotationDiff);
        void setViewRotation(const Vector3& rotation);
        void resetView();

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

        void setFrameTimerLimits(UInt64 limitForSceneResourcesUploadMicrosec, UInt64 limitForClientResourcesUploadMicrosec, UInt64 limitForSceneActionsApplyMicrosec, UInt64 limitForOffscreenBufferRenderMicrosec);
        void setLimitsFlushesForceApply(UInt limitFlushesForceApply);
        void setLimitsFlushesForceUnsubscribe(UInt limitFlushesForceUnsubscribe);
        void setSkippingOfUnmodifiedBuffers(Bool enable);

        // own functions
        void lock();
        void unlock();

        void addCommands(const RendererCommands& commands);

    private:
        PlatformLock m_lock;
    };
}

#endif
