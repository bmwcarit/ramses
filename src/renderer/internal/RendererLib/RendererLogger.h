//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Core/Utils/LogContext.h"
#include "internal/Core/Utils/LoggingUtils.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"
#include "internal/SceneGraph/SceneAPI/Handles.h"
#include "internal/SceneGraph/SceneAPI/EDataType.h"
#include "internal/SceneGraph/SceneAPI/EDataSlotType.h"
#include "internal/RendererLib/Types.h"

#include <cstdint>
#include <string_view>

namespace ramses::internal
{
    class RendererSceneUpdater;
    class RendererLogContext;
    class RendererScenes;
    class SceneLinksManager;
    class SceneLinks;
    class RendererCachedScene;
    struct DisplayBufferInfo;

    class RendererLogger
    {
    public:
        RendererLogger() = delete;
        static void LogTopic(const RendererSceneUpdater& updater, ERendererLogTopic topic, bool verbose, NodeHandle nodeHandleFilter = NodeHandle::Invalid());

    private:
        static void LogSceneStates(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogDisplays(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogDisplayBuffer(const RendererSceneUpdater& updater, DeviceResourceHandle bufferHandle, const DisplayBufferInfo& dispBufferInfo, RendererLogContext& context);
        static void LogClientResources(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogSceneResources(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogMissingResources(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogRenderQueue(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogRenderQueueOfScenesRenderedToBuffer(const RendererSceneUpdater& updater, RendererLogContext& context, DeviceResourceHandle buffer);
        static void LogLinks(const RendererScenes& scenes, RendererLogContext& context);
        static void LogEmbeddedCompositor(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogEventQueue(const RendererSceneUpdater& updater, RendererLogContext& context);
        static void LogPeriodicInfo(const RendererSceneUpdater& updater);
        static void LogReferencedScenes(const RendererSceneUpdater& updater, RendererLogContext& context);

        static void LogProvider(const RendererScenes& scenes, RendererLogContext& context, const RendererCachedScene& scene, const DataSlotHandle slotHandle, bool asLinked = false);
        static void LogConsumer(const RendererScenes& scenes, RendererLogContext& context, const RendererCachedScene& scene, const DataSlotHandle slotHandle, bool asLinked = false);
        static void LogSpecialSlotInfo(RendererLogContext& context, const RendererCachedScene& scene, const DataSlotHandle slotHandle);

        static const SceneLinks& GetSceneLinks(const SceneLinksManager& linkManager, const EDataSlotType type);
        static EDataType GetDataTypeForSlot(const RendererCachedScene& scene, const DataSlotHandle slotHandle);
        static OffscreenBufferHandle GetOffscreenBufferLinkedToConsumer(const RendererScenes& scenes, SceneId consumerScene, DataSlotHandle consumerSlot);
        static StreamBufferHandle GetStreamBufferLinkedToConsumer(const RendererScenes& scenes, SceneId consumerScene, DataSlotHandle consumerSlot);

        static void StartSection(std::string_view name, RendererLogContext& context);
        static void EndSection(std::string_view name, RendererLogContext& context);

        static void Log(const LogContext& logContext, const RendererLogContext& context);
    };
}
