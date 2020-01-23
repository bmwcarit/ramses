//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// These includes are needed by the tests
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "TestScenesAndRenderer.h"
#include "RendererTestEventHandler.h"
#include "RendererTestsFramework.h"
#include "ReadPixelCallbackHandler.h"
#include "TestScenes/MultipleTrianglesScene.h"
#include "TestScenes/TextureBufferScene.h"
#include "TestScenes/DataBufferScene.h"
#include "TestScenes/TextScene.h"
#include "TestScenes/FileLoadingScene.h"
#include "TestScenes/Texture2DFormatScene.h"
#include "RendererTestUtils.h"

// These includes are needed because of ramses API usage
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-client-api/DataInt32.h"

// These includes should not be needed... Technical debts + usage of internal classes
#include "RamsesRendererImpl.h"
#include "DisplayConfigImpl.h"
#include "RendererAPI/IDevice.h"
#include "RendererAPI/IRenderBackend.h"
#include "RendererAPI/ISurface.h"

#include "gtest/gtest.h"
#include <thread>

namespace ramses_internal
{
    class ARendererLifecycleTest : public ::testing::Test
    {
    public:
        ARendererLifecycleTest()
            : frameworkConfig()
            , testScenesAndRenderer(frameworkConfig)
            , testRenderer(testScenesAndRenderer.getTestRenderer())
        {}

        ramses::displayId_t createDisplayForWindow(uint32_t iviSurfaceIdOffset = 0u, bool iviWindowStartVisible = true)
        {
            ramses::DisplayConfig displayConfig = RendererTestUtils::CreateTestDisplayConfig(iviSurfaceIdOffset, iviWindowStartVisible);
            displayConfig.setWindowRectangle(WindowX, WindowY, WindowWidth, WindowHeight);
            displayConfig.setPerspectiveProjection(19.f, static_cast<float>(WindowWidth) / WindowHeight, 0.1f, 1500.f);
            return testRenderer.createDisplay(displayConfig);
        }

        testing::AssertionResult checkScreenshot(ramses::displayId_t display, const char* screenshotFile)
        {
            if (testRenderer.performScreenshotCheck(display, 0u, 0u, WindowWidth, WindowHeight, screenshotFile))
                return testing::AssertionSuccess();


            return testing::AssertionFailure() << "Screenshot failed " << screenshotFile;
        }

        static const uint32_t WindowX = 0u;
        static const uint32_t WindowY = 0u;
        static const uint32_t WindowWidth = 128u;
        static const uint32_t WindowHeight = 64u;

        ramses::RamsesFrameworkConfig frameworkConfig;
        TestScenesAndRenderer testScenesAndRenderer;
        TestRenderer& testRenderer;
    };

    TEST_F(ARendererLifecycleTest, RenderScene)
    {
        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(display != ramses::displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RecreateSceneWithSameId)
    {
        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.getScenesRegistry().destroyScene(sceneId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, sceneId, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SaveLoadSceneFromFileThenRender)
    {
        const ramses::sceneId_t sceneId(1234u);

        testScenesAndRenderer.getScenesRegistry().createFileLoadingScene(sceneId, ramses_internal::Vector3(0.0f, 0.0f, 5.0f), frameworkConfig, ramses_internal::FileLoadingScene::CREATE_SAVE_DESTROY_LOAD_USING_SEPARATE_CLIENT);

        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);

        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_AfterLoadSave"));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SaveLoadSceneFromFileThenRender_Threaded)
    {
        const ramses::sceneId_t sceneId(1234u);

        testScenesAndRenderer.getScenesRegistry().createFileLoadingScene(sceneId, ramses_internal::Vector3(0.0f, 0.0f, 5.0f), frameworkConfig, ramses_internal::FileLoadingScene::CREATE_SAVE_DESTROY_LOAD_USING_SEPARATE_CLIENT);
        const auto validateResult = testScenesAndRenderer.validateScene(sceneId);
        ASSERT_EQ(ramses::StatusOK, validateResult) << testScenesAndRenderer.getValidationReport(sceneId);

        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        const ramses::displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);

        // Subscribe
        testRenderer.subscribeScene(sceneId);

        // Map
        testRenderer.mapScene(display, sceneId);

        testScenesAndRenderer.flush(sceneId, 1);
        testRenderer.waitForNamedFlush(sceneId, 1);

        // Show
        testRenderer.showScene(sceneId);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_AfterLoadSave"));

        testRenderer.hideScene(sceneId);
        testRenderer.unmapScene(sceneId);

        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, DestroyAndRecreateRenderer)
    {
        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES,
                                                                                                                                ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        ramses::displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();

        testScenesAndRenderer.initializeRenderer();
        display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, DestroyRenderer_ChangeScene_ThenRecreateRenderer)
    {
        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::TextScene>(ramses_internal::TextScene::EState_INITIAL);
        testScenesAndRenderer.initializeRenderer();
        ramses::displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();

        testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::TextScene>(sceneId, ramses_internal::TextScene::EState_INITIAL_128_BY_64_VIEWPORT);

        testScenesAndRenderer.initializeRenderer();
        display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_SimpleText"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, UnsubscribeRenderer_ChangeScene_ThenResubscribeRenderer)
    {
        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testRenderer.hideUnmapAndUnsubscribeScene(sceneId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Triangles_reordered"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, ChangeScene_UnsubscribeRenderer_Flush_ThenResubscribeRenderer)
    {
        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(display != ramses::displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
        testRenderer.hideUnmapAndUnsubscribeScene(sceneId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Triangles_reordered"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RAMSES2881_CreateRendererAfterScene)
    {
        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);

        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, DestroyDisplayAndRemapSceneToOtherDisplay)
    {
        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display1 = createDisplayForWindow(0u);
        const ramses::displayId_t display2 = createDisplayForWindow(1u);
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display1);
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display2);

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display1, sceneId);
        testRenderer.showScene(sceneId);

        testRenderer.hideScene(sceneId);
        testRenderer.unmapScene(sceneId);

        testRenderer.destroyDisplay(display1);

        testRenderer.mapScene(display2, sceneId);
        testRenderer.showScene(sceneId);

        ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RenderScene_Threaded)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);

        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);

        testScenesAndRenderer.flush(sceneId, 1u);
        testRenderer.waitForNamedFlush(sceneId, 1u);

        testRenderer.showScene(sceneId);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testRenderer.hideScene(sceneId);
        testRenderer.unmapScene(sceneId);

        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RenderChangingScene_Threaded)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));

        // create data to change
        ramses::DataInt32* data = testScenesAndRenderer.getScenesRegistry().getScene(sceneId).createDataInt32();
        testScenesAndRenderer.flush(sceneId);

        testScenesAndRenderer.publish(sceneId);

        //do not wait for subscription
        testRenderer.subscribeScene(sceneId, false);

        // change scene while subscription is ongoing
        for (int i = 0; i < 80; i++)
        {
            data->setValue(i);
            testScenesAndRenderer.flush(sceneId);
        }
        testRenderer.waitForSubscription(sceneId);

        testRenderer.mapScene(display, sceneId);

        testScenesAndRenderer.flush(sceneId, 1u);
        testRenderer.waitForNamedFlush(sceneId, 1u);

        testRenderer.showScene(sceneId);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testRenderer.hideScene(sceneId);
        testRenderer.unmapScene(sceneId);

        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RenderScene_StartStopThreadMultipleTimes)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);

        testRenderer.subscribeScene(sceneId, false);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.waitForSubscription(sceneId);

        testRenderer.mapScene(display, sceneId);

        testScenesAndRenderer.flush(sceneId, 1u);
        testRenderer.waitForNamedFlush(sceneId, 1u);

        testRenderer.showScene(sceneId);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testRenderer.stopRendererThread();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        testRenderer.startRendererThread();

        testRenderer.stopRendererThread();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        testRenderer.startRendererThread();

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testRenderer.hideScene(sceneId);
        testRenderer.unmapScene(sceneId);

        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, DestroyRendererWhileThreadRunning)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);

        testRenderer.subscribeScene(sceneId, false);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.waitForSubscription(sceneId);

        testRenderer.mapScene(display, sceneId);

        testScenesAndRenderer.flush(sceneId, 1u);
        testRenderer.waitForNamedFlush(sceneId, 1u);

        testRenderer.showScene(sceneId);

        testScenesAndRenderer.destroyRenderer();

    }

    TEST_F(ARendererLifecycleTest, RendererUploadsResourcesIfIviSurfaceInvisible)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        const ramses::displayId_t display = createDisplayForWindow(0u, false);
        ASSERT_TRUE(display != ramses::displayId_t::Invalid());

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::Texture2DFormatScene>(ramses_internal::Texture2DFormatScene::EState_R8, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId); // this would time out if resources for the scene could not be uploaded

        testRenderer.unmapScene(sceneId);
        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RendererUploadsResourcesIfIviSurfaceInvisibleInLoopModeUpdateOnly)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.setLoopMode(ramses::ELoopMode_UpdateOnly);
        testRenderer.startRendererThread();
        const ramses::displayId_t display = createDisplayForWindow(0u, false);
        ASSERT_TRUE(display != ramses::displayId_t::Invalid());

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::Texture2DFormatScene>(ramses_internal::Texture2DFormatScene::EState_R8, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId); // this would time out if resources for the scene could not be uploaded

        testRenderer.unmapScene(sceneId);
        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RemapScenesWithDynamicResourcesToOtherDisplay)
    {
        const ramses::sceneId_t sceneId1 = testScenesAndRenderer.getScenesRegistry().createScene<TextureBufferScene>(TextureBufferScene::EState_RGBA8_OneMip_ScaledDown, Vector3(-0.1f, -0.1f, 15.0f));
        const ramses::sceneId_t sceneId2 = testScenesAndRenderer.getScenesRegistry().createScene<DataBufferScene>(DataBufferScene::INDEX_DATA_BUFFER_UINT16, Vector3(-2.0f, -2.0f, 15.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display1 = createDisplayForWindow(0u);
        const ramses::displayId_t display2 = createDisplayForWindow(1u);
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display1);
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display2);

        testScenesAndRenderer.publish(sceneId1);
        testScenesAndRenderer.publish(sceneId2);
        testScenesAndRenderer.flush(sceneId1);
        testScenesAndRenderer.flush(sceneId2);
        testRenderer.subscribeScene(sceneId1);
        testRenderer.subscribeScene(sceneId2);
        testRenderer.mapScene(display1, sceneId1);
        testRenderer.mapScene(display1, sceneId2);
        testRenderer.showScene(sceneId1);
        testRenderer.showScene(sceneId2);

        testRenderer.hideScene(sceneId1);
        testRenderer.hideScene(sceneId2);
        testRenderer.unmapScene(sceneId1);
        testRenderer.unmapScene(sceneId2);

        ASSERT_TRUE(checkScreenshot(display1, "ARendererDisplays_Black"));

        testRenderer.mapScene(display2, sceneId1);
        testRenderer.mapScene(display2, sceneId2);
        testRenderer.showScene(sceneId1);
        testRenderer.showScene(sceneId2);

        ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_DynamicResources"));

        testScenesAndRenderer.unpublish(sceneId1);
        testScenesAndRenderer.unpublish(sceneId2);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneCanReachShownStateWithLoopModeUpdateOnly_UsingDoOneLoop)
    {
        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::Texture2DFormatScene>(ramses_internal::Texture2DFormatScene::EState_R8, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        testRenderer.setLoopMode(ramses::ELoopMode_UpdateOnly);

        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(display != ramses::displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneCanReachShownStateWithLoopModeUpdateOnly_UsingRenderThread)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        testRenderer.setLoopMode(ramses::ELoopMode_UpdateOnly);

        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::Texture2DFormatScene>(ramses_internal::Texture2DFormatScene::EState_R8, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);

        testRenderer.hideScene(sceneId);
        testRenderer.unmapScene(sceneId);

        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneCanReachShownStateWithLoopModeUpdateOnly_IfIviSurfaceInvisible)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        testRenderer.setLoopMode(ramses::ELoopMode_UpdateOnly);

        const ramses::displayId_t display = createDisplayForWindow(0u, false);
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::Texture2DFormatScene>(ramses_internal::Texture2DFormatScene::EState_R8, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);

        testRenderer.hideScene(sceneId);
        testRenderer.unmapScene(sceneId);

        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, DoesNotRenderToFramebufferInLoopModeUpdateOnly)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow(0u);
        ASSERT_TRUE(display != ramses::displayId_t::Invalid());

        testRenderer.setLoopMode(ramses::ELoopMode_UpdateOnly);
        testRenderer.readPixels(display, 0u, 0u, WindowWidth, WindowHeight);
        testRenderer.flushRenderer();
        testRenderer.doOneLoop();

        ReadPixelCallbackHandler callbackHandler;

        testRenderer.dispatchRendererEvents(callbackHandler);
        ASSERT_FALSE(callbackHandler.m_pixelDataRead);

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, Republish_ThenChangeScene)
    {
        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        ramses::displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);

        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Triangles_reordered"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, PollingFrameCallbacks_DoesNotBlockIfNoDisplaysExist)
    {
        const std::chrono::seconds largePollingTime{100u};
        RendererTestUtils::SetMaxFrameCallbackPollingTime(std::chrono::duration_cast<std::chrono::microseconds>(largePollingTime));

        testScenesAndRenderer.initializeRenderer();

        const auto startTime = std::chrono::steady_clock::now();

        testRenderer.flushRenderer();
        testRenderer.doOneLoop();
        testRenderer.flushRenderer();
        testRenderer.doOneLoop();
        testRenderer.flushRenderer();
        testRenderer.doOneLoop();
        testRenderer.flushRenderer();
        testRenderer.doOneLoop();

        const auto timeElapsed  = std::chrono::steady_clock::now() - startTime;
        const std::chrono::milliseconds maximumExpectedTime{ largePollingTime/ 2u };
        ASSERT_TRUE(timeElapsed < maximumExpectedTime);

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, PollingFrameCallbacks_BlocksIfDisplayNotReadyToRender)
    {
        const std::chrono::milliseconds nonTrivialPollingTime{50u};
        RendererTestUtils::SetMaxFrameCallbackPollingTime(std::chrono::duration_cast<std::chrono::microseconds>(nonTrivialPollingTime));

        testScenesAndRenderer.initializeRenderer();

        if(testRenderer.hasSystemCompositorController())
        {
            createDisplayForWindow(0u, false);

            const auto startTime = std::chrono::steady_clock::now();

            testRenderer.flushRenderer();
            testRenderer.doOneLoop();
            testRenderer.flushRenderer();
            testRenderer.doOneLoop();

            const auto timeElapsed  = std::chrono::steady_clock::now() - startTime;
            ASSERT_TRUE(timeElapsed >= nonTrivialPollingTime);
        }

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, PollingFrameCallbacks_BlocksIfAllDisplaysNotReadyToRender)
    {
        const std::chrono::milliseconds nonTrivialPollingTime{50u};
        RendererTestUtils::SetMaxFrameCallbackPollingTime(std::chrono::duration_cast<std::chrono::microseconds>(nonTrivialPollingTime));

        testScenesAndRenderer.initializeRenderer();

        if(testRenderer.hasSystemCompositorController())
        {
            createDisplayForWindow(0u, false);
            createDisplayForWindow(1u, false);

            const auto startTime = std::chrono::steady_clock::now();

            testRenderer.flushRenderer();
            testRenderer.doOneLoop();
            testRenderer.flushRenderer();
            testRenderer.doOneLoop();

            const auto timeElapsed  = std::chrono::steady_clock::now() - startTime;
            ASSERT_TRUE(timeElapsed >= nonTrivialPollingTime);
        }

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, PollingFrameCallbacks_UnreadyDisplayDoesNotBlockReadyDisplay)
    {
        const std::chrono::seconds largePollingTime{100u};
        RendererTestUtils::SetMaxFrameCallbackPollingTime(std::chrono::duration_cast<std::chrono::microseconds>(largePollingTime));

        testScenesAndRenderer.initializeRenderer();

        if(testRenderer.hasSystemCompositorController())
        {
            const ramses::displayId_t display1 = createDisplayForWindow(0u, true);
            createDisplayForWindow(1u, false);

            const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));

            testScenesAndRenderer.publish(sceneId);
            testScenesAndRenderer.flush(sceneId);
            testRenderer.subscribeScene(sceneId);
            testRenderer.mapScene(display1, sceneId);
            testRenderer.showScene(sceneId);

            const auto startTime = std::chrono::steady_clock::now();

            testRenderer.flushRenderer();
            testRenderer.doOneLoop();

            testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.flush(sceneId);
            ASSERT_TRUE(checkScreenshot(display1, "ARendererInstance_Triangles_reordered"));

            const auto timeElapsed  = std::chrono::steady_clock::now() - startTime;

            const auto maximumExpectedTime = largePollingTime / 2;
            ASSERT_TRUE(timeElapsed < maximumExpectedTime);
        }

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, PollingFrameCallbacks_UnreadyDisplayDoesNotBlockReadyDisplay_DisplaysInOtherOrder)
    {
        const std::chrono::seconds largePollingTime{100u};
        RendererTestUtils::SetMaxFrameCallbackPollingTime(std::chrono::duration_cast<std::chrono::microseconds>(largePollingTime));

        testScenesAndRenderer.initializeRenderer();

        if(testRenderer.hasSystemCompositorController())
        {
            createDisplayForWindow(0u, false);
            const ramses::displayId_t display2 = createDisplayForWindow(1u, true);

            const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));

            testScenesAndRenderer.publish(sceneId);
            testScenesAndRenderer.flush(sceneId);
            testRenderer.subscribeScene(sceneId);
            testRenderer.mapScene(display2, sceneId);
            testRenderer.showScene(sceneId);

            const auto startTime = std::chrono::steady_clock::now();

            testRenderer.flushRenderer();
            testRenderer.doOneLoop();

            testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.flush(sceneId);
            ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_Triangles_reordered"));

            const auto timeElapsed  = std::chrono::steady_clock::now() - startTime;
            const auto maximumExpectedTime = largePollingTime / 2;
            ASSERT_TRUE(timeElapsed < maximumExpectedTime);
        }

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, PollingFrameCallbacks_ReadyDisplayDoesNotStarveOtherDisplay)
    {
        const std::chrono::milliseconds nonTrivialPollingTime{50u};
        RendererTestUtils::SetMaxFrameCallbackPollingTime(std::chrono::duration_cast<std::chrono::microseconds>(nonTrivialPollingTime));

        testScenesAndRenderer.initializeRenderer();

        if(testRenderer.hasSystemCompositorController())
        {
            const ramses::displayId_t display1 = createDisplayForWindow(0u, true);
            createDisplayForWindow(1u, true); //nothing gets rendered on it, so it is ALWAYS ready (except right after clearing)

            ASSERT_TRUE(checkScreenshot(display1, "ARendererDisplays_Black"));

            const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));

            testScenesAndRenderer.publish(sceneId);
            testScenesAndRenderer.flush(sceneId);
            testRenderer.subscribeScene(sceneId);
            testRenderer.mapScene(display1, sceneId);
            testRenderer.showScene(sceneId);

            ASSERT_TRUE(checkScreenshot(display1, "ARendererInstance_Three_Triangles"));

            //render again and make sure the display was updated
            testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.flush(sceneId);
            //taking the screenshot would timeout if display1 is being starved by the (always ready) other display
            ASSERT_TRUE(checkScreenshot(display1, "ARendererInstance_Triangles_reordered"));
        }

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, PollingFrameCallbacks_ReadyDisplayDoesNotStarveOtherDisplay_DisplaysInOtherOrder)
    {
        const std::chrono::milliseconds nonTrivialPollingTime{50u};
        RendererTestUtils::SetMaxFrameCallbackPollingTime(std::chrono::duration_cast<std::chrono::microseconds>(nonTrivialPollingTime));

        testScenesAndRenderer.initializeRenderer();

        if(testRenderer.hasSystemCompositorController())
        {
            createDisplayForWindow(0u, true); //nothing gets rendered on it, so it is ALWAYS ready (except right after clearing)
            const ramses::displayId_t display2 = createDisplayForWindow(1u, true);

            ASSERT_TRUE(checkScreenshot(display2, "ARendererDisplays_Black"));

            const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(0.0f, 0.0f, 5.0f));

            testScenesAndRenderer.publish(sceneId);
            testScenesAndRenderer.flush(sceneId);
            testRenderer.subscribeScene(sceneId);
            testRenderer.mapScene(display2, sceneId);
            testRenderer.showScene(sceneId);

            ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_Three_Triangles"));

            //render again and make sure the display was updated
            testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.flush(sceneId);
            //taking the screenshot would timeout if display1 is being starved by the (always ready) other display
            ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_Triangles_reordered"));
        }

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneNotExpiredWhenUpdatedAndSubscribed)
    {
        testScenesAndRenderer.initializeRenderer();

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);

        for (int i = 0; i < 5; ++i)
        {
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::hours(1));
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }

        ASSERT_FALSE(testRenderer.consumeEventsAndCheckExpiredScenes({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneExpiredWhenSubscribed)
    {
        testScenesAndRenderer.initializeRenderer();

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.doOneLoop();

        // next flush expired already in past to trigger the exceeded event
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() - std::chrono::hours(1));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        ASSERT_TRUE(testRenderer.consumeEventsAndCheckExpiredScenes({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneExpiredAndRecoveredWhenSubscribed)
    {
        testScenesAndRenderer.initializeRenderer();

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);

        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::hours(1));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.doOneLoop();

        // next flush will be in past to trigger the exceeded event
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() - std::chrono::hours(1));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();
        ASSERT_TRUE(testRenderer.consumeEventsAndCheckExpiredScenes({ sceneId }));

        // next flush will be in future again to trigger the recovery event
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::hours(1));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        ASSERT_TRUE(testRenderer.consumeEventsAndCheckRecoveredScenes({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneNotExpiredWhenUpdatedAndRendered)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);
        testRenderer.doOneLoop();

        // send flushes and render within limit
        for (int i = 0; i < 5; ++i)
        {
            // make modifications to scene
            testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::hours(1));
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }

        ASSERT_FALSE(testRenderer.consumeEventsAndCheckExpiredScenes({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneNotExpiredWhenUpdatedWithEmptyFlushesAndRendered)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);
        testRenderer.doOneLoop();

        // send flushes and render within limit
        for (int i = 0; i < 5; ++i)
        {
            // no modifications to scene
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::hours(1));
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }

        ASSERT_FALSE(testRenderer.consumeEventsAndCheckExpiredScenes({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneExpiredWhenRendered)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);
        testRenderer.doOneLoop();

        // set expiration of content that will be rendered and eventually will expire
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(300));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        // send flushes within limit but do not render
        testRenderer.setLoopMode(ramses::ELoopMode_UpdateOnly);
        for (int i = 0; i < 5; ++i)
        {
            // make modifications to scene
            testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(500)); // these will not expire
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        ASSERT_TRUE(testRenderer.consumeEventsAndCheckExpiredScenes({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneExpiredWhenRenderedAndRecoveredAfterHidden)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);
        testRenderer.doOneLoop();

        // set expiration of content that will be rendered and eventually will expire
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(300));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        // send flushes within limit but do not render
        testRenderer.setLoopMode(ramses::ELoopMode_UpdateOnly);
        for (int i = 0; i < 5; ++i)
        {
            // make modifications to scene
            testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(500)); // these will not expire
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        // rendered content expired
        ASSERT_TRUE(testRenderer.consumeEventsAndCheckExpiredScenes({ sceneId }));

        // make sure the scene is still expired till after hidden
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() - std::chrono::hours(1));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        // now hide scene so regular flushes are enough to recover
        testRenderer.hideScene(sceneId);
        for (int i = 0; i < 5; ++i)
        {
            // make modifications to scene
            testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }
        ASSERT_TRUE(testRenderer.consumeEventsAndCheckRecoveredScenes({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneExpirationCanBeDisabled_ConfidenceTest)
    {
        struct ExpirationCounter final : public ramses::RendererEventHandlerEmpty
        {
            virtual void sceneExpired(ramses::sceneId_t) override final
            {
                numExpirationEvents++;
            }
            size_t numExpirationEvents = 0u;
        } expirationCounter;

        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);
        testRenderer.doOneLoop();

        // set expiration of content that will be rendered
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(2000));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        // send flushes within limit and render
        for (int i = 0; i < 5; ++i)
        {
            // make modifications to scene
            testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(2000)); // these will not expire
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }
        testRenderer.dispatchRendererEvents(expirationCounter);
        ASSERT_TRUE(expirationCounter.numExpirationEvents == 0u);

        // now hide scene
        testRenderer.hideScene(sceneId);

        // send few more flushes within limit and no changes
        for (int i = 0; i < 3; ++i)
        {
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(1000)); // these will not expire
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }
        testRenderer.dispatchRendererEvents(expirationCounter);
        ASSERT_TRUE(expirationCounter.numExpirationEvents == 0u);

        // disable expiration together with scene changes
        testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::InvalidTimestamp);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        // stop sending flushes altogether but keep looping,
        // loop long enough to prove that expiration checking was really disabled,
        // i.e. render past the last non-zero expiration TS set
        for (int i = 0; i < 5; ++i)
        {
            testRenderer.doOneLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        testRenderer.dispatchRendererEvents(expirationCounter);
        ASSERT_TRUE(expirationCounter.numExpirationEvents == 0u);

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneExpiredAndRecoveredWhenRendered)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.subscribeScene(sceneId);
        testRenderer.mapScene(display, sceneId);
        testRenderer.showScene(sceneId);
        testRenderer.doOneLoop();

        // set expiration of content that will be rendered and eventually will expire
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(300));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        // send flushes within limit but do not render
        testRenderer.setLoopMode(ramses::ELoopMode_UpdateOnly);
        for (int i = 0; i < 5; ++i)
        {
            // make modifications to scene
            testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        ASSERT_TRUE(testRenderer.consumeEventsAndCheckExpiredScenes({ sceneId }));

        // now also render within limit to recover
        testRenderer.setLoopMode(ramses::ELoopMode_UpdateAndRender);
        for (int i = 0; i < 5; ++i)
        {
            // make modifications to scene
            testScenesAndRenderer.getScenesRegistry().setSceneState<ramses_internal::MultipleTrianglesScene>(sceneId, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }
        ASSERT_TRUE(testRenderer.consumeEventsAndCheckRecoveredScenes({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, ScenesExpireOneAfterAnother)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId1 = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(-0.50f, 1.0f, 5.0f));
        const ramses::sceneId_t sceneId2 = testScenesAndRenderer.getScenesRegistry().createScene<ramses_internal::MultipleTrianglesScene>(ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses_internal::Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId1);
        testScenesAndRenderer.publish(sceneId2);
        testScenesAndRenderer.flush(sceneId1);
        testScenesAndRenderer.flush(sceneId2);
        testRenderer.subscribeScene(sceneId1);
        testRenderer.subscribeScene(sceneId2);
        testRenderer.mapScene(display, sceneId1);
        testRenderer.mapScene(display, sceneId2);
        testRenderer.showScene(sceneId1);
        testRenderer.showScene(sceneId2);
        testRenderer.doOneLoop();

        testScenesAndRenderer.setExpirationTimestamp(sceneId1, FlushTime::Clock::now() + std::chrono::milliseconds(500));
        testScenesAndRenderer.setExpirationTimestamp(sceneId2, FlushTime::Clock::now() + std::chrono::milliseconds(500));
        testScenesAndRenderer.flush(sceneId1);
        testScenesAndRenderer.flush(sceneId2);

        // S1 exceeds, S2 is ok
        for (int i = 0; i < 5; ++i)
        {
            testScenesAndRenderer.setExpirationTimestamp(sceneId2, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.flush(sceneId2);
            testRenderer.doOneLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        ASSERT_TRUE(testRenderer.consumeEventsAndCheckExpiredScenes({ sceneId1 }));

        // S1 recovers, S2 is ok
        for (int i = 0; i < 5; ++i)
        {
            testScenesAndRenderer.setExpirationTimestamp(sceneId1, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.setExpirationTimestamp(sceneId2, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.flush(sceneId1);
            testScenesAndRenderer.flush(sceneId2);
            testRenderer.doOneLoop();
        }
        ASSERT_TRUE(testRenderer.consumeEventsAndCheckRecoveredScenes({ sceneId1 }));

        // S1 ok, S2 exceeds
        for (int i = 0; i < 5; ++i)
        {
            testScenesAndRenderer.setExpirationTimestamp(sceneId1, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.flush(sceneId1);
            testRenderer.doOneLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        ASSERT_TRUE(testRenderer.consumeEventsAndCheckExpiredScenes({ sceneId2 }));

        // S1 ok, S2 is recovers
        for (int i = 0; i < 5; ++i)
        {
            testScenesAndRenderer.setExpirationTimestamp(sceneId1, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.setExpirationTimestamp(sceneId2, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.flush(sceneId1);
            testScenesAndRenderer.flush(sceneId2);
            testRenderer.doOneLoop();
        }
        ASSERT_TRUE(testRenderer.consumeEventsAndCheckRecoveredScenes({ sceneId2 }));

        // both S1 and S2 exceed
        for (int i = 0; i < 5; ++i)
        {
            testRenderer.doOneLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        ASSERT_TRUE(testRenderer.consumeEventsAndCheckExpiredScenes({ sceneId1, sceneId2 }));

        testScenesAndRenderer.destroyRenderer();
    }
}
