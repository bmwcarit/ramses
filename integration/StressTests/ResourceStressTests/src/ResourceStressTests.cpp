//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceStressTests.h"

#include "ramses-renderer-api/RendererConfig.h"

#include "Utils/Argument.h"
#include "ResourceStressTestSceneArray.h"
#include "RenderExecutor.h"

namespace ramses_internal
{
    const char* StressTestCaseNames[] = {
        "EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush",
        "EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_LowRendererFPS",
        "EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_ExtremelyLowRendererFPS",
        "EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_MapSceneAfterAWhile",
        "EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_MapSceneAfterAWhile_LowRendererFPS",
        "EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_MapSceneAfterAWhile_ExtremelyLowRendererFPS",
        "EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_RemapSceneAllTheTime",
    };
    ENUM_TO_STRING(EStressTestCaseId, StressTestCaseNames, EStressTestCaseId_NUMBER_OF_ELEMENTS);

    ResourceStressTests::ResourceStressTests(const StressTestConfig& config)
        : m_testConfig(config)
        , m_framework(config.argc, config.argv)
        , m_client("resource-stress-tests", m_framework)
        , m_testRenderer(m_framework, ramses::RendererConfig(config.argc, config.argv))
        , m_displays(config.displayCount)
        , m_sceneSetsPerDisplay(config.sceneSetsPerDisplay)
    {

        m_framework.connect();
        m_testRenderer.startLooping();

        //ramses_internal::RenderExecutor::NumRenderablesToRenderInBetweenTimeBudgetChecks = config.renderablesBatchSizeForRenderingInterruption;

        m_testRenderer.setSkippingOfUnmodifiedBuffers(!config.disableSkippingOfFrames);

        m_testRenderer.setFrameTimerLimits(config.perFrameBudgetMSec_ClientRes, config.perFrameBudgetMSec_SceneActions, config.perFrameBudgetMSec_Rendering);

        uint32_t displayOffset = 0;
        for (UInt i = 0; i < config.displayCount; ++i)
        {
            const auto displayWidth = FirstDisplayWidth >> i;
            const auto displayHeight = FirstDisplayHeight >> i;
            const auto displayId = m_testRenderer.createDisplay(displayOffset, displayWidth, displayHeight, uint32_t(i), config.argc, config.argv);

            // Offscreen buffers can be smaller, to make the tests run faster
            const auto obWidth = displayWidth / 10;
            const auto obHeight = displayHeight / 10;

            const OffscreenBuffers offscreenBuffers =
            {
                { m_testRenderer.createOffscreenBuffer(displayId, obWidth, obHeight, true ), obWidth, obHeight, true  },
                { m_testRenderer.createOffscreenBuffer(displayId, obWidth, obHeight, true ), obWidth, obHeight, true  },
                { m_testRenderer.createOffscreenBuffer(displayId, obWidth, obHeight, false), obWidth, obHeight, false },
                { m_testRenderer.createOffscreenBuffer(displayId, obWidth, obHeight, false), obWidth, obHeight, false }
            };

            m_displays[i] = DisplayData{ displayId, displayWidth , displayHeight, offscreenBuffers };
            displayOffset += FirstDisplayWidth >> i;
        }
    }

    Int32 ResourceStressTests::runTest(EStressTestCaseId testToRun)
    {
        static const UInt32 MinDurationPerTestSeconds[] =
        {
            1, //"EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush",
            1, //"EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_LowRendererFPS",
            1, //"EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_ExtremelyLowRendererFPS",
            1, //"EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_MapSceneAfterAWhile",
            3, //"EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_MapSceneAfterAWhile_LowRendererFPS",
            5, //"EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_MapSceneAfterAWhile_ExtremelyLowRendererFPS",
            15,//"EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_RemapSceneAllTheTime",
        };
        static_assert(static_cast<std::size_t>(EStressTestCaseId_NUMBER_OF_ELEMENTS) == sizeof(MinDurationPerTestSeconds)/sizeof(MinDurationPerTestSeconds[0]), "Size mismatch");

        if (m_testConfig.durationEachTestSeconds < MinDurationPerTestSeconds[testToRun])
        {
            LOG_ERROR(CONTEXT_SMOKETEST, "Test " << EnumToString(testToRun) << " has too short execution time! Aborting test and marking as failed...");
            return -1;
        }

        // How many times more scenes to create for stress tests which run multiple scene instances
        const uint32_t Scene10FPS = 10;
        const uint32_t Scene20FPS = 20;
        const uint32_t Scene60FPS = 60;
        Int32 returnValue = -1;
        switch (testToRun)
        {
        case EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush:
            returnValue = recreateResourcesEveryFrame(Scene60FPS);
            break;
        case EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_LowRendererFPS:
            setRendererFPS(20);
            returnValue = recreateResourcesEveryFrame(Scene10FPS);
            break;
        case EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_ExtremelyLowRendererFPS:
            setRendererFPS(10);
            returnValue = recreateResourcesEveryFrame(Scene20FPS);
            break;
        case EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_MapSceneAfterAWhile:
            returnValue = recreateResourcesEveryFrame_MapSceneAfterAWhile(Scene60FPS, 500 * m_testConfig.durationEachTestSeconds);
            break;
        case EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_MapSceneAfterAWhile_LowRendererFPS:
            setRendererFPS(20);
            returnValue = recreateResourcesEveryFrame_MapSceneAfterAWhile(Scene10FPS, 500 * m_testConfig.durationEachTestSeconds);
            break;
        case EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_MapSceneAfterAWhile_ExtremelyLowRendererFPS:
            setRendererFPS(10);
            returnValue = recreateResourcesEveryFrame_MapSceneAfterAWhile(Scene20FPS, 500 * m_testConfig.durationEachTestSeconds);
            break;
        case EStressTestCaseId_recreateResourcesEveryFrameWithSyncFlush_RemapSceneAllTheTime:
            returnValue = recreateResourcesEveryFrame_RemapSceneAllTheTime(2000);
            break;
        default:
            assert(false);
            break;
        }
        return returnValue;
    }

    SceneArrayConfig ResourceStressTests::generateStressSceneConfig() const
    {
        // Top level scene == scene which is rendered directly to framebuffer
        const uint32_t displayCount = static_cast<uint32_t>(m_displays.size());
        const uint32_t topLevelSceneCount = m_sceneSetsPerDisplay * displayCount;

        ramses::dataConsumerId_t textureConsumerId = 0;
        ramses::sceneId_t sceneId(0);

        SceneArrayConfig sceneArrayConfig;
        for (uint32_t topLevelScene = 0; topLevelScene < topLevelSceneCount; ++topLevelScene)
        {
            const auto& display = m_displays[topLevelScene % m_displays.size()];
            const uint32_t topLevelSceneIndexWithinDisplay = topLevelScene / displayCount;
            const ramses::sceneId_t topLevelSceneId(sceneId++);

            const TextureConsumerDataIds topLevelSceneConsumerIds = {
                textureConsumerId++,
                textureConsumerId++,
                textureConsumerId++,
                textureConsumerId++ };

            // Construct a quad to position the top-level scene on a fraction of the display (horizontally)
            // For one scene per display, that's the whole display, for two scenes: half of it, and so on
            const ScreenspaceQuad topLevelSceneQuad =
                ScreenspaceQuad(display.width, display.height,
                    { static_cast<float>(topLevelSceneIndexWithinDisplay) / m_sceneSetsPerDisplay, // Left offset == (id)/Nth of display size, where id=0, 1, 2... N
                    0.0f,
                    (topLevelSceneIndexWithinDisplay + 1.0f) / m_sceneSetsPerDisplay,              // Right offset = (id + 1) Nth of display size, where id=0, 1, 2... N
                    1.0f
                    });

            sceneArrayConfig.push_back({
                topLevelSceneId,
                display.displayId,
                ramses::InvalidOffscreenBufferId, // show on framebuffer
                topLevelSceneQuad,
                topLevelSceneConsumerIds,
                ramses::sceneId_t(0xffffffff),          // invalid consumer scene, because this is the consumer scene
                ramses::dataConsumerId_t(0xffffffff)    // invalid consumer data, because this is the consumer scene
                });

            // Offscreenbuffer scenes -> rendered to offscreen buffer and linked to top level scene as texture
            for (uint32_t obIndex = 0; obIndex <  display.offscreenBuffers.size(); ++obIndex)
            {
                const auto& offscreenBuffer = display.offscreenBuffers[obIndex];
                const uint32_t scenesPerBuffer = (offscreenBuffer.isInterruptable) ? 2 : 1;

                for (uint32_t i = 0; i < scenesPerBuffer; ++i)
                {
                    // Same positioning as top-level scenes - divide the buffer into fractions
                    const float bufferFractionStart = static_cast<float>(i) / scenesPerBuffer;
                    const float bufferFractionEnd = bufferFractionStart + 1.0f / scenesPerBuffer;
                    const ScreenspaceQuad obSceneQuad = ScreenspaceQuad(offscreenBuffer.width, offscreenBuffer.height, { bufferFractionStart, 0.0f, bufferFractionEnd, 1.0f });
                    const ramses::sceneId_t obSceneId(sceneId++);
                    sceneArrayConfig.push_back({
                        obSceneId,
                        display.displayId,
                        offscreenBuffer.bufferId,
                        obSceneQuad,
                        TextureConsumerDataIds{},   // no consumer data of itself, because this is a consumed scene
                        topLevelSceneId,
                        topLevelSceneConsumerIds[obIndex]
                        });
                }
            }
        }

        return sceneArrayConfig;
    }

    void ResourceStressTests::setRendererFPS(uint32_t rendererFPS)
    {
        m_testRenderer.setFPS(rendererFPS);
    }

    Int32 ResourceStressTests::recreateResourcesEveryFrame(uint32_t sceneFpsLimit)
    {
        ResourceStressTestSceneArray scenes(m_client, m_testRenderer, generateStressSceneConfig());
        scenes.subscribeAll();
        scenes.mapAndShowAll();

        const UInt64 startTimeMs = PlatformTime::GetMillisecondsMonotonic();
        ramses::sceneVersionTag_t flushName = 0;

        while ((PlatformTime::GetMillisecondsMonotonic() - startTimeMs) / 1000 < m_testConfig.durationEachTestSeconds)
        {
            scenes.doExpensiveFlushOnAll(flushName++);

            throttleSceneUpdatesAndConsumeRendererEvents(sceneFpsLimit);
        }

        const ramses::sceneVersionTag_t nameOfLastFlush = flushName - 1;
        scenes.waitForFlushOnAll(nameOfLastFlush);
        return 0;
    }

    Int32 ResourceStressTests::recreateResourcesEveryFrame_MapSceneAfterAWhile(uint32_t sceneFpsLimit, uint32_t mapSceneDelayMSec)
    {
        ResourceStressTestSceneArray scenes(m_client, m_testRenderer, generateStressSceneConfig());
        scenes.subscribeAll();

        const UInt64 startTimeMs = PlatformTime::GetMillisecondsMonotonic();
        ramses::sceneVersionTag_t flushName = 0;
        bool scenesMapped = false;

        while ((PlatformTime::GetMillisecondsMonotonic() - startTimeMs) / 1000 < m_testConfig.durationEachTestSeconds)
        {
            // if half the time for the test passed -> map scene
            if ((PlatformTime::GetMillisecondsMonotonic() - startTimeMs) >= mapSceneDelayMSec)
            {
                if (!scenesMapped)
                {
                    scenes.mapAndShowAll();
                    scenesMapped = true;
                }
            }

            scenes.doExpensiveFlushOnAll(flushName++);

            throttleSceneUpdatesAndConsumeRendererEvents(sceneFpsLimit);
        }

        const ramses::sceneVersionTag_t nameOfLastFlush = flushName - 1;
        scenes.waitForFlushOnAll(nameOfLastFlush);

        return 0;
    }

    Int32 ResourceStressTests::recreateResourcesEveryFrame_RemapSceneAllTheTime(uint32_t remapCycleDurationMSec)
    {
        ResourceStressTestSceneArray scenes(m_client, m_testRenderer, generateStressSceneConfig());
        scenes.subscribeAll();

        const UInt64 startTimeMs = PlatformTime::GetMillisecondsMonotonic();
        ramses::sceneVersionTag_t flushName = 0;
        bool scenesMapped = false;

        while (
            ((PlatformTime::GetMillisecondsMonotonic() - startTimeMs) / 1000 < m_testConfig.durationEachTestSeconds)
            // This condition is needed to make sure the loop always ends with all scenes in "mapped" state, otherwise it's undefined if the resources will become ready
            || !scenesMapped)
        {
            const bool isSecondHalfOfRemapCycle = ((PlatformTime::GetMillisecondsMonotonic() - startTimeMs) % remapCycleDurationMSec >= remapCycleDurationMSec/2);

            // if half the time of the remap cycle passed -> map scene
            if (isSecondHalfOfRemapCycle && !scenesMapped)
            {
                scenes.mapAndShowAll();
                scenesMapped = true;
            }
            // if second half of the cycle passed -> unmap the scene again
            else if (!isSecondHalfOfRemapCycle && scenesMapped)
            {
                scenes.hideAndUnmapAll();
                scenesMapped = false;
            }

            scenes.doExpensiveFlushOnAll(flushName++);
        }

        const ramses::sceneVersionTag_t nameOfLastFlush = flushName - 1;
        scenes.waitForFlushOnAll(nameOfLastFlush);

        return 0;
    }

    void ResourceStressTests::throttleSceneUpdatesAndConsumeRendererEvents(uint32_t sceneFpsLimit)
    {
        if (0 != sceneFpsLimit)
        {
            PlatformThread::Sleep(1000u / sceneFpsLimit);
        }

        // Consume named flush events every now and then to prevent event overflow

        if (m_consumeRendererEventsCounter % 500 == 0)
        {
            m_testRenderer.consumePendingEvents();
            m_consumeRendererEventsCounter = 0;
        }
        ++m_consumeRendererEventsCounter;
    }

    Int32 ResourceStressTests::RunTest(EStressTestCaseId testToRun, const StressTestConfig& config)
    {
        ResourceStressTests resourceStressTests(config);
        const Int32 testResult = resourceStressTests.runTest(testToRun);

        if (testResult != 0)
        {
            LOG_ERROR(CONTEXT_SMOKETEST, "Test " << EnumToString(testToRun) << " finished with errors.");
        }
        else
        {
            LOG_INFO(CONTEXT_SMOKETEST, "Test " << EnumToString(testToRun) << " finished successfully.");
        }
        return testResult;
    }

    Int32 ResourceStressTests::RunAllTests(const StressTestConfig& config)
    {
        StringOutputStream verboseTestResults;
        Int32 testResult = 0;
        for (Int32 currentTest = 0; currentTest < static_cast<Int32>(EStressTestCaseId_NUMBER_OF_ELEMENTS); ++currentTest)
        {
            if (RunTest(static_cast<EStressTestCaseId>(currentTest), config) != 0)
            {
                testResult -= (1 << currentTest);
                verboseTestResults << "Test " << currentTest << " [" << EnumToString(static_cast<EStressTestCaseId>(currentTest)) << "] failed!\n";
            }
            else
            {
                verboseTestResults << "Test " << currentTest << " [" << EnumToString(static_cast<EStressTestCaseId>(currentTest)) << "] passed!\n";
            }
        }

        printf("Test results:\n%s", verboseTestResults.c_str());

        return testResult;
    }
}
