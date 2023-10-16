//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/client/RamsesClient.h"
#include "ramses/framework/RamsesFrameworkConfig.h"
#include "ramses/renderer/RendererConfig.h"
#include "ramses/renderer/DisplayConfig.h"

#include "StressTestRenderer.h"
#include "ResourceStressTestScene.h"

#include "ResourceStressTestSceneArray.h"

#include <cstdint>

namespace ramses::internal
{
    struct StressTestConfig
    {
        ramses::RamsesFrameworkConfig frameworkConfig{ ramses::EFeatureLevel_Latest };
        ramses::RendererConfig        rendererConfig;
        ramses::DisplayConfig         displayConfig;
        uint32_t durationEachTestSeconds = 0;
        uint32_t displayCount = 0;
        uint32_t sceneSetsPerDisplay = 0;
        bool disableSkippingOfFrames = false;
        uint32_t perFrameBudgetMSec_ClientRes = 0;
        uint32_t perFrameBudgetMSec_Rendering = 0;
        uint32_t renderablesBatchSizeForRenderingInterruption = 0;
    };

    enum EStressTestCaseId
    {
        //always begin with 0, do not break continuity
        EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush = 0,
        EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_LowRendererFPS,
        EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_ExtremelyLowRendererFPS,
        EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_MapSceneAfterAWhile,
        EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_MapSceneAfterAWhile_LowRendererFPS,
        EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_MapSceneAfterAWhile_ExtremelyLowRendererFPS,
        EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_RemapSceneAllTheTime,
        /*
        TODO Violin Add these tests too
        EStressTestCaseId_createDestroySameClientTexture,
        EStressTestCaseId_recreateResourcesEveryFrame_RecreateTheSameResourceEverySecondFrame,
        EStressTestCaseId_createDestroyStreamTextureWithSameFallbackTexture,
        EStressTestCaseId_createDestroyStreamTextureWithChangingFallbackTexture,
        EStressTestCaseId_createDestroyMultipleClientResources_WhileRemappingScene,
        EStressTestCaseId_createDestroyMultipleClientResources_WhileMovingSceneBetweenDisplays,
        EStressTestCaseId_createDestroyMultipleClientResources_WhileRecreatingRenderer,
        EStressTestCaseId_createDestroyMeshes_WhichUseResourcesFromFile,
        EStressTestCaseId_createDestroyMeshes_WhichUseResourcesFromFile_WhileRecreatingRenderer,

        EStressTestCaseId_createDestroyDynamicResources,
        EStressTestCaseId_updateDynamicResourcesLikeCrazy,
        EStressTestCaseId_switchBetweenClientAndSceneResourcesLikeCrazy,*/
    };

    class ResourceStressTests
    {
    public:
        static int32_t RunTest(EStressTestCaseId testToRun, const StressTestConfig& config);
        static int32_t RunAllTests(const StressTestConfig& config);

    private:

        explicit ResourceStressTests(const StressTestConfig& config);

        int32_t runTest(EStressTestCaseId testToRun);

        int32_t recreateResourcesEveryFrame(uint32_t sceneFpsLimit);
        int32_t recreateResourcesEveryFrame_MapSceneAfterAWhile(uint32_t sceneFpsLimit, uint32_t mapSceneDelayMSec);
        int32_t recreateResourcesEveryFrame_RemapSceneAllTheTime(uint32_t remapCycleDurationMSec);

        SceneArrayConfig generateStressSceneConfig() const;

        // convenience
        void setRendererFPS(uint32_t rendererFPS);
        void throttleSceneUpdatesAndConsumeRendererEvents(uint32_t sceneFpsLimit);

        const StressTestConfig& m_testConfig;
        ramses::RamsesFramework m_framework;
        ramses::RamsesClient&   m_client;
        StressTestRenderer      m_testRenderer;

        struct OffscreenBufferData
        {
            ramses::displayBufferId_t   bufferId;
            uint32_t                    width;
            uint32_t                    height;
            bool                        isInterruptable;
        };

        using OffscreenBuffers = std::vector<OffscreenBufferData>;

        struct DisplayData
        {
            ramses::displayId_t displayId;
            uint32_t            width = 0;
            uint32_t            height = 0;
            OffscreenBuffers    offscreenBuffers;
        };

        std::vector<DisplayData>             m_displays;
        const uint32_t                  m_sceneSetsPerDisplay;

        static const uint32_t           FirstDisplayWidth = 1200u;
        static const uint32_t           FirstDisplayHeight = 800u;
        mutable uint32_t m_consumeRendererEventsCounter = 1;
    };
}
