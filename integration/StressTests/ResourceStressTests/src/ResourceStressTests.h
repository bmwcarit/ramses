//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCESTRESSTESTS_RESOURCESTRESSTESTS_H
#define RAMSES_RESOURCESTRESSTESTS_RESOURCESTRESSTESTS_H

#include "ramses-client-api/RamsesClient.h"

#include "StressTestRenderer.h"
#include "ResourceStressTestScene.h"

#include "PlatformAbstraction/PlatformTypes.h"
#include "ResourceStressTestSceneArray.h"

namespace ramses_internal
{
    struct StressTestConfig
    {
        int32_t argc;
        const char** argv;
        uint32_t durationEachTestSeconds;
        uint32_t displayCount;
        uint32_t sceneSetsPerDisplay;
        bool disableSkippingOfFrames;
        UInt32 perFrameBudgetMSec_ClientRes;
        UInt32 perFrameBudgetMSec_SceneActions;
        UInt32 perFrameBudgetMSec_Rendering;
        UInt32 renderablesBatchSizeForRenderingInterruption;
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

        //keep this at the end
        EStressTestCaseId_NUMBER_OF_ELEMENTS
    };

    class ResourceStressTests
    {
    public:
        static Int32 RunTest(EStressTestCaseId testToRun, const StressTestConfig& config);
        static Int32 RunAllTests(const StressTestConfig& config);

    private:

        ResourceStressTests(const StressTestConfig& config);

        Int32 runTest(EStressTestCaseId testToRun);

        Int32 recreateResourcesEveryFrame(uint32_t sceneFpsLimit);
        Int32 recreateResourcesEveryFrame_MapSceneAfterAWhile(uint32_t sceneFpsLimit, uint32_t mapSceneDelayMSec);
        Int32 recreateResourcesEveryFrame_RemapSceneAllTheTime(uint32_t remapCycleDurationMSec);

        SceneArrayConfig generateStressSceneConfig() const;

        // convenience
        void setRendererFPS(uint32_t rendererFPS);
        void throttleSceneUpdatesAndConsumeRendererEvents(uint32_t sceneFpsLimit);

        const StressTestConfig  m_testConfig;
        ramses::RamsesFramework m_framework;
        ramses::RamsesClient    m_client;
        StressTestRenderer      m_testRenderer;

        struct OffscreenBufferData
        {
            ramses::offscreenBufferId_t bufferId;
            uint32_t                    width;
            uint32_t                    height;
            bool                        isInterruptable;
        };

        using OffscreenBuffers = std::vector<OffscreenBufferData>;

        struct DisplayData
        {
            ramses::displayId_t displayId;
            uint32_t            width;
            uint32_t            height;
            OffscreenBuffers    offscreenBuffers;
        };

        std::vector<DisplayData>             m_displays;
        const uint32_t                  m_sceneSetsPerDisplay;

        static const uint32_t           FirstDisplayWidth = 1200u;
        static const uint32_t           FirstDisplayHeight = 800u;
        mutable uint32_t m_consumeRendererEventsCounter = 1;
    };
}

#endif
