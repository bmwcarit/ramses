//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLifecycleTests.h"

// These includes are needed by the tests
#include "ramses-framework-api/RamsesFrameworkConfig.h"
#include "ReadPixelCallbackHandler.h"
#include "TestScenes/MultipleTrianglesScene.h"
#include "TestScenes/TextureBufferScene.h"
#include "TestScenes/DataBufferScene.h"
#include "TestScenes/TextScene.h"
#include "TestScenes/FileLoadingScene.h"
#include "TestScenes/Texture2DFormatScene.h"
#include "TestScenes/TextureSamplerScene.h"
#include "TestScenes/TransformationLinkScene.h"
#include "TestScenes/VisibilityScene.h"

// These includes are needed because of ramses API usage
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-client-api/DataInt32.h"
#include "ramses-client-api/SceneReference.h"
#include "RamsesObjectTypeUtils.h"

#include <thread>
#include <unordered_map>

namespace ramses_internal
{
    TEST_F(ARendererLifecycleTest, RenderScene)
    {
        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(display != ramses::displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RecreateSceneWithSameId)
    {
        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.getScenesRegistry().destroyScene(sceneId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, sceneId, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SaveLoadSceneFromFileThenRender)
    {
        const ramses::sceneId_t sceneId(1234u);

        testScenesAndRenderer.getScenesRegistry().createFileLoadingScene(sceneId, Vector3(0.0f, 0.0f, 5.0f), frameworkConfig, FileLoadingScene::CREATE_SAVE_DESTROY_LOAD_USING_SEPARATE_CLIENT, WindowWidth, WindowHeight);
        const auto validateResult = testScenesAndRenderer.validateScene(sceneId);
        ASSERT_EQ(ramses::StatusOK, validateResult) << testScenesAndRenderer.getValidationReport(sceneId);

        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_AfterLoadSave"));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SaveLoadSceneFromFileThenRender_Threaded)
    {
        const ramses::sceneId_t sceneId(1234u);

        testScenesAndRenderer.getScenesRegistry().createFileLoadingScene(sceneId, Vector3(0.0f, 0.0f, 5.0f), frameworkConfig, FileLoadingScene::CREATE_SAVE_DESTROY_LOAD_USING_SEPARATE_CLIENT, WindowWidth, WindowHeight);
        const auto validateResult = testScenesAndRenderer.validateScene(sceneId);
        ASSERT_EQ(ramses::StatusOK, validateResult) << testScenesAndRenderer.getValidationReport(sceneId);

        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        const ramses::displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
        testScenesAndRenderer.flush(sceneId, 1);
        testRenderer.waitForFlush(sceneId, 1);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_AfterLoadSave"));

        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, createDestroyDisplayMultipleTimesThreaded)
    {
        const ramses::sceneId_t sceneId(1234u);

        testScenesAndRenderer.getScenesRegistry().createFileLoadingScene(sceneId, Vector3(0.0f, 0.0f, 5.0f), frameworkConfig, FileLoadingScene::CREATE_SAVE_DESTROY_LOAD_USING_SEPARATE_CLIENT, WindowWidth, WindowHeight);
        const auto validateResult = testScenesAndRenderer.validateScene(sceneId);
        ASSERT_EQ(ramses::StatusOK, validateResult) << testScenesAndRenderer.getValidationReport(sceneId);

        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();

        for (uint32_t i = 0; i < 10; ++i)
        {
            const ramses::displayId_t display = createDisplayForWindow(i);

            testScenesAndRenderer.publish(sceneId);
            testRenderer.setSceneMapping(sceneId, display);
            ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
            testScenesAndRenderer.flush(sceneId, 1);
            testRenderer.waitForFlush(sceneId, 1);

            ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_AfterLoadSave"));

            testScenesAndRenderer.unpublish(sceneId);
            ASSERT_TRUE(testScenesAndRenderer.getTestRenderer().waitForSceneStateChange(sceneId, ramses::RendererSceneState::Unavailable));
            testRenderer.destroyDisplay(display);
        }

        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, DestroyAndRecreateRenderer)
    {
        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        ramses::displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();

        testScenesAndRenderer.initializeRenderer();
        display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, DestroyRenderer_ChangeScene_ThenRecreateRenderer)
    {
        const ramses::sceneId_t sceneId = createScene<TextScene>(TextScene::EState_INITIAL);
        testScenesAndRenderer.initializeRenderer();
        ramses::displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();

        testScenesAndRenderer.getScenesRegistry().setSceneState<TextScene>(sceneId, TextScene::EState_INITIAL_128_BY_64_VIEWPORT);

        testScenesAndRenderer.initializeRenderer();
        display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_SimpleText"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, UnsubscribeRenderer_ChangeScene_ThenResubscribeRenderer)
    {
        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Available));
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Triangles_reordered"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, ChangeScene_UnsubscribeRenderer_Flush_ThenResubscribeRenderer)
    {
        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(display != ramses::displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Available));
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Triangles_reordered"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RAMSES2881_CreateRendererAfterScene)
    {
        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);

        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, DestroyDisplayAndRemapSceneToOtherDisplay)
    {
        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display1 = createDisplayForWindow(0u);
        const ramses::displayId_t display2 = createDisplayForWindow(1u);
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display1);
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display2);

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display1);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Available));

        testRenderer.destroyDisplay(display1);

        testRenderer.setSceneMapping(sceneId, display2);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

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

        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);

        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        testScenesAndRenderer.flush(sceneId, 1u);
        testRenderer.waitForFlush(sceneId, 1u);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

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

        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));

        // create data to change
        ramses::DataInt32* data = testScenesAndRenderer.getScenesRegistry().getScene(sceneId).createDataInt32();
        testScenesAndRenderer.flush(sceneId);

        testScenesAndRenderer.publish(sceneId);

        //do not wait for subscription
        testRenderer.setSceneState(sceneId, ramses::RendererSceneState::Available);

        // change scene while subscription is ongoing
        for (int i = 0; i < 80; i++)
        {
            data->setValue(i);
            testScenesAndRenderer.flush(sceneId);
        }
        ASSERT_TRUE(testRenderer.waitForSceneStateChange(sceneId, ramses::RendererSceneState::Available));

        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        testScenesAndRenderer.flush(sceneId, 1u);
        testRenderer.waitForFlush(sceneId, 1u);

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

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

        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testRenderer.stopRendererThread();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        testRenderer.startRendererThread();

        testRenderer.stopRendererThread();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        testRenderer.startRendererThread();

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

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

        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RendererUploadsResourcesIfIviSurfaceInvisible)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        const ramses::displayId_t display = createDisplayForWindow(0u, false);
        ASSERT_TRUE(display != ramses::displayId_t::Invalid());

        const ramses::sceneId_t sceneId = createScene<Texture2DFormatScene>(Texture2DFormatScene::EState_R8, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
        // this would time out if resources for the scene could not be uploaded

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

        const ramses::sceneId_t sceneId = createScene<Texture2DFormatScene>(Texture2DFormatScene::EState_R8, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
        // this would time out if resources for the scene could not be uploaded

        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RemapScenesWithDynamicResourcesToOtherDisplay)
    {
        const ramses::sceneId_t sceneId1 = createScene<TextureBufferScene>(TextureBufferScene::EState_RGBA8_OneMip_ScaledDown, Vector3(-0.1f, -0.1f, 15.0f), 200u, 200u); // stretch a bit by using bigger viewport
        const ramses::sceneId_t sceneId2 = createScene<DataBufferScene>(DataBufferScene::INDEX_DATA_BUFFER_UINT16, Vector3(-2.0f, -2.0f, 15.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display1 = createDisplayForWindow(0u);
        const ramses::displayId_t display2 = createDisplayForWindow(1u);
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display1);
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display2);

        testScenesAndRenderer.publish(sceneId1);
        testScenesAndRenderer.publish(sceneId2);
        testScenesAndRenderer.flush(sceneId1);
        testScenesAndRenderer.flush(sceneId2);
        testRenderer.setSceneMapping(sceneId1, display1);
        testRenderer.setSceneMapping(sceneId2, display1);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId1, ramses::RendererSceneState::Rendered));
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId2, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(testRenderer.getSceneToState(sceneId1, ramses::RendererSceneState::Available));
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId2, ramses::RendererSceneState::Available));

        ASSERT_TRUE(checkScreenshot(display1, "ARendererDisplays_Black"));

        testRenderer.setSceneMapping(sceneId1, display2);
        testRenderer.setSceneMapping(sceneId2, display2);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId1, ramses::RendererSceneState::Rendered));
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId2, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_DynamicResources"));

        testScenesAndRenderer.unpublish(sceneId1);
        testScenesAndRenderer.unpublish(sceneId2);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneCanReachShownStateWithLoopModeUpdateOnly_UsingDoOneLoop)
    {
        const ramses::sceneId_t sceneId = createScene<Texture2DFormatScene>(Texture2DFormatScene::EState_R8, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        testRenderer.setLoopMode(ramses::ELoopMode_UpdateOnly);

        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(display != ramses::displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

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

        const ramses::sceneId_t sceneId = createScene<Texture2DFormatScene>(Texture2DFormatScene::EState_R8, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

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

        const ramses::sceneId_t sceneId = createScene<Texture2DFormatScene>(Texture2DFormatScene::EState_R8, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

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
        ramses::RendererSceneControlEventHandlerEmpty dummy;
        testRenderer.dispatchEvents(callbackHandler, dummy);
        ASSERT_FALSE(callbackHandler.m_pixelDataRead);

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, Republish_ThenChangeScene)
    {
        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        ramses::displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        ASSERT_TRUE(testScenesAndRenderer.getTestRenderer().waitForSceneStateChange(sceneId, ramses::RendererSceneState::Unavailable));

        testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);

        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Triangles_reordered"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    class ReferencedSceneStateEventHandler final : public TestClientEventHandler
    {
    public:
        using ReferenceStateMap = std::unordered_map<ramses::sceneId_t, ramses::RendererSceneState>;

        explicit ReferencedSceneStateEventHandler(ReferenceStateMap&& targetStateMap) : m_targetStateMap{ std::move(targetStateMap) } {}

        virtual void sceneReferenceStateChanged(ramses::SceneReference& sceneRef, ramses::RendererSceneState state) override
        {
            m_currentStateMap[sceneRef.getReferencedSceneId()] = state;
        }
        virtual bool waitCondition() const override
        {
            return m_currentStateMap == m_targetStateMap;
        }
    private:
        const ReferenceStateMap m_targetStateMap;
        ReferenceStateMap m_currentStateMap;
    };

    TEST_F(ARendererLifecycleTest, ReferencedScenesWithDataLinkAndRenderOrder)
    {
        testScenesAndRenderer.initializeRenderer();
        ramses::displayId_t display = createDisplayForWindow();

        // simulate referenced content published
        const ramses::sceneId_t sceneRefId1 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER, Vector3(0.0f, 0.0f, 5.0f));
        const ramses::sceneId_t sceneRefId2 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER, Vector3(-1.5f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneRefId1);
        testScenesAndRenderer.publish(sceneRefId2);
        testScenesAndRenderer.flush(sceneRefId1);
        testScenesAndRenderer.flush(sceneRefId2);

        const ramses::sceneId_t sceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);
        auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
        auto sceneRef1 = masterScene.createSceneReference(sceneRefId1);
        auto sceneRef2 = masterScene.createSceneReference(sceneRefId2);
        sceneRef1->requestState(ramses::RendererSceneState::Rendered);
        sceneRef2->requestState(ramses::RendererSceneState::Rendered);
        sceneRef1->setRenderOrder(1);
        sceneRef2->setRenderOrder(2);
        testScenesAndRenderer.publish(sceneMasterId);
        testScenesAndRenderer.flush(sceneMasterId);

        // get master to ready
        testRenderer.setSceneMapping(sceneMasterId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneMasterId, ramses::RendererSceneState::Ready));

        // get ref scenes to ready (requested rendered but blocked by master)
        ReferencedSceneStateEventHandler handler({ { sceneRefId1, ramses::RendererSceneState::Ready }, { sceneRefId2, ramses::RendererSceneState::Ready } });
        EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(handler));

        // nothing rendered
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        // link data
        masterScene.linkData(sceneRef1, TransformationLinkScene::transformProviderDataId_Left, sceneRef2, TransformationLinkScene::transformConsumerDataId);
        testScenesAndRenderer.flush(sceneMasterId);

        // now all rendered
        ASSERT_TRUE(testRenderer.getSceneToState(sceneMasterId, ramses::RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ReferencedScenes"));

        // kill master, nothing rendered
        testScenesAndRenderer.unpublish(sceneMasterId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.unpublish(sceneRefId1);
        testScenesAndRenderer.unpublish(sceneRefId2);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, ReferencedScenesWithDataLinkAndRenderOrder_Threaded)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        ramses::displayId_t display = createDisplayForWindow();

        // simulate referenced content published
        const ramses::sceneId_t sceneRefId1 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER, Vector3(0.0f, 0.0f, 5.0f));
        const ramses::sceneId_t sceneRefId2 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER, Vector3(-1.5f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneRefId1);
        testScenesAndRenderer.publish(sceneRefId2);
        testScenesAndRenderer.flush(sceneRefId1);
        testScenesAndRenderer.flush(sceneRefId2);

        const ramses::sceneId_t sceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);
        auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
        auto sceneRef1 = masterScene.createSceneReference(sceneRefId1);
        auto sceneRef2 = masterScene.createSceneReference(sceneRefId2);
        sceneRef1->requestState(ramses::RendererSceneState::Ready);
        sceneRef2->requestState(ramses::RendererSceneState::Ready);
        sceneRef1->setRenderOrder(1);
        sceneRef2->setRenderOrder(2);
        testScenesAndRenderer.publish(sceneMasterId);
        testScenesAndRenderer.flush(sceneMasterId);

        // get master to rendered so refs can request also rendered
        testRenderer.setSceneMapping(sceneMasterId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneMasterId, ramses::RendererSceneState::Rendered));
        testScenesAndRenderer.flush(sceneMasterId);

        // get ref scenes to rendered
        // The sequence of operations is intentionally different from non-threaded version of this test
        // in order to increase chance of detecting any potential race condition by thread sanitizer.
        // Following code interleaves executing sceneref commands in render thread and dispatching events sent back in main thread.
        class EventHandler final : public TestClientEventHandler
        {
        public:
            EventHandler(TestScenesAndRenderer& tester_, ramses::sceneId_t sceneMasterId_) : m_tester(tester_), m_sceneMasterId(sceneMasterId_) {}
            virtual void sceneReferenceStateChanged(ramses::SceneReference& sceneRef, ramses::RendererSceneState state) override
            {
                if (state == ramses::RendererSceneState::Ready)
                    sceneRef.requestState(ramses::RendererSceneState::Rendered);
                m_tester.flush(m_sceneMasterId);
                m_states[sceneRef.getReferencedSceneId()] = state;
            }
            virtual bool waitCondition() const override
            {
                return 2u == m_states.size()
                    && std::all_of(m_states.cbegin(), m_states.cend(), [](const auto& it) { return it.second == ramses::RendererSceneState::Rendered; });
            }
        private:
            TestScenesAndRenderer& m_tester;
            ramses::sceneId_t m_sceneMasterId;
            std::unordered_map<ramses::sceneId_t, ramses::RendererSceneState> m_states;
        } handler(testScenesAndRenderer, sceneMasterId);
        EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(handler));

        // link data
        masterScene.linkData(sceneRef1, TransformationLinkScene::transformProviderDataId_Left, sceneRef2, TransformationLinkScene::transformConsumerDataId);
        testScenesAndRenderer.flush(sceneMasterId);

        // now all rendered
        ASSERT_TRUE(checkScreenshot(display, "ReferencedScenes"));

        // kill master, nothing rendered
        testScenesAndRenderer.unpublish(sceneMasterId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.unpublish(sceneRefId1);
        testScenesAndRenderer.unpublish(sceneRefId2);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    class ReferencedSceneFlushEventHandler : public TestClientEventHandler
    {
    public:
        virtual void sceneReferenceFlushed(ramses::SceneReference&, ramses::sceneVersionTag_t versionTag) override
        {
            m_lastReportedVersion = versionTag;
        }
        virtual bool waitCondition() const override
        {
            return m_lastReportedVersion != ramses::InvalidSceneVersionTag;
        }
        ramses::sceneVersionTag_t m_lastReportedVersion = ramses::InvalidSceneVersionTag;
    };

    TEST_F(ARendererLifecycleTest, ReferencedSceneReportsVersionedFlush)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();

        // simulate referenced content published
        const ramses::sceneId_t sceneRefId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneRefId);
        testScenesAndRenderer.flush(sceneRefId);

        const ramses::sceneId_t sceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);
        auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
        auto sceneRef = masterScene.createSceneReference(sceneRefId);
        sceneRef->requestState(ramses::RendererSceneState::Ready);
        testScenesAndRenderer.publish(sceneMasterId);
        testScenesAndRenderer.flush(sceneMasterId);

        // enable receiving of applied versioned flushes
        sceneRef->requestNotificationsForSceneVersionTags(true);
        masterScene.flush();

        // get master to ready
        testRenderer.setSceneMapping(sceneMasterId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneMasterId, ramses::RendererSceneState::Ready));

        // get ref scenes to ready
        ReferencedSceneStateEventHandler stateHandler({ { sceneRefId, ramses::RendererSceneState::Ready } });
        EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(stateHandler));

        constexpr ramses::sceneVersionTag_t version1{ 123u };
        testScenesAndRenderer.flush(sceneRefId, version1);

        // wait for versioned flush reported back to client
        ReferencedSceneFlushEventHandler flushHandler;
        EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(flushHandler));
        EXPECT_EQ(version1, flushHandler.m_lastReportedVersion);

        // another version
        constexpr ramses::sceneVersionTag_t version2{ 666u };
        testScenesAndRenderer.flush(sceneRefId, version2);
        flushHandler.m_lastReportedVersion = ramses::InvalidSceneVersionTag;
        EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(flushHandler));
        EXPECT_EQ(version2, flushHandler.m_lastReportedVersion);

        testScenesAndRenderer.unpublish(sceneMasterId);
        testScenesAndRenderer.unpublish(sceneRefId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, ReferencedSceneReportsLastAppliedVersionWhenEnablingNotifications)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();

        // simulate referenced content published
        const ramses::sceneId_t sceneRefId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneRefId);
        testScenesAndRenderer.flush(sceneRefId);

        const ramses::sceneId_t sceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);
        auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
        auto sceneRef = masterScene.createSceneReference(sceneRefId);
        sceneRef->requestState(ramses::RendererSceneState::Ready);
        testScenesAndRenderer.publish(sceneMasterId);
        testScenesAndRenderer.flush(sceneMasterId);

        // get master to ready
        testRenderer.setSceneMapping(sceneMasterId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneMasterId, ramses::RendererSceneState::Ready));

        // get ref scenes to ready
        ReferencedSceneStateEventHandler stateHandler({ { sceneRefId, ramses::RendererSceneState::Ready } });
        EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(stateHandler));

        constexpr ramses::sceneVersionTag_t version{ 123u };
        testScenesAndRenderer.flush(sceneRefId, version);
        // loop few times to make sure flush was applied
        // Sceneref version flushes are not reported to renderer so there is no way to check,
        // however assumption is that scene in READY state will apply empty flush in single loop.
        testRenderer.doOneLoop();
        testRenderer.doOneLoop();
        testRenderer.doOneLoop();

        // loop few times, expect no flush version reported to client side -> wait condition not met
        ReferencedSceneFlushEventHandler flushHandler;
        EXPECT_FALSE(testScenesAndRenderer.loopTillClientEvent(flushHandler));
        EXPECT_EQ(ramses::InvalidSceneVersionTag, flushHandler.m_lastReportedVersion);

        // only now enable receiving of applied versioned flushes
        sceneRef->requestNotificationsForSceneVersionTags(true);
        masterScene.flush();

        // even though sceneref was not flushed again, expect last applied version to be reported
        EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(flushHandler));
        EXPECT_EQ(version, flushHandler.m_lastReportedVersion);

        testScenesAndRenderer.unpublish(sceneMasterId);
        testScenesAndRenderer.unpublish(sceneRefId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, ReferencedSceneDestroyedAndReusedInAnotherMasterScene)
    {
        testScenesAndRenderer.initializeRenderer();
        ramses::displayId_t display = createDisplayForWindow();

        const ramses::sceneId_t sceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);
        const ramses::sceneId_t sceneRefId1 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER, Vector3(0.0f, 0.0f, 5.0f));
        const ramses::sceneId_t sceneRefId2 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER, Vector3(-1.5f, 0.0f, 5.0f));

        // get master and referenced scenes rendered
        {
            // simulate referenced content published
            testScenesAndRenderer.publish(sceneRefId1);
            testScenesAndRenderer.publish(sceneRefId2);
            testScenesAndRenderer.flush(sceneRefId1);
            testScenesAndRenderer.flush(sceneRefId2);

            auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
            auto sceneRef1 = masterScene.createSceneReference(sceneRefId1, "REF1");
            auto sceneRef2 = masterScene.createSceneReference(sceneRefId2, "REF2");
            sceneRef1->requestState(ramses::RendererSceneState::Rendered);
            sceneRef2->requestState(ramses::RendererSceneState::Rendered);
            sceneRef1->setRenderOrder(1);
            sceneRef2->setRenderOrder(2);
            testScenesAndRenderer.publish(sceneMasterId);
            testScenesAndRenderer.flush(sceneMasterId);

            // get master to ready
            testRenderer.setSceneMapping(sceneMasterId, display);
            ASSERT_TRUE(testRenderer.getSceneToState(sceneMasterId, ramses::RendererSceneState::Ready));

            // get ref scenes to ready (requested rendered but blocked by master)
            ReferencedSceneStateEventHandler handler({ { sceneRefId1, ramses::RendererSceneState::Ready }, { sceneRefId2, ramses::RendererSceneState::Ready } });
            EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(handler));

            // nothing rendered
            ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

            // link data
            masterScene.linkData(sceneRef1, TransformationLinkScene::transformProviderDataId_Left, sceneRef2, TransformationLinkScene::transformConsumerDataId);
            testScenesAndRenderer.flush(sceneMasterId);

            // now all rendered
            ASSERT_TRUE(testRenderer.getSceneToState(sceneMasterId, ramses::RendererSceneState::Rendered));
            ASSERT_TRUE(checkScreenshot(display, "ReferencedScenes"));
        }

        // 'unref' scenes by destroying the HL objects
        {
            // first ramp down the scenes' states
            auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
            auto& sceneRef1 = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::SceneReference>(*masterScene.findObjectByName("REF1"));
            auto& sceneRef2 = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::SceneReference>(*masterScene.findObjectByName("REF2"));
            sceneRef1.requestState(ramses::RendererSceneState::Available);
            sceneRef2.requestState(ramses::RendererSceneState::Available);
            testScenesAndRenderer.flush(sceneMasterId);
            ReferencedSceneStateEventHandler stateHandler({ { sceneRefId1, ramses::RendererSceneState::Available }, { sceneRefId2, ramses::RendererSceneState::Available } });
            EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(stateHandler));

            masterScene.destroy(sceneRef1);
            masterScene.destroy(sceneRef2);
            testScenesAndRenderer.flush(sceneMasterId);
            ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));
        }

        // create another master (same content as original master)
        const ramses::sceneId_t otherSceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);

        // reference existing scenes by new master and get all rendered
        {
            auto& otherMasterScene = testScenesAndRenderer.getScenesRegistry().getScene(otherSceneMasterId);
            auto sceneRef1 = otherMasterScene.createSceneReference(sceneRefId1);
            auto sceneRef2 = otherMasterScene.createSceneReference(sceneRefId2);
            sceneRef1->requestState(ramses::RendererSceneState::Rendered);
            sceneRef2->requestState(ramses::RendererSceneState::Rendered);
            sceneRef1->setRenderOrder(1);
            sceneRef2->setRenderOrder(2);
            testScenesAndRenderer.publish(otherSceneMasterId);
            testScenesAndRenderer.flush(otherSceneMasterId);

            // get master to ready
            testRenderer.setSceneMapping(otherSceneMasterId, display);
            testRenderer.getSceneToState(otherSceneMasterId, ramses::RendererSceneState::Ready);

            // get ref scenes to ready (requested rendered but blocked by master)
            ReferencedSceneStateEventHandler handler({ { sceneRefId1, ramses::RendererSceneState::Ready }, { sceneRefId2, ramses::RendererSceneState::Ready } });
            EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(handler));

            // nothing rendered
            ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

            // link data
            otherMasterScene.linkData(sceneRef1, TransformationLinkScene::transformProviderDataId_Left, sceneRef2, TransformationLinkScene::transformConsumerDataId);
            testScenesAndRenderer.flush(otherSceneMasterId);

            // now all rendered
            testRenderer.getSceneToState(otherSceneMasterId, ramses::RendererSceneState::Rendered);
            ASSERT_TRUE(checkScreenshot(display, "ReferencedScenes"));
        }

        testScenesAndRenderer.unpublish(sceneMasterId);
        testScenesAndRenderer.unpublish(otherSceneMasterId);
        testScenesAndRenderer.unpublish(sceneRefId1);
        testScenesAndRenderer.unpublish(sceneRefId2);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, ReferencedSceneDestroyedAndReusedInAnotherMasterScene_whileRendered)
    {
        testScenesAndRenderer.initializeRenderer();
        ramses::displayId_t display = createDisplayForWindow();

        const ramses::sceneId_t sceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);
        const ramses::sceneId_t sceneRefId1 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER, Vector3(0.0f, 0.0f, 5.0f));
        const ramses::sceneId_t sceneRefId2 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER, Vector3(-1.5f, 0.0f, 5.0f));

        // get master and referenced scenes rendered
        {
            // simulate referenced content published
            testScenesAndRenderer.publish(sceneRefId1);
            testScenesAndRenderer.publish(sceneRefId2);
            testScenesAndRenderer.flush(sceneRefId1);
            testScenesAndRenderer.flush(sceneRefId2);

            auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
            auto sceneRef1 = masterScene.createSceneReference(sceneRefId1, "REF1");
            auto sceneRef2 = masterScene.createSceneReference(sceneRefId2, "REF2");
            sceneRef1->requestState(ramses::RendererSceneState::Rendered);
            sceneRef2->requestState(ramses::RendererSceneState::Rendered);
            sceneRef1->setRenderOrder(1);
            sceneRef2->setRenderOrder(2);
            testScenesAndRenderer.publish(sceneMasterId);
            testScenesAndRenderer.flush(sceneMasterId);

            // get master to ready
            testRenderer.setSceneMapping(sceneMasterId, display);
            ASSERT_TRUE(testRenderer.getSceneToState(sceneMasterId, ramses::RendererSceneState::Ready));

            // get ref scenes to ready (requested rendered but blocked by master)
            ReferencedSceneStateEventHandler handler({ { sceneRefId1, ramses::RendererSceneState::Ready }, { sceneRefId2, ramses::RendererSceneState::Ready } });
            EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(handler));

            // nothing rendered
            ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

            // link data
            masterScene.linkData(sceneRef1, TransformationLinkScene::transformProviderDataId_Left, sceneRef2, TransformationLinkScene::transformConsumerDataId);
            testScenesAndRenderer.flush(sceneMasterId);

            // now all rendered
            ASSERT_TRUE(testRenderer.getSceneToState(sceneMasterId, ramses::RendererSceneState::Rendered));
            ASSERT_TRUE(checkScreenshot(display, "ReferencedScenes"));
        }

        // 'unref' scenes by destroying the HL objects
        {
            // just unref, dont ramp down any of the rendering states
            auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
            auto& sceneRef1 = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::SceneReference>(*masterScene.findObjectByName("REF1"));
            auto& sceneRef2 = ramses::RamsesObjectTypeUtils::ConvertTo<ramses::SceneReference>(*masterScene.findObjectByName("REF2"));
            masterScene.destroy(sceneRef1);
            masterScene.destroy(sceneRef2);
            testScenesAndRenderer.flush(sceneMasterId);
            // still rendered
            ASSERT_TRUE(checkScreenshot(display, "ReferencedScenes"));
        }

        // create another master (same content as original master)
        const ramses::sceneId_t otherSceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);

        // reference existing scenes by new master and get all rendered
        {
            // get new master to rendered first
            testScenesAndRenderer.publish(otherSceneMasterId);
            testScenesAndRenderer.flush(otherSceneMasterId);
            testRenderer.setSceneMapping(otherSceneMasterId, display);
            ASSERT_TRUE(testRenderer.getSceneToState(otherSceneMasterId, ramses::RendererSceneState::Rendered));

            // reference scenes again, now from new master
            auto& otherMasterScene = testScenesAndRenderer.getScenesRegistry().getScene(otherSceneMasterId);
            auto sceneRef1 = otherMasterScene.createSceneReference(sceneRefId1);
            auto sceneRef2 = otherMasterScene.createSceneReference(sceneRefId2);
            sceneRef1->requestState(ramses::RendererSceneState::Rendered);
            sceneRef2->requestState(ramses::RendererSceneState::Rendered);
            sceneRef1->setRenderOrder(1);
            sceneRef2->setRenderOrder(2);
            testScenesAndRenderer.flush(otherSceneMasterId);

            // new master is rendered and took over the referenced scenes which were already rendered, so rendered output did not change
            ASSERT_TRUE(checkScreenshot(display, "ReferencedScenes"));

            // now change state of ref scenes while referenced by new master
            sceneRef1->requestState(ramses::RendererSceneState::Ready);
            sceneRef2->requestState(ramses::RendererSceneState::Ready);
            testScenesAndRenderer.flush(otherSceneMasterId);

            // nothing rendered
            ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));
        }

        testScenesAndRenderer.unpublish(sceneMasterId);
        testScenesAndRenderer.unpublish(otherSceneMasterId);
        testScenesAndRenderer.unpublish(sceneRefId1);
        testScenesAndRenderer.unpublish(sceneRefId2);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, PollingFrameCallbacks_DoesNotBlockIfNoDisplaysExist)
    {
        const std::chrono::seconds largePollingTime{100u};
        auto rendererConfig = RendererTestUtils::CreateTestRendererConfig();
        rendererConfig.setFrameCallbackMaxPollTime(std::chrono::duration_cast<std::chrono::microseconds>(largePollingTime).count());

        testScenesAndRenderer.initializeRenderer(rendererConfig);

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
        auto rendererConfig = RendererTestUtils::CreateTestRendererConfig();
        rendererConfig.setFrameCallbackMaxPollTime(std::chrono::duration_cast<std::chrono::microseconds>(nonTrivialPollingTime).count());

        testScenesAndRenderer.initializeRenderer(rendererConfig);

        createDisplayForWindow(0u, false);

        if (testRenderer.hasSystemCompositorController())
        {
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
        auto rendererConfig = RendererTestUtils::CreateTestRendererConfig();
        rendererConfig.setFrameCallbackMaxPollTime(std::chrono::duration_cast<std::chrono::microseconds>(nonTrivialPollingTime).count());

        testScenesAndRenderer.initializeRenderer(rendererConfig);

        createDisplayForWindow(0u, false);
        createDisplayForWindow(1u, false);

        if (testRenderer.hasSystemCompositorController())
        {
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

    TEST_F(ARendererLifecycleTest, PollingFrameCallbacks_ReadyDisplayDoesNotStarveOtherDisplay)
    {
        const std::chrono::milliseconds nonTrivialPollingTime{50u};
        auto rendererConfig = RendererTestUtils::CreateTestRendererConfig();
        rendererConfig.setFrameCallbackMaxPollTime(std::chrono::duration_cast<std::chrono::microseconds>(nonTrivialPollingTime).count());

        testScenesAndRenderer.initializeRenderer(rendererConfig);

        const ramses::displayId_t display1 = createDisplayForWindow(0u, true);
        createDisplayForWindow(1u, true); //nothing gets rendered on it, so it is ALWAYS ready (except right after clearing)

        if (testRenderer.hasSystemCompositorController())
        {
            ASSERT_TRUE(checkScreenshot(display1, "ARendererDisplays_Black"));

            const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));

            testScenesAndRenderer.publish(sceneId);
            testScenesAndRenderer.flush(sceneId);
            testRenderer.setSceneMapping(sceneId, display1);
            ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

            ASSERT_TRUE(checkScreenshot(display1, "ARendererInstance_Three_Triangles"));

            //render again and make sure the display was updated
            testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.flush(sceneId);
            //taking the screenshot would timeout if display1 is being starved by the (always ready) other display
            ASSERT_TRUE(checkScreenshot(display1, "ARendererInstance_Triangles_reordered"));
        }

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, PollingFrameCallbacks_ReadyDisplayDoesNotStarveOtherDisplay_DisplaysInOtherOrder)
    {
        const std::chrono::milliseconds nonTrivialPollingTime{50u};
        auto rendererConfig = RendererTestUtils::CreateTestRendererConfig();
        rendererConfig.setFrameCallbackMaxPollTime(std::chrono::duration_cast<std::chrono::microseconds>(nonTrivialPollingTime).count());

        testScenesAndRenderer.initializeRenderer(rendererConfig);

        createDisplayForWindow(0u, true); //nothing gets rendered on it, so it is ALWAYS ready (except right after clearing)
        const ramses::displayId_t display2 = createDisplayForWindow(1u, true);

        if (testRenderer.hasSystemCompositorController())
        {
            ASSERT_TRUE(checkScreenshot(display2, "ARendererDisplays_Black"));

            const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));

            testScenesAndRenderer.publish(sceneId);
            testScenesAndRenderer.flush(sceneId);
            testRenderer.setSceneMapping(sceneId, display2);
            ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

            ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_Three_Triangles"));

            //render again and make sure the display was updated
            testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.flush(sceneId);
            //taking the screenshot would timeout if display1 is being starved by the (always ready) other display
            ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_Triangles_reordered"));
        }

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneNotExpiredWhenUpdatedAndSubscribed)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();

        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Available));

        for (int i = 0; i < 5; ++i)
        {
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::hours(1));
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }

        ASSERT_FALSE(testRenderer.checkScenesExpired({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneExpiredWhenSubscribed)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();

        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Ready));
        testRenderer.doOneLoop();

        // next flush expired already in past to trigger the exceeded event
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() - std::chrono::hours(1));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        ASSERT_TRUE(testRenderer.checkScenesExpired({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneExpiredAndRecoveredWhenSubscribed)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();

        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);

        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::hours(1));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Ready));
        testRenderer.doOneLoop();

        // next flush will be in past to trigger the exceeded event
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() - std::chrono::hours(1));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();
        ASSERT_TRUE(testRenderer.checkScenesExpired({ sceneId }));

        // next flush will be in future again to trigger the recovery event
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::hours(1));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        ASSERT_TRUE(testRenderer.checkScenesNotExpired({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneNotExpiredWhenUpdatedAndRendered)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
        testRenderer.doOneLoop();

        // send flushes and render within limit
        for (int i = 0; i < 5; ++i)
        {
            // make modifications to scene
            testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::hours(1));
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }

        ASSERT_FALSE(testRenderer.checkScenesExpired({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneNotExpiredWhenUpdatedWithEmptyFlushesAndRendered)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
        testRenderer.doOneLoop();

        // send flushes and render within limit
        for (int i = 0; i < 5; ++i)
        {
            // no modifications to scene
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::hours(1));
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }

        ASSERT_FALSE(testRenderer.checkScenesExpired({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneExpiredWhenRendered)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
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
            testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(500)); // these will not expire
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        ASSERT_TRUE(testRenderer.checkScenesExpired({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneExpiredWhenRenderedAndRecoveredAfterHidden)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
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
            testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(500)); // these will not expire
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        // rendered content expired
        ASSERT_TRUE(testRenderer.checkScenesExpired({ sceneId }));

        // make sure the scene is still expired till after hidden
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() - std::chrono::hours(1));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        // now hide scene so regular flushes are enough to recover
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Ready));
        for (int i = 0; i < 5; ++i)
        {
            // make modifications to scene
            testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }
        ASSERT_TRUE(testRenderer.checkScenesNotExpired({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneExpirationCanBeDisabled_ConfidenceTest)
    {
        struct ExpirationCounter final : public ramses::RendererSceneControlEventHandlerEmpty
        {
            virtual void sceneExpired(ramses::sceneId_t) override
            {
                numExpirationEvents++;
            }
            size_t numExpirationEvents = 0u;
        } expirationCounter;

        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
        testRenderer.doOneLoop();

        // set expiration of content that will be rendered
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(5000));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        // send flushes within limit and render
        for (int i = 0; i < 3; ++i)
        {
            // make modifications to scene
            testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(5000)); // these will not expire
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }
        ramses::RendererEventHandlerEmpty dummy;
        testRenderer.dispatchEvents(dummy, expirationCounter);
        ASSERT_TRUE(expirationCounter.numExpirationEvents == 0u);

        // now hide scene
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Ready));

        // send few more flushes within limit and no changes
        for (int i = 0; i < 3; ++i)
        {
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(5000)); // these will not expire
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }
        testRenderer.dispatchEvents(dummy, expirationCounter);
        ASSERT_TRUE(expirationCounter.numExpirationEvents == 0u);

        // disable expiration together with scene changes
        testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
        testScenesAndRenderer.flush(sceneId);
        // intentionally send empty flush with expiration disabling only
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::InvalidTimestamp);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        // stop sending flushes altogether but keep looping,
        // loop long enough to prove that expiration checking was really disabled,
        // i.e. render past the last non-zero expiration TS set
        for (int i = 0; i < 5; ++i)
        {
            testRenderer.doOneLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
        testRenderer.dispatchEvents(dummy, expirationCounter);
        ASSERT_TRUE(expirationCounter.numExpirationEvents == 0u);

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneExpiredAndRecoveredWhenRendered)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
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
            testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        ASSERT_TRUE(testRenderer.checkScenesExpired({ sceneId }));

        // now also render within limit to recover
        testRenderer.setLoopMode(ramses::ELoopMode_UpdateAndRender);
        for (int i = 0; i < 5; ++i)
        {
            // make modifications to scene
            testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.flush(sceneId);
            testRenderer.doOneLoop();
        }
        ASSERT_TRUE(testRenderer.checkScenesNotExpired({ sceneId }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, ScenesExpireOneAfterAnother)
    {
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const ramses::sceneId_t sceneId1 = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(-0.50f, 1.0f, 5.0f));
        const ramses::sceneId_t sceneId2 = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId1);
        testScenesAndRenderer.publish(sceneId2);
        testScenesAndRenderer.flush(sceneId1);
        testScenesAndRenderer.flush(sceneId2);
        testRenderer.setSceneMapping(sceneId1, display);
        testRenderer.setSceneMapping(sceneId2, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId1, ramses::RendererSceneState::Rendered));
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId2, ramses::RendererSceneState::Rendered));
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
        ASSERT_TRUE(testRenderer.checkScenesExpired({ sceneId1 }));

        // S1 recovers, S2 is ok
        for (int i = 0; i < 5; ++i)
        {
            testScenesAndRenderer.setExpirationTimestamp(sceneId1, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.setExpirationTimestamp(sceneId2, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.flush(sceneId1);
            testScenesAndRenderer.flush(sceneId2);
            testRenderer.doOneLoop();
        }
        ASSERT_TRUE(testRenderer.checkScenesNotExpired({ sceneId1 }));

        // S1 ok, S2 exceeds
        for (int i = 0; i < 5; ++i)
        {
            testScenesAndRenderer.setExpirationTimestamp(sceneId1, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.flush(sceneId1);
            testRenderer.doOneLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        ASSERT_TRUE(testRenderer.checkScenesExpired({ sceneId2 }));

        // S1 ok, S2 is recovers
        for (int i = 0; i < 5; ++i)
        {
            testScenesAndRenderer.setExpirationTimestamp(sceneId1, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.setExpirationTimestamp(sceneId2, FlushTime::Clock::now() + std::chrono::milliseconds(500));
            testScenesAndRenderer.flush(sceneId1);
            testScenesAndRenderer.flush(sceneId2);
            testRenderer.doOneLoop();
        }
        ASSERT_TRUE(testRenderer.checkScenesNotExpired({ sceneId2 }));

        // both S1 and S2 exceed
        for (int i = 0; i < 5; ++i)
        {
            testRenderer.doOneLoop();
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        ASSERT_TRUE(testRenderer.checkScenesExpired({ sceneId1, sceneId2 }));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, HandlesSwitchingTriStateVisibilityFullyOffAndOn)
    {
        testScenesAndRenderer.initializeRenderer();
        const auto display = createDisplayForWindow();
        ASSERT_TRUE(ramses::displayId_t::Invalid() != display);

        const auto sceneId = createScene<VisibilityScene>(VisibilityScene::VISIBILITY_ALL_OFF, Vector3(0.f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_OFF_AND_INVISIBLE);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_OFF_AND_VISIBLE);
        testScenesAndRenderer.flush(sceneId, ramses::sceneVersionTag_t{1u});
        testRenderer.waitForFlush(sceneId, ramses::sceneVersionTag_t{1u});
        testRenderer.doOneLoop();
        ASSERT_TRUE(checkScreenshot(display, "VisibilityScene_Partial"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_ALL_VISIBLE);
        testScenesAndRenderer.flush(sceneId, ramses::sceneVersionTag_t{2u});
        testRenderer.waitForFlush(sceneId, ramses::sceneVersionTag_t{2u});
        ASSERT_TRUE(checkScreenshot(display, "VisibilityScene_On"));

        // another cycle of OFF to ON
        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_ALL_OFF);
        testScenesAndRenderer.flush(sceneId, ramses::sceneVersionTag_t{3u});
        testRenderer.waitForFlush(sceneId, ramses::sceneVersionTag_t{3u});
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_OFF_AND_INVISIBLE);
        testScenesAndRenderer.flush(sceneId, ramses::sceneVersionTag_t{4u});
        testRenderer.waitForFlush(sceneId, ramses::sceneVersionTag_t{4u});
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_OFF_AND_VISIBLE);
        testScenesAndRenderer.flush(sceneId, ramses::sceneVersionTag_t{5u});
        testRenderer.waitForFlush(sceneId, ramses::sceneVersionTag_t{5u});
        ASSERT_TRUE(checkScreenshot(display, "VisibilityScene_Partial"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_ALL_VISIBLE);
        testScenesAndRenderer.flush(sceneId, ramses::sceneVersionTag_t{6u});
        testRenderer.waitForFlush(sceneId, ramses::sceneVersionTag_t{6u});
        ASSERT_TRUE(checkScreenshot(display, "VisibilityScene_On"));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, canDestroyAndRecreateRendererOnSameFramework)
    {
        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display_1 = createDisplayForWindow();
        ASSERT_TRUE(display_1 != ramses::displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display_1);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display_1, "ARendererInstance_Three_Triangles"));

        // destroy and recreate renderer
        testScenesAndRenderer.destroyRenderer();
        testScenesAndRenderer.initializeRenderer();

        const ramses::displayId_t display_2 = createDisplayForWindow();
        ASSERT_TRUE(display_2 != ramses::displayId_t::Invalid());

        testRenderer.setSceneMapping(sceneId, display_2);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display_2, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, VertexArrayGetsUploadedWhenRenderableResourcesUploaded)
    {
        // Test dirtiness of VAO is handled correctly in case renderable stays dirty after geometry
        // resources are ready, e.g., in case VAO upload is blocked till other renderable resources
        // get uploaded
        const ramses::sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<TextureSamplerScene>(TextureSamplerScene::EState_NoTextureSampler, Vector3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const ramses::displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(display != ramses::displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

        testScenesAndRenderer.getScenesRegistry().setSceneState<TextureSamplerScene>(sceneId, TextureSamplerScene::EState_SetTextureSampler);
        testScenesAndRenderer.flush(sceneId, ramses::sceneVersionTag_t{ 1u });
        testRenderer.waitForFlush(sceneId, ramses::sceneVersionTag_t{ 1u });

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }
}
