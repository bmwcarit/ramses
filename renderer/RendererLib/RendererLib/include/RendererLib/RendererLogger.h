//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERLOGGER_H
#define RAMSES_RENDERERLOGGER_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "Collections/String.h"
#include "Utils/LogContext.h"
#include "Utils/LoggingUtils.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/Handles.h"
#include "SceneAPI/EDataType.h"
#include "SceneAPI/EDataSlotType.h"
#include "RendererAPI/Types.h"

namespace ramses_internal
{
    class RendererSceneUpdater;
    class RendererLogContext;
    class RendererScenes;
    class SceneLinksManager;
    class SceneLinks;
    class RendererCachedScene;
    struct DisplayBufferInfo;

    enum ERendererLogTopic
    {
        ERendererLogTopic_Displays = 0,
        ERendererLogTopic_SceneStates,
        ERendererLogTopic_StreamTextures,
        ERendererLogTopic_Resources,
        ERendererLogTopic_MissingResources,
        ERendererLogTopic_RenderQueue,
        ERendererLogTopic_Links,
        ERendererLogTopic_EmbeddedCompositor,
        ERendererLogTopic_EventQueue,
        ERendererLogTopic_All,
        ERendererLogTopic_PeriodicLog,
        ERendererLogTopic_NUMBER_OF_ELEMENTS
    };

    static const char* RendererLogTopicNames[] =
    {
        "ERendererLogTopic_Displays",
        "ERendererLogTopic_SceneStates",
        "ERendererLogTopic_StreamTextures",
        "ERendererLogTopic_Resources",
        "ERendererLogTopic_MissingResources",
        "ERendererLogTopic_RenderQueue",
        "ERendererLogTopic_Links",
        "ERendererLogTopic_EmbeddedCompositor",
        "ERendererLogTopic_EventQueue",
        "ERendererLogTopic_All",
        "ERendererLogTopic_PeriodicLog"
    };

    ENUM_TO_STRING(ERendererLogTopic, RendererLogTopicNames, ERendererLogTopic_NUMBER_OF_ELEMENTS);

    class RendererLogger
    {
    public:
        static void LogTopic(const RendererSceneUpdater& updater, ERendererLogTopic topic, Bool verbose, NodeHandle nodeHandleFilter = NodeHandle::Invalid());

    private:
        RendererLogger();

        static void LogSceneStates(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogDisplays(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogDisplayBuffer(const RendererSceneUpdater& updater, DeviceResourceHandle bufferHandle, const DisplayBufferInfo& dispBufferInfo, RendererLogContext& context);
        static void LogClientResources(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogSceneResources(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogMissingResources(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogRenderQueue(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogRenderQueueOfScenesRenderedToBuffer(const RendererSceneUpdater& updater, RendererLogContext& context, DisplayHandle display, DeviceResourceHandle buffer);
        static void LogLinks(const RendererScenes& scenes, RendererLogContext& context);
        static void LogEmbeddedCompositor(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogEventQueue(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogPeriodicInfo(const RendererSceneUpdater& updater);
        static void LogStreamTextures(const RendererSceneUpdater& updater, RendererLogContext& context);

        static void LogProvider(const RendererScenes& scenes, RendererLogContext& context, const RendererCachedScene& scene, const DataSlotHandle slotHandle, bool asLinked = false);
        static void LogConsumer(const RendererScenes& scenes, RendererLogContext& context, const RendererCachedScene& scene, const DataSlotHandle slotHandle, bool asLinked = false);
        static void LogSpecialSlotInfo(RendererLogContext& context, const RendererCachedScene& scene, const DataSlotHandle slotHandle);

        static const SceneLinks& GetSceneLinks(const SceneLinksManager& linkManager, const EDataSlotType type);
        static EDataType GetDataTypeForSlot(const RendererCachedScene& scene, const DataSlotHandle slotHandle);
        static OffscreenBufferHandle GetOffscreenBufferLinkedToConsumer(const RendererScenes& scenes, SceneId consumerScene, DataSlotHandle consumerSlot);

        static void StartSection(const String& name, RendererLogContext& context);
        static void EndSection(const String& name, RendererLogContext& context);

        static void Log(const LogContext& logContext, const RendererLogContext& context);
    };
}

#endif
