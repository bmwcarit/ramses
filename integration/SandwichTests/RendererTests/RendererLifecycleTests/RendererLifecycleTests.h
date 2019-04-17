//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERLIFECYCLETESTS_H
#define RAMSES_RENDERERLIFECYCLETESTS_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "Collections/StringOutputStream.h"
#include "Utils/LoggingUtils.h"
#include "Utils/StringUtils.h"

class RendererTestInstance;

namespace ramses_internal
{
    enum ELifecycleTest
    {
        ELifecycleTest_RenderScene = 0,
        ELifecycleTest_RecreateSceneWithSameId,
        ELifecycleTest_SaveLoadSceneFromFileThenRender,
        ELifecycleTest_SaveLoadSceneFromFileThenRender_Threaded,
        ELifecycleTest_DestroyAndRecreateRenderer,
        ELifecycleTest_DestroyRenderer_ChangeScene_ThenRecreateRenderer,
        ELifecycleTest_UnsubscribeRenderer_ChangeScene_ThenResubscribeRenderer,
        ELifecycleTest_ChangeScene_UnsubscribeRenderer_Flush_ThenResubscribeRenderer,
        ELifecycleTest_RAMSES2881_CreateRendererAfterScene,
        ELifecycleTest_DestroyDisplayAndRemapSceneToOtherDisplay,
        ELifecycleTest_RenderScene_Threaded,
        ELifecycleTest_RenderChangingScene_Threaded,
        ELifecycleTest_RenderScene_StartStopThreadMultipleTimes,
        ELifecycleTest_DestroyRendererWhileThreadRunning,
        ELifecycleTest_RendererUploadsResourcesIfIviSurfaceInvisible,
        ELifecycleTest_RendererUploadsResourcesIfIviSurfaceInvisibleInLoopModeUpdateOnly,
        ELifecycleTest_RemapScenesWithDynamicResourcesToOtherDisplay,
        ELifecycleTest_SceneCanReachShownStateWithLoopModeUpdateOnly_UsingDoOneLoop,
        ELifecycleTest_SceneCanReachShownStateWithLoopModeUpdateOnly_UsingRenderThread,
        ELifecycleTest_SceneCanReachShownStateWithLoopModeUpdateOnly_IfIviSurfaceInvisible,
        ELifecycleTest_DoesNotRenderToFramebufferInLoopModeUpdateOnly,
        ELifecycleTest_Republish_ThenChangeScene,
        ELifecycleTest_PollingFrameCallbacks_DoesNotBlockIfNoDisplaysExist,
        ELifecycleTest_PollingFrameCallbacks_BlocksIfDisplayNotReadyToRender,
        ELifecycleTest_PollingFrameCallbacks_BlocksIfAllDisplaysNotReadyToRender,
        ELifecycleTest_PollingFrameCallbacks_UnreadyDisplayDoesNotBlockReadyDisplay,
        ELifecycleTest_PollingFrameCallbacks_UnreadyDisplayDoesNotBlockReadyDisplay_DisplaysInOtherOrder,
        ELifecycleTest_PollingFrameCallbacks_ReadyDisplayDoesNotStarveOtherDisplay,
        ELifecycleTest_PollingFrameCallbacks_ReadyDisplayDoesNotStarveOtherDisplay_DisplaysInOtherOrder,
        ELifecycleTest_SceneNotExpiredWhenUpdatedAndSubscribed,
        ELifecycleTest_SceneExpiredWhenSubscribed,
        ELifecycleTest_SceneExpiredAndRecoveredWhenSubscribed,
        ELifecycleTest_SceneNotExpiredWhenUpdatedAndRendered,
        ELifecycleTest_SceneExpiredWhenRendered,
        ELifecycleTest_SceneNotExpiredWhenUpdatedWithEmptyFlushesAndRendered,
        ELifecycleTest_SceneExpirationCanBeDisabled_ConfidenceTest,
        ELifecycleTest_SceneExpiredAndRecoveredWhenRendered,
        ELifecycleTest_SceneExpiredWhenRenderedAndRecoveredAfterHidden,
        ELifecycleTest_ScenesExpireOneAfterAnother,

        ELifecycleTest_NUMBER_OF_ELEMENTS
    };

    static const char* LifecycleTestNames[] =
    {
        "RenderScene",
        "RecreateSceneWithSameId",
        "SaveLoadSceneFromFileThenRender",
        "SaveLoadSceneFromFileThenRender_Threaded",
        "DestroyAndRecreateRenderer",
        "DestroyRenderer_ChangeScene_ThenRecreateRenderer",
        "UnsubscribeRenderer_ChangeScene_ThenResubscribeRenderer",
        "ChangeScene_UnsubscribeRenderer_Flush_ThenResubscribeRenderer",
        "RAMSES2881_CreateRendererAfterScene",
        "DestroyDisplayAndRemapSceneToOtherDisplay",
        "RenderScene_Threaded",
        "RenderChangingScene_Threaded",
        "RenderScene_StartStopThreadMultipleTimes",
        "DestroyRendererWhileThreadRunning",
        "RendererUploadsResourcesIfIviSurfaceInvisible",
        "RendererUploadsResourcesIfIviSurfaceInvisibleInLoopModeUpdateOnly",
        "RemapScenesWithDynamicResourcesToOtherDisplay",
        "SceneCanReachShownStateWithLoopModeUpdateOnly_UsingDoOneLoop",
        "SceneCanReachShownStateWithLoopModeUpdateOnly_UsingRenderThread",
        "SceneCanReachShownStateWithLoopModeUpdateOnly_IfIviSurfaceInvisible",
        "DoesNotRenderToFramebufferInLoopModeUpdateOnly",
        "Republish_ThenChangeScene",
        "PollingFrameCallbacks_DoesNotBlockIfNoDisplaysExist",
        "PollingFrameCallbacks_BlocksIfDisplayNotReadyToRender",
        "PollingFrameCallbacks_BlocksIfAllDisplaysNotReadyToRender",
        "PollingFrameCallbacks_UnreadyDisplayDoesNotBlockReadyDisplay",
        "PollingFrameCallbacks_UnreadyDisplayDoesNotBlockReadyDisplay_DisplaysInOtherOrder",
        "PollingFrameCallbacks_ReadyDisplayDoesNotStarveOtherDisplay",
        "PollingFrameCallbacks_ReadyDisplayDoesNotStarveOtherDisplay_DisplaysInOtherOrder",
        "SceneNotExpiredWhenUpdatedAndSubscribed",
        "SceneExpiredWhenSubscribed",
        "SceneExpiredAndRecoveredWhenSubscribed",
        "SceneNotExpiredWhenUpdatedAndRendered",
        "SceneExpiredWhenRendered",
        "SceneNotExpiredWhenUpdatedWithEmptyFlushesAndRendered",
        "SceneExpirationCanBeDisabled_ConfidenceTest",
        "SceneExpiredAndRecoveredWhenRendered",
        "SceneExpiredWhenRenderedAndRecoveredAfterHidden",
        "ScenesExpireOneAfterAnother"
    };

    ENUM_TO_STRING(ELifecycleTest, LifecycleTestNames, ELifecycleTest_NUMBER_OF_ELEMENTS);

    class RendererLifecycleTests
    {
    public:
        RendererLifecycleTests(const ramses_internal::StringVector& filterIn, const ramses_internal::StringVector& filterOut, int32_t argc, const char* argv[]);

        bool runTests();
        void logReport();
        void writeReportToFile(ramses_internal::String fileName);

    private:
        bool isTestFiltered(ELifecycleTest test) const;

        bool runTest(ELifecycleTest testId);


        ramses::displayId_t createDisplayForWindow(RendererTestInstance& renderer, uint32_t iviSurfaceIdOffset = 0u, bool iviWindowStartVisible = true);
        bool checkScreenshot(RendererTestInstance& renderer, ramses::displayId_t display, const char* screenshotFile);

        const ramses_internal::StringVector m_filterIn;
        const ramses_internal::StringVector m_filterOut;

        ramses_internal::StringOutputStream m_testReport;

        ramses::RamsesFrameworkConfig m_frameworkConfig;

        static const uint32_t WindowX = 0u;
        static const uint32_t WindowY = 0u;
        static const uint32_t WindowWidth = 128u;
        static const uint32_t WindowHeight = 64u;
    };
}

#endif
