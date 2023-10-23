//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/TextureEnums.h"
#include "internal/RendererLib/Types.h"
#include "internal/SceneGraph/Scene/EScenePublicationMode.h"
#include "internal/Components/SceneUpdate.h"
#include "internal/RendererLib/DisplayConfig.h"
#include "internal/PlatformAbstraction/VariantWrapper.h"
#include "impl/DataTypesImpl.h"

#include <vector>
#include <string>

namespace ramses::internal
{
    class IBinaryShaderCache;

    namespace RendererCommand
    {
        struct ScenePublished
        {
            SceneId scene;
            EScenePublicationMode publicationMode = EScenePublicationMode::LocalAndRemote;
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
            RendererSceneState state = RendererSceneState::Unavailable;
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
            int32_t renderOrder = 0;
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

        struct LinkExternalBuffer
        {
            ExternalBufferHandle providerBuffer;
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
            glm::vec2 coordsNormalizedToBufferSize;
        };

        struct CreateDisplay
        {
            DisplayHandle display;
            DisplayConfig config;
            IBinaryShaderCache* binaryShaderCache{nullptr};
        };

        struct DestroyDisplay
        {
            DisplayHandle display;
        };

        struct CreateOffscreenBuffer
        {
            DisplayHandle display;
            OffscreenBufferHandle offscreenBuffer;
            uint32_t width = 0;
            uint32_t height = 0;
            uint32_t sampleCount = 0;
            bool interruptible = false;
            EDepthBufferType depthStencilBufferType = EDepthBufferType::None;
        };

        struct CreateDmaOffscreenBuffer
        {
            DisplayHandle display;
            OffscreenBufferHandle offscreenBuffer;
            uint32_t width = 0;
            uint32_t height = 0;
            DmaBufferFourccFormat dmaBufferFourccFormat;
            DmaBufferUsageFlags dmaBufferUsageFlags;
            DmaBufferModifiers dmaBufferModifiers;
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

        struct CreateExternalBuffer
        {
            DisplayHandle display;
            ExternalBufferHandle externalBuffer;
        };

        struct DestroyExternalBuffer
        {
            DisplayHandle display;
            ExternalBufferHandle externalBuffer;
        };

        struct SetClearFlags
        {
            DisplayHandle display;
            OffscreenBufferHandle offscreenBuffer;
            ClearFlags clearFlags;
        };

        struct SetClearColor
        {
            DisplayHandle display;
            OffscreenBufferHandle offscreenBuffer;
            glm::vec4 clearColor;
        };

        struct SetExterallyOwnedWindowSize
        {
            DisplayHandle display;
            uint32_t width = 0;
            uint32_t height = 0;
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
            std::string filename;
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
            ERendererLogTopic topic = ERendererLogTopic::All;
            bool verbose = false;
            NodeHandle nodeFilter;
        };

        struct SCListIviSurfaces
        {
            bool _dummyValue = false; // work around unsolved gcc bug https://bugzilla.redhat.com/show_bug.cgi?id=1507359
        };

        struct SCSetIviSurfaceVisibility
        {
            WaylandIviSurfaceId surface;
            bool visibility = false;
        };

        struct SCSetIviSurfaceOpacity
        {
            WaylandIviSurfaceId surface;
            float opacity = NAN;
        };

        struct SCSetIviSurfaceDestRectangle
        {
            WaylandIviSurfaceId surface;
            int32_t x = 0;
            int32_t y = 0;
            int32_t width = 0;
            int32_t height = 0;
        };

        struct SCScreenshot
        {
            int32_t screenId = 0;
            std::string filename;
        };

        struct SCAddIviSurfaceToIviLayer
        {
            WaylandIviSurfaceId surface;
            WaylandIviLayerId layer;
        };

        struct SCSetIviLayerVisibility
        {
            WaylandIviLayerId layer;
            bool visibility = false;
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
            uint64_t limitForSceneResourcesUploadMicrosec = 0;
            uint64_t limitForResourcesUploadMicrosec = 0;
            uint64_t limitForOffscreenBufferRenderMicrosec = 0;
        };

        struct SetLimits_FlushesForceApply
        {
            uint32_t limitForPendingFlushesForceApply = 0;
        };

        struct SetLimits_FlushesForceUnsubscribe
        {
            uint32_t limitForPendingFlushesForceUnsubscribe = 0;
        };

        struct ConfirmationEcho
        {
            DisplayHandle display;
            std::string text;
        };

        using Variant = std::variant<
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
            LinkExternalBuffer,
            UnlinkData,
            PickEvent,
            CreateDisplay,
            DestroyDisplay,
            CreateOffscreenBuffer,
            CreateDmaOffscreenBuffer,
            DestroyOffscreenBuffer,
            CreateStreamBuffer,
            DestroyStreamBuffer,
            CreateExternalBuffer,
            DestroyExternalBuffer,
            SetClearFlags,
            SetClearColor,
            SetExterallyOwnedWindowSize,
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
            ConfirmationEcho
        >;
    }

    using RendererCommands = std::vector<RendererCommand::Variant>;
}
