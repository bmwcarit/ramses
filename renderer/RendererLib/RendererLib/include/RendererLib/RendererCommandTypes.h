//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERCOMMANDTYPES_H
#define RAMSES_RENDERERCOMMANDTYPES_H

#include "SceneAPI/DataSlot.h"
#include "RendererAPI/Types.h"
#include "RendererLib/WarpingMeshData.h"
#include "RendererLib/DisplayConfig.h"
#include "CommandT.h"
#include "Scene/SceneActionCollection.h"
#include "Math3d/Vector3.h"
#include "RendererLogger.h"
#include "EKeyCode.h"
#include "EKeyEventType.h"
#include "EMouseEventType.h"
#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    enum ERendererCommandType
    {
        ERendererCommandType_Scene = 0,
        ERendererCommandType_SceneActions,
        ERendererCommandType_Display,
        ERendererCommandType_SceneMapping,
        ERendererCommandType_SceneRendering,
        ERendererCommandType_WarpingData,
        ERendererCommandType_ReadPixels,
        ERendererCommandType_SetClearColor,
        ERendererCommandType_DataLinking,
        ERendererCommandType_OffscreenBuffer,
        ERendererCommandType_RendererView,
        ERendererCommandType_Log,
        ERendererCommandType_Screenshot,
        ERendererCommandType_WindowEvent,
        ERendererCommandType_Compositor,
        ERendererCommandType_ConfirmationEcho,
        ERendererCommandType_FrameProfiler,
        ERendererCommandType_FrameTimerLimits,
        ERendererCommandType_SetFeature
    };

    enum ERendererCommand
    {
        // Scene and display control
        ERendererCommand_PublishedScene = 0,
        ERendererCommand_UnpublishedScene,
        ERendererCommand_ReceivedScene,
        ERendererCommand_SubscribeScene,
        ERendererCommand_UnsubscribeScene,
        ERendererCommand_CreateDisplay,
        ERendererCommand_DestroyDisplay,
        ERendererCommand_MapSceneToDisplay,
        ERendererCommand_UnmapSceneFromDisplays,
        ERendererCommand_ShowScene,
        ERendererCommand_HideScene,
        ERendererCommand_RelativeTranslation,
        ERendererCommand_AbsoluteTranslation,
        ERendererCommand_RelativeRotation,
        ERendererCommand_AbsoluteRotation,
        ERendererCommand_ResetRenderView,
        ERendererCommand_UpdateWarpingData,
        ERendererCommand_ReadPixels,
        ERendererCommand_SetClearColor,
        ERendererCommand_SceneActions,
        // Data linking
        ERendererCommand_LinkSceneData,
        ERendererCommand_LinkBufferToSceneData,
        ERendererCommand_UnlinkSceneData,
        ERendererCommand_CreateOffscreenBuffer,
        ERendererCommand_DestroyOffscreenBuffer,
        ERendererCommand_AssignSceneToOffscreenBuffer,
        ERendererCommand_AssignSceneToFramebuffer,
        // Logging
        ERendererCommand_LogRendererStatistics,
        ERendererCommand_LogRendererInfo,
        // Compositing
        ERendererCommand_SystemCompositorControllerListIviSurfaces,
        ERendererCommand_SystemCompositorControllerSetIviSurfaceVisibility,
        ERendererCommand_SystemCompositorControllerSetIviSurfaceOpacity,
        ERendererCommand_SystemCompositorControllerSetIviSurfaceDestRectangle,
        ERendererCommand_SystemCompositorControllerScreenshot,
        ERendererCommand_SystemCompositorControllerAddIviSurfaceToIviLayer,
        ERendererCommand_SystemCompositorControllerSetIviLayerVisibility,
        ERendererCommand_SystemCompositorControllerRemoveIviSurfaceFromIviLayer,
        ERendererCommand_SystemCompositorControllerDestroyIviSurface,
        // Miscellaneous
        ERendererCommand_SetFrameTimerLimits,
        ERendererCommand_SetLimits_FlushesForceApply,
        ERendererCommand_SetLimits_FlushesForceUnsubscribe,
        ERendererCommand_SetSkippingOfUnmodifiedBuffers,

        // THINK THREE TIMES BEFORE ADDING SOMETHING HERE!
        // This is not a bucket for junk to pass to the renderer because it is convenient
        ERendererCommand_ConfirmationEcho,
        ERendererCommand_FrameProfiler_Toggle,
        ERendererCommand_FrameProfiler_TimingGraphHeight,
        ERendererCommand_FrameProfiler_CounterGraphHeight,
        ERendererCommand_FrameProfiler_RegionFilterFlags,


        ERendererCommand_COUNT
    };

    class IResourceProvider;
    class IResourceUploader;

    typedef Command< ERendererCommandType > RendererCommand;

    struct SceneInfoCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(SceneInfoCommand, ERendererCommandType_Scene);

        SceneInfo sceneInformation;
        Guid clientID;
        Bool indirect = false;
    };

    struct SceneActionsCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(SceneActionsCommand, ERendererCommandType_SceneActions);

        SceneId sceneId;
        SceneActionCollection sceneActions;
    };

    struct DisplayCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(DisplayCommand, ERendererCommandType_Display);

        DisplayHandle displayHandle;
        DisplayConfig displayConfig;
        IResourceProvider* resourceProvider = nullptr;
        IResourceUploader* resourceUploader = nullptr;
    };

    struct SceneMappingCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(SceneMappingCommand, ERendererCommandType_SceneMapping);

        SceneId                 sceneId;
        DisplayHandle           displayHandle;
        Int32                   sceneRenderOrder;
    };

    struct SceneRenderCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(SceneRenderCommand, ERendererCommandType_SceneRendering);

        SceneId                 sceneId;
    };

    struct WarpingDataCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(WarpingDataCommand, ERendererCommandType_WarpingData);

        DisplayHandle           displayHandle;
        WarpingMeshData         warpingData;
    };

    struct ReadPixelsCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(ReadPixelsCommand, ERendererCommandType_ReadPixels);

        DisplayHandle           displayHandle;
        UInt32                  x;
        UInt32                  y;
        UInt32                  width;
        UInt32                  height;
        Bool                    fullScreen;
        Bool                    sendViaDLT;
        String                  filename;
    };

    struct SetClearColorCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(SetClearColorCommand, ERendererCommandType_SetClearColor);

        DisplayHandle           displayHandle;
        Vector4                 clearColor;
    };

    struct DataLinkCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(DataLinkCommand, ERendererCommandType_DataLinking);

        SceneId                providerScene;
        DataSlotId             providerData;
        SceneId                consumerScene;
        DataSlotId             consumerData;
        OffscreenBufferHandle  providerBuffer;
    };

    struct OffscreenBufferCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(OffscreenBufferCommand, ERendererCommandType_OffscreenBuffer);

        DisplayHandle          displayHandle;
        OffscreenBufferHandle  bufferHandle;
        UInt32                 bufferWidth = 0u;
        UInt32                 bufferHeight = 0u;
        SceneId                assignedScene;
        Bool                   interruptible = false;
    };

    struct RendererViewCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(RendererViewCommand, ERendererCommandType_RendererView);

        Vector3                 displayMovement;
    };

    struct LogCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(LogCommand, ERendererCommandType_Log);

        ERendererLogTopic       topic;
        Bool                    verbose;
        NodeHandle              nodeHandleFilter;
    };

    struct WindowEventCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(WindowEventCommand, ERendererCommandType_WindowEvent);

        DisplayHandle           display;

        EKeyEventType           keyEvent;
        UInt32                  keyModifier;
        EKeyCode                keyCode;

        EMouseEventType         mouseAction;
        Int32                   mousePosX;
        Int32                   mousePosY;
    };

    struct CompositorCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(CompositorCommand, ERendererCommandType_Compositor);

        WaylandIviSurfaceId waylandIviSurfaceId;
        Bool visibility = false;
        Float opacity = 1.f;
        Int32 x = 0;
        Int32 y = 0;
        Int32 width = 0;
        Int32 height = 0;
        WaylandIviLayerId waylandIviLayerId;
        String fileName;
        int32_t screenIviId = 0;
    };

    struct ConfirmationEchoCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(ConfirmationEchoCommand, ERendererCommandType_ConfirmationEcho);

        String text;
    };

    struct UpdateFrameProfilerCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(UpdateFrameProfilerCommand, ERendererCommandType_FrameProfiler);

        Bool toggleVisibility = false;
        UInt32 newCounterGraphHeight = 0;
        UInt32 newTimingGraphHeight = 0;
        UInt32 newRegionFilterFlags = 0;
    };


    struct SetFrameTimerLimitsCommmand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(SetFrameTimerLimitsCommmand, ERendererCommandType_FrameTimerLimits);

        UInt64 limitForClientResourcesUploadMicrosec = 0u;
        UInt64 limitForSceneResourcesUploadMicrosec = 0u;
        UInt64 limitForSceneActionsApplyMicrosec = 0u;
        UInt64 limitForOffscreenBufferRenderMicrosec = 0u;
        UInt limitForPendingFlushesForceApply = 0u;
        UInt limitForPendingFlushesForceUnsubscribe = 0u;
    };

    struct SetFeatureCommand : public RendererCommand
    {
        DEFINE_COMMAND_TYPE(SetFeatureCommand, ERendererCommandType_SetFeature);

        Bool enable = false;
    };

    static const Char* RendererCommandNames[] =
    {
        "ERendererCommand_PublishedScene",
        "ERendererCommand_UnpublishedScene",
        "ERendererCommand_ReceivedScene",
        "ERendererCommand_SubscribeScene",
        "ERendererCommand_UnsubscribeScene",
        "ERendererCommand_CreateDisplay",
        "ERendererCommand_DestroyDisplay",
        "ERendererCommand_MapSceneToDisplay",
        "ERendererCommand_UnmapSceneFromDisplays",
        "ERendererCommand_ShowScene",
        "ERendererCommand_HideScene",
        "ERendererCommand_RelativeTranslation",
        "ERendererCommand_AbsoluteTranslation",
        "ERendererCommand_RelativeRotation",
        "ERendererCommand_AbsoluteRotation",
        "ERendererCommand_ResetRenderView",
        "ERendererCommand_UpdateWarpingData",
        "ERendererCommand_ReadPixels",
        "ERendererCommand_SetClearColor",
        "ERendererCommand_SceneActions",
        "ERendererCommand_LinkSceneData",
        "ERendererCommand_LinkBufferToSceneData",
        "ERendererCommand_UnlinkSceneData",
        "ERendererCommand_CreateOffscreenBuffer",
        "ERendererCommand_DestroyOffscreenBuffer",
        "ERendererCommand_AssignSceneToOffscreenBuffer",
        "ERendererCommand_AssignSceneToFramebuffer",
        "ERendererCommand_LogRendererStatistics",
        "ERendererCommand_LogRendererInfo",
        "ERendererCommand_SystemCompositorControllerListIviSurfaces",
        "ERendererCommand_SystemCompositorControllerSetIviSurfaceVisibility",
        "ERendererCommand_SystemCompositorControllerSetIviSurfaceOpacity",
        "ERendererCommand_SystemCompositorControllerSetIviSurfaceDestRectangle",
        "ERendererCommand_SystemCompositorControllerScreenshot",
        "ERendererCommand_SystemCompositorControllerAddIviSurfaceToIviLayer",
        "ERendererCommand_SystemCompositorControllerSetIviLayerVisibility",
        "ERendererCommand_SystemCompositorControllerRemoveIviSurfaceFromIviLayer",
        "ERendererCommand_SystemCompositorControllerDestroyIviSurface",
        "ERendererCommand_SetFrameTimerLimits",
        "ERendererCommand_SetLimits_FlushesForceApply",
        "ERendererCommand_SetLimits_FlushesForceUnsubscribe",
        "ERendererCommand_SetSkippingOfUnmodifiedBuffers",
        "ERendererCommand_ConfirmationEcho",
        "ERendererCommand_FrameProfiler_Toggle",
        "ERendererCommand_FrameProfiler_TimingGraphHeight",
        "ERendererCommand_FrameProfiler_CounterGraphHeight",
        "ERendererCommand_FrameProfiler_RegionFilterFlags"
    };

    ENUM_TO_STRING(ERendererCommand, RendererCommandNames, ERendererCommand_COUNT);
}

#endif
