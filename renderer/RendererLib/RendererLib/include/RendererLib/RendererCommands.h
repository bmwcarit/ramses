//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCOMMANDS_H
#define RAMSES_RENDERERCOMMANDS_H

#include "SceneAPI/Handles.h"
#include "RendererAPI/Types.h"
#include "Scene/EScenePublicationMode.h"
#include "Components/SceneUpdate.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/WarpingMeshData.h"
#include "Collections/String.h"
#include "Math3d/Vector2.h"
#include "PlatformAbstraction/VariantWrapper.h"
#include <vector>

namespace ramses_internal
{
    class IBinaryShaderCache;

    namespace RendererCommand
    {
        struct ScenePublished
        {
            SceneId scene;
            EScenePublicationMode publicationMode;
        };

        struct SceneUnpublished
        {
            SceneId scene;
        };

        struct ReceiveScene
        {
            SceneInfo info;
        };

        struct UpdateScene
        {
            SceneId scene;
            SceneUpdate updateData;
        };

        struct SetSceneState
        {
            SceneId scene;
            RendererSceneState state;
        };

        struct SetSceneMapping
        {
            SceneId scene;
            DisplayHandle display;
        };

        struct SetSceneDisplayBufferAssignment
        {
            SceneId scene;
            OffscreenBufferHandle buffer;
            int32_t renderOrder;
        };

        struct LinkData
        {
            SceneId providerScene;
            DataSlotId providerData;
            SceneId consumerScene;
            DataSlotId consumerData;
        };

        struct LinkOffscreenBuffer
        {
            OffscreenBufferHandle providerBuffer;
            SceneId consumerScene;
            DataSlotId consumerData;
        };

        struct LinkStreamBuffer
        {
            StreamBufferHandle providerBuffer;
            SceneId consumerScene;
            DataSlotId consumerData;
        };

        struct UnlinkData
        {
            SceneId consumerScene;
            DataSlotId consumerData;
        };

        struct PickEvent
        {
            SceneId scene;
            Vector2 coordsNormalizedToBufferSize;
        };

        struct CreateDisplay
        {
            DisplayHandle display;
            DisplayConfig config;
            IBinaryShaderCache* binaryShaderCache;
        };

        struct DestroyDisplay
        {
            DisplayHandle display;
        };

        struct CreateOffscreenBuffer
        {
            DisplayHandle display;
            OffscreenBufferHandle offscreenBuffer;
            uint32_t width;
            uint32_t height;
            uint32_t sampleCount;
            bool interruptible;
        };

        struct DestroyOffscreenBuffer
        {
            DisplayHandle display;
            OffscreenBufferHandle offscreenBuffer;
        };

        struct CreateStreamBuffer
        {
            DisplayHandle display;
            StreamBufferHandle streamBuffer;
            WaylandIviSurfaceId source;
        };

        struct DestroyStreamBuffer
        {
            DisplayHandle display;
            StreamBufferHandle streamBuffer;
        };

        struct SetStreamBufferState
        {
            DisplayHandle display;
            StreamBufferHandle streamBuffer;
            bool newState;
        };

        struct SetClearColor
        {
            DisplayHandle display;
            OffscreenBufferHandle offscreenBuffer;
            Vector4 clearColor;
        };

        struct UpdateWarpingData
        {
            DisplayHandle display;
            WarpingMeshData data;
        };

        struct ReadPixels
        {
            DisplayHandle display;
            OffscreenBufferHandle offscreenBuffer;
            uint32_t offsetX;
            uint32_t offsetY;
            uint32_t width;
            uint32_t height;
            bool fullScreen;
            bool sendViaDLT;
            String filename;
        };

        struct SetSkippingOfUnmodifiedBuffers
        {
            bool enable;
        };

        struct LogStatistics
        {
            bool _dummyValue = false; // work around unsolved gcc bug https://bugzilla.redhat.com/show_bug.cgi?id=1507359
        };

        struct LogInfo
        {
            ERendererLogTopic topic;
            bool verbose;
            NodeHandle nodeFilter;
        };

        struct SCListIviSurfaces
        {
            bool _dummyValue = false; // work around unsolved gcc bug https://bugzilla.redhat.com/show_bug.cgi?id=1507359
        };

        struct SCSetIviSurfaceVisibility
        {
            WaylandIviSurfaceId surface;
            bool visibility;
        };

        struct SCSetIviSurfaceOpacity
        {
            WaylandIviSurfaceId surface;
            float opacity;
        };

        struct SCSetIviSurfaceDestRectangle
        {
            WaylandIviSurfaceId surface;
            int32_t x;
            int32_t y;
            int32_t width;
            int32_t height;
        };

        struct SCScreenshot
        {
            int32_t screenId;
            String filename;
        };

        struct SCAddIviSurfaceToIviLayer
        {
            WaylandIviSurfaceId surface;
            WaylandIviLayerId layer;
        };

        struct SCSetIviLayerVisibility
        {
            WaylandIviLayerId layer;
            bool visibility;
        };

        struct SCRemoveIviSurfaceFromIviLayer
        {
            WaylandIviSurfaceId surface;
            WaylandIviLayerId layer;
        };

        struct SCDestroyIviSurface
        {
            WaylandIviSurfaceId surface;
        };

        struct SetLimits_FrameBudgets
        {
            uint64_t limitForSceneResourcesUploadMicrosec;
            uint64_t limitForResourcesUploadMicrosec;
            uint64_t limitForOffscreenBufferRenderMicrosec;
        };

        struct SetLimits_FlushesForceApply
        {
            uint32_t limitForPendingFlushesForceApply;
        };

        struct SetLimits_FlushesForceUnsubscribe
        {
            uint32_t limitForPendingFlushesForceUnsubscribe;
        };

        struct FrameProfiler_Toggle
        {
            bool toggle;
        };

        struct FrameProfiler_TimingGraphHeight
        {
            uint32_t height;
        };

        struct FrameProfiler_CounterGraphHeight
        {
            uint32_t height;
        };

        struct FrameProfiler_RegionFilterFlags
        {
            uint32_t flags;
        };

        struct ConfirmationEcho
        {
            String text;
        };

        using Variant = absl::variant<
            ScenePublished,
            SceneUnpublished,
            ReceiveScene,
            UpdateScene,
            SetSceneState,
            SetSceneMapping,
            SetSceneDisplayBufferAssignment,
            LinkData,
            LinkOffscreenBuffer,
            LinkStreamBuffer,
            UnlinkData,
            PickEvent,
            CreateDisplay,
            DestroyDisplay,
            CreateOffscreenBuffer,
            DestroyOffscreenBuffer,
            CreateStreamBuffer,
            DestroyStreamBuffer,
            SetStreamBufferState,
            SetClearColor,
            UpdateWarpingData,
            ReadPixels,
            SetSkippingOfUnmodifiedBuffers,
            LogStatistics,
            LogInfo,
            SCListIviSurfaces,
            SCSetIviSurfaceVisibility,
            SCSetIviSurfaceOpacity,
            SCSetIviSurfaceDestRectangle,
            SCScreenshot,
            SCAddIviSurfaceToIviLayer,
            SCSetIviLayerVisibility,
            SCRemoveIviSurfaceFromIviLayer,
            SCDestroyIviSurface,
            SetLimits_FrameBudgets,
            SetLimits_FlushesForceApply,
            SetLimits_FlushesForceUnsubscribe,
            FrameProfiler_Toggle,
            FrameProfiler_TimingGraphHeight,
            FrameProfiler_CounterGraphHeight,
            FrameProfiler_RegionFilterFlags,
            ConfirmationEcho
        >;
    }

    using RendererCommands = std::vector<RendererCommand::Variant>;
}

#endif
