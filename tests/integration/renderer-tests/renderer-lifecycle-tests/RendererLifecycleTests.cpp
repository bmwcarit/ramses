//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLifecycleTests.h"

// These includes are needed by the tests
#include "ramses/framework/RamsesFrameworkConfig.h"
#include "ReadPixelCallbackHandler.h"
#include "TestScenes/MultipleTrianglesScene.h"
#include "TestScenes/TextureBufferScene.h"
#include "TestScenes/ArrayBufferScene.h"
#include "TestScenes/TextScene.h"
#include "TestScenes/FileLoadingScene.h"
#include "TestScenes/Texture2DFormatScene.h"
#include "TestScenes/TextureSamplerScene.h"
#include "TestScenes/TransformationLinkScene.h"
#include "TestScenes/VisibilityScene.h"
#include "TestScenes/RenderTargetScene.h"
#include "TestScenes/LogicScene.h"
#include "internal/Core/Utils/File.h"
#include "internal/Core/Utils/ThreadBarrier.h"

// These includes are needed because of ramses API usage
#include "ramses/renderer/IRendererEventHandler.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/client/DataObject.h"
#include "ramses/client/SceneReference.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/Property.h"
#include "impl/RamsesObjectTypeUtils.h"
#include "impl/ValidationReportImpl.h"

#include <thread>
#include <unordered_map>

namespace ramses::internal
{
    TEST_F(ARendererLifecycleTest, RenderScene)
    {
        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(display != displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RecreateSceneWithSameId)
    {
        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.getScenesRegistry().destroyScene(sceneId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, sceneId, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SaveLoadSceneFromFileThenRender)
    {
        const sceneId_t sceneId(1234u);

        testScenesAndRenderer.getScenesRegistry().createFileLoadingScene(sceneId, glm::vec3(0.0f, 0.0f, 5.0f), frameworkConfig, FileLoadingScene::CREATE_SAVE_DESTROY_LOAD_USING_SEPARATE_CLIENT, WindowWidth, WindowHeight);
        const auto validateResult = testScenesAndRenderer.validateScene(sceneId);
        ASSERT_FALSE(validateResult.hasIssue()) << validateResult.impl().toString();

        testScenesAndRenderer.initializeRenderer();
        const displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_AfterLoadSave"));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SaveLoadSceneFromFileThenRender_Threaded)
    {
        const sceneId_t sceneId(1234u);

        testScenesAndRenderer.getScenesRegistry().createFileLoadingScene(sceneId, glm::vec3(0.0f, 0.0f, 5.0f), frameworkConfig, FileLoadingScene::CREATE_SAVE_DESTROY_LOAD_USING_SEPARATE_CLIENT, WindowWidth, WindowHeight);
        const auto validateResult = testScenesAndRenderer.validateScene(sceneId);
        ASSERT_FALSE(validateResult.hasIssue()) << validateResult.impl().toString();

        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        const displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
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
        const sceneId_t sceneId(1234u);

        testScenesAndRenderer.getScenesRegistry().createFileLoadingScene(sceneId, glm::vec3(0.0f, 0.0f, 5.0f), frameworkConfig, FileLoadingScene::CREATE_SAVE_DESTROY_LOAD_USING_SEPARATE_CLIENT, WindowWidth, WindowHeight);
        const auto validateResult = testScenesAndRenderer.validateScene(sceneId);
        ASSERT_FALSE(validateResult.hasIssue()) << validateResult.impl().toString();

        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();

        for (uint32_t i = 0; i < 10; ++i)
        {
            const displayId_t display = createDisplayForWindow(i);

            testScenesAndRenderer.publish(sceneId);
            testRenderer.setSceneMapping(sceneId, display);
            ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
            testScenesAndRenderer.flush(sceneId, 1);
            testRenderer.waitForFlush(sceneId, 1);

            ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_AfterLoadSave"));

            testScenesAndRenderer.unpublish(sceneId);
            ASSERT_TRUE(testScenesAndRenderer.waitForSceneStateChange(sceneId, RendererSceneState::Unavailable));
            testRenderer.destroyDisplay(display);
        }

        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, DestroyAndRecreateRenderer)
    {
        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();

        testScenesAndRenderer.initializeRenderer();
        display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

#if defined(RAMSES_TEXT_ENABLED)
    TEST_F(ARendererLifecycleTest, DestroyRenderer_ChangeScene_ThenRecreateRenderer)
    {
        const sceneId_t sceneId = createScene<TextScene>(TextScene::EState_INITIAL);
        testScenesAndRenderer.initializeRenderer();
        displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();

        testScenesAndRenderer.getScenesRegistry().setSceneState<TextScene>(sceneId, TextScene::EState_INITIAL_128_BY_64_VIEWPORT);

        testScenesAndRenderer.initializeRenderer();
        display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_SimpleText"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }
#endif

    TEST_F(ARendererLifecycleTest, MergeScenes)
    {
        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(display != displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        auto& client = testScenesAndRenderer.getClient();
        // create two more scenes and save to files
        for (size_t i = 0; i < 2; ++i) {
            int sign = (i%2 != 0u)? -1 : 1;
            const sceneId_t sid = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(3.0f * sign, 1.0f * sign, 3.0f));
            auto& newScene = testScenesAndRenderer.getScenesRegistry().getScene(sid);
            ASSERT_TRUE(newScene.saveToFile(fmt::format("newSceneToMerge{}.ramses", i)));
            EXPECT_TRUE(client.destroy(newScene));
        }

        auto& renderedScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneId);
        for (size_t i = 0; i < 2; ++i) {
            ASSERT_TRUE(client.mergeSceneFromFile(renderedScene, fmt::format("newSceneToMerge{}.ramses", i)));
        }

        testScenesAndRenderer.flush(sceneId);
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_MergeScenes"));
        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, UnsubscribeRenderer_ChangeScene_ThenResubscribeRenderer)
    {
        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Available));
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Triangles_reordered"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, ChangeScene_UnsubscribeRenderer_Flush_ThenResubscribeRenderer)
    {
        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(display != displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Available));
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Triangles_reordered"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RAMSES2881_CreateRendererAfterScene)
    {
        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);

        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, DestroyDisplayAndRemapSceneToOtherDisplay)
    {
        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const displayId_t display1 = createDisplayForWindow(0u);
        const displayId_t display2 = createDisplayForWindow(1u);
        ASSERT_TRUE(displayId_t::Invalid() != display1);
        ASSERT_TRUE(displayId_t::Invalid() != display2);

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display1);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Available));

        testRenderer.destroyDisplay(display1);

        testRenderer.setSceneMapping(sceneId, display2);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, DestroyDisplayAndRemapSceneToOtherDisplay_LocalOnly)
    {
        ramses::SceneConfig config;
        config.setPublicationMode(ramses::EScenePublicationMode::LocalOnly);
        const ramses::sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f), WindowWidth, WindowHeight, config);
        testScenesAndRenderer.initializeRenderer();
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);

        {
            const ramses::displayId_t display0 = createDisplayForWindow(0u);
            ASSERT_TRUE(ramses::displayId_t::Invalid() != display0);

            testRenderer.setSceneMapping(sceneId, display0);
            testRenderer.setSceneState(sceneId, ramses::RendererSceneState::Rendered);
            testRenderer.doOneLoop(); // calls addSubscriber, but does NOT send the scene
            // LocalOnly scenes cannot send the initial flush automatically
            testRenderer.doOneLoop();
            testRenderer.doOneLoop();
            testRenderer.setSceneState(sceneId, ramses::RendererSceneState::Available); // unsubscribes the scene
            testRenderer.doOneLoop();
            testRenderer.doOneLoop();
            testRenderer.destroyDisplay(display0);
        }

        {
            testScenesAndRenderer.flush(sceneId);
            const ramses::displayId_t display1 = createDisplayForWindow(1u);
            ASSERT_TRUE(ramses::displayId_t::Invalid() != display1);
            testRenderer.setSceneMapping(sceneId, display1);
            testRenderer.setSceneState(sceneId, ramses::RendererSceneState::Rendered);
            testRenderer.doOneLoop(); // calls addSubscriber
            testScenesAndRenderer.flush(sceneId); // sends the scene !
            ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, ramses::RendererSceneState::Rendered));

            ASSERT_TRUE(checkScreenshot(display1, "ARendererInstance_Three_Triangles"));
        }

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RenderScene_Threaded)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);

        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

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
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));

        // create data to change
        auto data = testScenesAndRenderer.getScenesRegistry().getScene(sceneId).createDataObject(ramses::EDataType::Int32);
        testScenesAndRenderer.flush(sceneId);

        testScenesAndRenderer.publish(sceneId);

        //do not wait for subscription
        testRenderer.setSceneState(sceneId, RendererSceneState::Available);

        // change scene while subscription is ongoing
        for (int i = 0; i < 80; i++)
        {
            data->setValue(i);
            testScenesAndRenderer.flush(sceneId);
        }
        ASSERT_TRUE(testScenesAndRenderer.waitForSceneStateChange(sceneId, RendererSceneState::Available));

        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

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
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

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
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RendererUploadsResourcesIfIviSurfaceInvisible)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        const displayId_t display = createDisplayForWindow(0u, false);
        ASSERT_TRUE(display != displayId_t::Invalid());

        const sceneId_t sceneId = createScene<Texture2DFormatScene>(Texture2DFormatScene::EState_R8, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        // this would time out if resources for the scene could not be uploaded

        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RendererUploadsResourcesIfIviSurfaceInvisibleInLoopModeUpdateOnly)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.setLoopMode(ELoopMode::UpdateOnly);
        testRenderer.startRendererThread();
        const displayId_t display = createDisplayForWindow(0u, false);
        ASSERT_TRUE(display != displayId_t::Invalid());

        const sceneId_t sceneId = createScene<Texture2DFormatScene>(Texture2DFormatScene::EState_R8, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        // this would time out if resources for the scene could not be uploaded

        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, RemapScenesWithDynamicResourcesToOtherDisplay)
    {
        const sceneId_t sceneId1 = createScene<TextureBufferScene>(TextureBufferScene::EState_RGBA8_OneMip_ScaledDown, glm::vec3(-0.1f, -0.1f, 15.0f), 200u, 200u); // stretch a bit by using bigger viewport
        const sceneId_t sceneId2 = createScene<ArrayBufferScene>(ArrayBufferScene::INDEX_DATA_BUFFER_UINT16, glm::vec3(-2.0f, -2.0f, 15.0f));
        testScenesAndRenderer.initializeRenderer();
        const displayId_t display1 = createDisplayForWindow(0u);
        const displayId_t display2 = createDisplayForWindow(1u);
        ASSERT_TRUE(displayId_t::Invalid() != display1);
        ASSERT_TRUE(displayId_t::Invalid() != display2);

        testScenesAndRenderer.publish(sceneId1);
        testScenesAndRenderer.publish(sceneId2);
        testScenesAndRenderer.flush(sceneId1);
        testScenesAndRenderer.flush(sceneId2);
        testRenderer.setSceneMapping(sceneId1, display1);
        testRenderer.setSceneMapping(sceneId2, display1);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId1, RendererSceneState::Rendered));
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId2, RendererSceneState::Rendered));

        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId1, RendererSceneState::Available));
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId2, RendererSceneState::Available));

        ASSERT_TRUE(checkScreenshot(display1, "ARendererDisplays_Black"));

        testRenderer.setSceneMapping(sceneId1, display2);
        testRenderer.setSceneMapping(sceneId2, display2);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId1, RendererSceneState::Rendered));
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId2, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_DynamicResources"));

        testScenesAndRenderer.unpublish(sceneId1);
        testScenesAndRenderer.unpublish(sceneId2);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneCanReachShownStateWithLoopModeUpdateOnly_UsingDoOneLoop)
    {
        const sceneId_t sceneId = createScene<Texture2DFormatScene>(Texture2DFormatScene::EState_R8, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        testRenderer.setLoopMode(ELoopMode::UpdateOnly);

        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(display != displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneCanReachShownStateWithLoopModeUpdateOnly_UsingRenderThread)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        testRenderer.setLoopMode(ELoopMode::UpdateOnly);

        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const sceneId_t sceneId = createScene<Texture2DFormatScene>(Texture2DFormatScene::EState_R8, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, SceneCanReachShownStateWithLoopModeUpdateOnly_IfIviSurfaceInvisible)
    {
        testScenesAndRenderer.initializeRenderer();
        testRenderer.startRendererThread();
        testRenderer.setLoopMode(ELoopMode::UpdateOnly);

        const displayId_t display = createDisplayForWindow(0u, false);
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const sceneId_t sceneId = createScene<Texture2DFormatScene>(Texture2DFormatScene::EState_R8, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        testScenesAndRenderer.unpublish(sceneId);
        testRenderer.destroyDisplay(display);
        testRenderer.stopRendererThread();
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, DoesNotRenderToFramebufferInLoopModeUpdateOnly)
    {
        testScenesAndRenderer.initializeRenderer();
        const displayId_t display = createDisplayForWindow(0u);
        ASSERT_TRUE(display != displayId_t::Invalid());

        testRenderer.setLoopMode(ELoopMode::UpdateOnly);
        testRenderer.readPixels(display, 0u, 0u, WindowWidth, WindowHeight);
        testRenderer.flushRenderer();
        testRenderer.doOneLoop();

        ReadPixelCallbackHandler callbackHandler;
        RendererSceneControlEventHandlerEmpty dummy;
        testRenderer.dispatchEvents(callbackHandler, dummy);
        ASSERT_FALSE(callbackHandler.m_pixelDataRead);

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, Republish_ThenChangeScene)
    {
        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        displayId_t display = createDisplayForWindow();

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        ASSERT_TRUE(testScenesAndRenderer.waitForSceneStateChange(sceneId, RendererSceneState::Unavailable));

        testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);

        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererInstance_Triangles_reordered"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    class ReferencedSceneStateEventHandler final : public TestClientEventHandler
    {
    public:
        using ReferenceStateMap = std::unordered_map<sceneId_t, RendererSceneState>;

        explicit ReferencedSceneStateEventHandler(ReferenceStateMap&& targetStateMap, TestScenes& scenes)
            : m_targetStateMap{ std::move(targetStateMap) }
            , m_scenes{ scenes }
        {}

        void sceneReferenceStateChanged(SceneReference& sceneRef, RendererSceneState state) override
        {
            m_currentStateMap[sceneRef.getReferencedSceneId()] = state;
        }
        bool waitCondition() const override
        {
            return m_currentStateMap == m_targetStateMap;
        }
        void onUpdate() override
        {
            for (const auto& s : m_targetStateMap)
                m_scenes.getScene(s.first).flush();
        }

    private:
        const ReferenceStateMap m_targetStateMap;
        ReferenceStateMap m_currentStateMap;
        TestScenes& m_scenes;
    };

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    TEST_F(ARendererLifecycleTest, ReferencedScenesWithDataLinkAndRenderOrder)
    {
        testScenesAndRenderer.initializeRenderer();
        displayId_t display = createDisplayForWindow();

        // simulate referenced content published
        const sceneId_t sceneRefId1 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER, glm::vec3(0.0f, 0.0f, 5.0f));
        const sceneId_t sceneRefId2 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER, glm::vec3(-1.5f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneRefId1);
        testScenesAndRenderer.publish(sceneRefId2);
        testScenesAndRenderer.flush(sceneRefId1);
        testScenesAndRenderer.flush(sceneRefId2);

        const sceneId_t sceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);
        auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
        auto sceneRef1 = masterScene.createSceneReference(sceneRefId1);
        auto sceneRef2 = masterScene.createSceneReference(sceneRefId2);
        sceneRef1->requestState(RendererSceneState::Rendered);
        sceneRef2->requestState(RendererSceneState::Rendered);
        sceneRef1->setRenderOrder(1);
        sceneRef2->setRenderOrder(2);
        testScenesAndRenderer.publish(sceneMasterId);
        testScenesAndRenderer.flush(sceneMasterId);

        // get master to ready
        testRenderer.setSceneMapping(sceneMasterId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneMasterId, RendererSceneState::Ready));

        // get ref scenes to ready (requested rendered but blocked by master)
        ReferencedSceneStateEventHandler handler({ { sceneRefId1, RendererSceneState::Ready }, { sceneRefId2, RendererSceneState::Ready } }, testScenesAndRenderer.getScenesRegistry());
        EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(handler));

        // nothing rendered
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        // link data
        masterScene.linkData(sceneRef1, TransformationLinkScene::transformProviderDataId_Left, sceneRef2, TransformationLinkScene::transformConsumerDataId);
        testScenesAndRenderer.flush(sceneMasterId);

        // now all rendered
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneMasterId, RendererSceneState::Rendered));
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
        displayId_t display = createDisplayForWindow();

        // simulate referenced content published
        const sceneId_t sceneRefId1 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER, glm::vec3(0.0f, 0.0f, 5.0f));
        const sceneId_t sceneRefId2 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER, glm::vec3(-1.5f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneRefId1);
        testScenesAndRenderer.publish(sceneRefId2);
        testScenesAndRenderer.flush(sceneRefId1);
        testScenesAndRenderer.flush(sceneRefId2);

        const sceneId_t sceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);
        auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
        auto sceneRef1 = masterScene.createSceneReference(sceneRefId1);
        auto sceneRef2 = masterScene.createSceneReference(sceneRefId2);
        sceneRef1->requestState(RendererSceneState::Ready);
        sceneRef2->requestState(RendererSceneState::Ready);
        sceneRef1->setRenderOrder(1);
        sceneRef2->setRenderOrder(2);
        testScenesAndRenderer.publish(sceneMasterId);
        testScenesAndRenderer.flush(sceneMasterId);

        // get master to rendered so refs can request also rendered
        testRenderer.setSceneMapping(sceneMasterId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneMasterId, RendererSceneState::Rendered));
        testScenesAndRenderer.flush(sceneMasterId);

        // get ref scenes to rendered
        // The sequence of operations is intentionally different from non-threaded version of this test
        // in order to increase chance of detecting any potential race condition by thread sanitizer.
        // Following code interleaves executing sceneref commands in render thread and dispatching events sent back in main thread.
        class EventHandler final : public TestClientEventHandler
        {
        public:
            EventHandler(TestScenesAndRenderer& tester_, sceneId_t sceneMasterId_, std::initializer_list<sceneId_t> refScenes)
                : m_tester(tester_)
                , m_sceneMasterId(sceneMasterId_)
            {
                for (const auto s : refScenes)
                    m_states[s] = RendererSceneState::Available;
            }

            void sceneReferenceStateChanged(SceneReference& sceneRef, RendererSceneState state) override
            {
                if (state == RendererSceneState::Ready)
                    sceneRef.requestState(RendererSceneState::Rendered);
                m_tester.flush(m_sceneMasterId);
                m_states[sceneRef.getReferencedSceneId()] = state;
            }
            bool waitCondition() const override
            {
                return 2u == m_states.size()
                    && std::all_of(m_states.cbegin(), m_states.cend(), [](const auto& it) { return it.second == RendererSceneState::Rendered; });
            }
            void onUpdate() override
            {
                m_tester.flush(m_sceneMasterId);
                for (const auto& s : m_states)
                    m_tester.flush(s.first);
            }

        private:
            TestScenesAndRenderer& m_tester;
            sceneId_t m_sceneMasterId;
            std::unordered_map<sceneId_t, RendererSceneState> m_states;
        } handler(testScenesAndRenderer, sceneMasterId, { sceneRefId1, sceneRefId2 });
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
        void sceneReferenceFlushed(SceneReference& /*unused*/, sceneVersionTag_t versionTag) override
        {
            m_lastReportedVersion = versionTag;
        }
        [[nodiscard]] bool waitCondition() const override
        {
            return m_lastReportedVersion != InvalidSceneVersionTag;
        }
        sceneVersionTag_t m_lastReportedVersion = InvalidSceneVersionTag;
    };

    TEST_F(ARendererLifecycleTest, ReferencedSceneReportsVersionedFlush)
    {
        testScenesAndRenderer.initializeRenderer();
        const displayId_t display = createDisplayForWindow();

        // simulate referenced content published
        const sceneId_t sceneRefId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneRefId);
        testScenesAndRenderer.flush(sceneRefId);

        const sceneId_t sceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);
        auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
        auto sceneRef = masterScene.createSceneReference(sceneRefId);
        sceneRef->requestState(RendererSceneState::Ready);
        testScenesAndRenderer.publish(sceneMasterId);
        testScenesAndRenderer.flush(sceneMasterId);

        // enable receiving of applied versioned flushes
        sceneRef->requestNotificationsForSceneVersionTags(true);
        masterScene.flush();

        // get master to ready
        testRenderer.setSceneMapping(sceneMasterId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneMasterId, RendererSceneState::Ready));

        // get ref scenes to ready
        ReferencedSceneStateEventHandler stateHandler({ { sceneRefId, RendererSceneState::Ready } }, testScenesAndRenderer.getScenesRegistry());
        EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(stateHandler));

        constexpr sceneVersionTag_t version1{ 123u };
        testScenesAndRenderer.flush(sceneRefId, version1);

        // wait for versioned flush reported back to client
        ReferencedSceneFlushEventHandler flushHandler;
        EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(flushHandler));
        EXPECT_EQ(version1, flushHandler.m_lastReportedVersion);

        // another version
        constexpr sceneVersionTag_t version2{ 666u };
        testScenesAndRenderer.flush(sceneRefId, version2);
        flushHandler.m_lastReportedVersion = InvalidSceneVersionTag;
        EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(flushHandler));
        EXPECT_EQ(version2, flushHandler.m_lastReportedVersion);

        testScenesAndRenderer.unpublish(sceneMasterId);
        testScenesAndRenderer.unpublish(sceneRefId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, ReferencedSceneReportsLastAppliedVersionWhenEnablingNotifications)
    {
        testScenesAndRenderer.initializeRenderer();
        const displayId_t display = createDisplayForWindow();

        // simulate referenced content published
        const sceneId_t sceneRefId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.publish(sceneRefId);
        testScenesAndRenderer.flush(sceneRefId);

        const sceneId_t sceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);
        auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
        auto sceneRef = masterScene.createSceneReference(sceneRefId);
        sceneRef->requestState(RendererSceneState::Ready);
        testScenesAndRenderer.publish(sceneMasterId);
        testScenesAndRenderer.flush(sceneMasterId);

        // get master to ready
        testRenderer.setSceneMapping(sceneMasterId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneMasterId, RendererSceneState::Ready));

        // get ref scenes to ready
        ReferencedSceneStateEventHandler stateHandler({ { sceneRefId, RendererSceneState::Ready } }, testScenesAndRenderer.getScenesRegistry());
        EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(stateHandler));

        constexpr sceneVersionTag_t version{ 123u };
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
        EXPECT_EQ(InvalidSceneVersionTag, flushHandler.m_lastReportedVersion);

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
        displayId_t display = createDisplayForWindow();

        const sceneId_t sceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);
        const sceneId_t sceneRefId1 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER, glm::vec3(0.0f, 0.0f, 5.0f));
        const sceneId_t sceneRefId2 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER, glm::vec3(-1.5f, 0.0f, 5.0f));

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
            sceneRef1->requestState(RendererSceneState::Rendered);
            sceneRef2->requestState(RendererSceneState::Rendered);
            sceneRef1->setRenderOrder(1);
            sceneRef2->setRenderOrder(2);
            testScenesAndRenderer.publish(sceneMasterId);
            testScenesAndRenderer.flush(sceneMasterId);

            // get master to ready
            testRenderer.setSceneMapping(sceneMasterId, display);
            ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneMasterId, RendererSceneState::Ready));

            // get ref scenes to ready (requested rendered but blocked by master)
            ReferencedSceneStateEventHandler handler({ { sceneRefId1, RendererSceneState::Ready }, { sceneRefId2, RendererSceneState::Ready } }, testScenesAndRenderer.getScenesRegistry());
            EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(handler));

            // nothing rendered
            ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

            // link data
            masterScene.linkData(sceneRef1, TransformationLinkScene::transformProviderDataId_Left, sceneRef2, TransformationLinkScene::transformConsumerDataId);
            testScenesAndRenderer.flush(sceneMasterId);

            // now all rendered
            ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneMasterId, RendererSceneState::Rendered));
            ASSERT_TRUE(checkScreenshot(display, "ReferencedScenes"));
        }

        // 'unref' scenes by destroying the HL objects
        {
            // first ramp down the scenes' states
            auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
            auto& sceneRef1 = *masterScene.findObject<SceneReference>("REF1");
            auto& sceneRef2 = *masterScene.findObject<SceneReference>("REF2");
            sceneRef1.requestState(RendererSceneState::Available);
            sceneRef2.requestState(RendererSceneState::Available);
            testScenesAndRenderer.flush(sceneMasterId);
            ReferencedSceneStateEventHandler stateHandler({ { sceneRefId1, RendererSceneState::Available }, { sceneRefId2, RendererSceneState::Available } }, testScenesAndRenderer.getScenesRegistry());
            EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(stateHandler));

            masterScene.destroy(sceneRef1);
            masterScene.destroy(sceneRef2);
            testScenesAndRenderer.flush(sceneMasterId);
            ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));
        }

        // create another master (same content as original master)
        const sceneId_t otherSceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);

        // reference existing scenes by new master and get all rendered
        {
            auto& otherMasterScene = testScenesAndRenderer.getScenesRegistry().getScene(otherSceneMasterId);
            auto sceneRef1 = otherMasterScene.createSceneReference(sceneRefId1);
            auto sceneRef2 = otherMasterScene.createSceneReference(sceneRefId2);
            sceneRef1->requestState(RendererSceneState::Rendered);
            sceneRef2->requestState(RendererSceneState::Rendered);
            sceneRef1->setRenderOrder(1);
            sceneRef2->setRenderOrder(2);
            testScenesAndRenderer.publish(otherSceneMasterId);
            testScenesAndRenderer.flush(otherSceneMasterId);

            // get master to ready
            testRenderer.setSceneMapping(otherSceneMasterId, display);
            testScenesAndRenderer.getSceneToState(otherSceneMasterId, RendererSceneState::Ready);

            // get ref scenes to ready (requested rendered but blocked by master)
            ReferencedSceneStateEventHandler handler({ { sceneRefId1, RendererSceneState::Ready }, { sceneRefId2, RendererSceneState::Ready } }, testScenesAndRenderer.getScenesRegistry());
            EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(handler));

            // nothing rendered
            ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

            // link data
            otherMasterScene.linkData(sceneRef1, TransformationLinkScene::transformProviderDataId_Left, sceneRef2, TransformationLinkScene::transformConsumerDataId);
            testScenesAndRenderer.flush(otherSceneMasterId);

            // now all rendered
            testScenesAndRenderer.getSceneToState(otherSceneMasterId, RendererSceneState::Rendered);
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
        displayId_t display = createDisplayForWindow();

        const sceneId_t sceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);
        const sceneId_t sceneRefId1 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER, glm::vec3(0.0f, 0.0f, 5.0f));
        const sceneId_t sceneRefId2 = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_CONSUMER, glm::vec3(-1.5f, 0.0f, 5.0f));

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
            sceneRef1->requestState(RendererSceneState::Rendered);
            sceneRef2->requestState(RendererSceneState::Rendered);
            sceneRef1->setRenderOrder(1);
            sceneRef2->setRenderOrder(2);
            testScenesAndRenderer.publish(sceneMasterId);
            testScenesAndRenderer.flush(sceneMasterId);

            // get master to ready
            testRenderer.setSceneMapping(sceneMasterId, display);
            ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneMasterId, RendererSceneState::Ready));

            // get ref scenes to ready (requested rendered but blocked by master)
            ReferencedSceneStateEventHandler handler({ { sceneRefId1, RendererSceneState::Ready }, { sceneRefId2, RendererSceneState::Ready } }, testScenesAndRenderer.getScenesRegistry());
            EXPECT_TRUE(testScenesAndRenderer.loopTillClientEvent(handler));

            // nothing rendered
            ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

            // link data
            masterScene.linkData(sceneRef1, TransformationLinkScene::transformProviderDataId_Left, sceneRef2, TransformationLinkScene::transformConsumerDataId);
            testScenesAndRenderer.flush(sceneMasterId);

            // now all rendered
            ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneMasterId, RendererSceneState::Rendered));
            ASSERT_TRUE(checkScreenshot(display, "ReferencedScenes"));
        }

        // 'unref' scenes by destroying the HL objects
        {
            // just unref, dont ramp down any of the rendering states
            auto& masterScene = testScenesAndRenderer.getScenesRegistry().getScene(sceneMasterId);
            auto& sceneRef1 = *masterScene.findObject<SceneReference>("REF1");
            auto& sceneRef2 = *masterScene.findObject<SceneReference>("REF2");
            masterScene.destroy(sceneRef1);
            masterScene.destroy(sceneRef2);
            testScenesAndRenderer.flush(sceneMasterId);
            // still rendered
            ASSERT_TRUE(checkScreenshot(display, "ReferencedScenes"));
        }

        // create another master (same content as original master)
        const sceneId_t otherSceneMasterId = createScene<TransformationLinkScene>(TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT);

        // reference existing scenes by new master and get all rendered
        {
            // get new master to rendered first
            testScenesAndRenderer.publish(otherSceneMasterId);
            testScenesAndRenderer.flush(otherSceneMasterId);
            testRenderer.setSceneMapping(otherSceneMasterId, display);
            ASSERT_TRUE(testScenesAndRenderer.getSceneToState(otherSceneMasterId, RendererSceneState::Rendered));

            // reference scenes again, now from new master
            auto& otherMasterScene = testScenesAndRenderer.getScenesRegistry().getScene(otherSceneMasterId);
            auto sceneRef1 = otherMasterScene.createSceneReference(sceneRefId1);
            auto sceneRef2 = otherMasterScene.createSceneReference(sceneRefId2);
            sceneRef1->requestState(RendererSceneState::Rendered);
            sceneRef2->requestState(RendererSceneState::Rendered);
            sceneRef1->setRenderOrder(1);
            sceneRef2->setRenderOrder(2);
            testScenesAndRenderer.flush(otherSceneMasterId);

            // new master is rendered and took over the referenced scenes which were already rendered, so rendered output did not change
            ASSERT_TRUE(checkScreenshot(display, "ReferencedScenes"));

            // now change state of ref scenes while referenced by new master
            sceneRef1->requestState(RendererSceneState::Ready);
            sceneRef2->requestState(RendererSceneState::Ready);
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
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

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

        const displayId_t display1 = createDisplayForWindow(0u, true);
        createDisplayForWindow(1u, true); //nothing gets rendered on it, so it is ALWAYS ready (except right after clearing)

        if (testRenderer.hasSystemCompositorController())
        {
            ASSERT_TRUE(checkScreenshot(display1, "ARendererDisplays_Black"));

            const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));

            testScenesAndRenderer.publish(sceneId);
            testScenesAndRenderer.flush(sceneId);
            testRenderer.setSceneMapping(sceneId, display1);
            ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

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
        const displayId_t display2 = createDisplayForWindow(1u, true);

        if (testRenderer.hasSystemCompositorController())
        {
            ASSERT_TRUE(checkScreenshot(display2, "ARendererDisplays_Black"));

            const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));

            testScenesAndRenderer.publish(sceneId);
            testScenesAndRenderer.flush(sceneId);
            testRenderer.setSceneMapping(sceneId, display2);
            ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

            ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_Three_Triangles"));

            //render again and make sure the display was updated
            testScenesAndRenderer.getScenesRegistry().setSceneState<MultipleTrianglesScene>(sceneId, MultipleTrianglesScene::TRIANGLES_REORDERED);
            testScenesAndRenderer.flush(sceneId);
            //taking the screenshot would timeout if display1 is being starved by the (always ready) other display
            ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_Triangles_reordered"));
        }

        testScenesAndRenderer.destroyRenderer();
    }

// expiration is deprecated feature
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
    TEST_F(ARendererLifecycleTest, SceneNotExpiredWhenUpdatedAndSubscribed)
    {
        testScenesAndRenderer.initializeRenderer();
        const displayId_t display = createDisplayForWindow();

        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Available));

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
        const displayId_t display = createDisplayForWindow();

        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Ready));
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
        const displayId_t display = createDisplayForWindow();

        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);

        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::hours(1));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Ready));
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
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
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
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
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
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        testRenderer.doOneLoop();

        // set expiration of content that will be rendered and eventually will expire
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(300));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        // send flushes within limit but do not render
        testRenderer.setLoopMode(ELoopMode::UpdateOnly);
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
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        testRenderer.doOneLoop();

        // set expiration of content that will be rendered and eventually will expire
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(300));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        // send flushes within limit but do not render
        testRenderer.setLoopMode(ELoopMode::UpdateOnly);
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
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Ready));
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
        struct ExpirationCounter final : public RendererSceneControlEventHandlerEmpty
        {
            void sceneExpired(sceneId_t /*sceneId*/) override
            {
                numExpirationEvents++;
            }
            size_t numExpirationEvents = 0u;
        } expirationCounter;

        testScenesAndRenderer.initializeRenderer();
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
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
        RendererEventHandlerEmpty dummy;
        testRenderer.dispatchEvents(dummy, expirationCounter);
        ASSERT_TRUE(expirationCounter.numExpirationEvents == 0u);

        // now hide scene
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Ready));

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
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        testRenderer.doOneLoop();

        // set expiration of content that will be rendered and eventually will expire
        testScenesAndRenderer.setExpirationTimestamp(sceneId, FlushTime::Clock::now() + std::chrono::milliseconds(300));
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();

        // send flushes within limit but do not render
        testRenderer.setLoopMode(ELoopMode::UpdateOnly);
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
        testRenderer.setLoopMode(ELoopMode::UpdateAndRender);
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
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const sceneId_t sceneId1 = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(-0.50f, 1.0f, 5.0f));
        const sceneId_t sceneId2 = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(-0.50f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId1);
        testScenesAndRenderer.publish(sceneId2);
        testScenesAndRenderer.flush(sceneId1);
        testScenesAndRenderer.flush(sceneId2);
        testRenderer.setSceneMapping(sceneId1, display);
        testRenderer.setSceneMapping(sceneId2, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId1, RendererSceneState::Rendered));
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId2, RendererSceneState::Rendered));
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
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

    TEST_F(ARendererLifecycleTest, HandlesSwitchingTriStateVisibilityFullyOffAndOn)
    {
        testScenesAndRenderer.initializeRenderer();
        const auto display = createDisplayForWindow();
        ASSERT_TRUE(displayId_t::Invalid() != display);

        const auto sceneId = createScene<VisibilityScene>(VisibilityScene::VISIBILITY_ALL_OFF, glm::vec3(0.f, 1.0f, 5.0f));
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_OFF_AND_INVISIBLE);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.doOneLoop();
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_OFF_AND_VISIBLE);
        testScenesAndRenderer.flush(sceneId, sceneVersionTag_t{1u});
        testRenderer.waitForFlush(sceneId, sceneVersionTag_t{1u});
        testRenderer.doOneLoop();
        ASSERT_TRUE(checkScreenshot(display, "VisibilityScene_Partial"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_ALL_VISIBLE);
        testScenesAndRenderer.flush(sceneId, sceneVersionTag_t{2u});
        testRenderer.waitForFlush(sceneId, sceneVersionTag_t{2u});
        ASSERT_TRUE(checkScreenshot(display, "VisibilityScene_On"));

        // another cycle of OFF to ON
        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_ALL_OFF);
        testScenesAndRenderer.flush(sceneId, sceneVersionTag_t{3u});
        testRenderer.waitForFlush(sceneId, sceneVersionTag_t{3u});
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_OFF_AND_INVISIBLE);
        testScenesAndRenderer.flush(sceneId, sceneVersionTag_t{4u});
        testRenderer.waitForFlush(sceneId, sceneVersionTag_t{4u});
        ASSERT_TRUE(checkScreenshot(display, "ARendererDisplays_Black"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_OFF_AND_VISIBLE);
        testScenesAndRenderer.flush(sceneId, sceneVersionTag_t{5u});
        testRenderer.waitForFlush(sceneId, sceneVersionTag_t{5u});
        ASSERT_TRUE(checkScreenshot(display, "VisibilityScene_Partial"));

        testScenesAndRenderer.getScenesRegistry().setSceneState<VisibilityScene>(sceneId, VisibilityScene::VISIBILITY_ALL_VISIBLE);
        testScenesAndRenderer.flush(sceneId, sceneVersionTag_t{6u});
        testRenderer.waitForFlush(sceneId, sceneVersionTag_t{6u});
        ASSERT_TRUE(checkScreenshot(display, "VisibilityScene_On"));

        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, canDestroyAndRecreateRendererOnSameFramework)
    {
        const sceneId_t sceneId = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const displayId_t display_1 = createDisplayForWindow();
        ASSERT_TRUE(display_1 != displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display_1);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display_1, "ARendererInstance_Three_Triangles"));

        // destroy and recreate renderer
        testScenesAndRenderer.destroyRenderer();
        testScenesAndRenderer.initializeRenderer();

        const displayId_t display_2 = createDisplayForWindow();
        ASSERT_TRUE(display_2 != displayId_t::Invalid());

        testRenderer.setSceneMapping(sceneId, display_2);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display_2, "ARendererInstance_Three_Triangles"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, VertexArrayGetsUploadedWhenRenderableResourcesUploaded)
    {
        // Test dirtiness of VAO is handled correctly in case renderable stays dirty after geometry
        // resources are ready, e.g., in case VAO upload is blocked till other renderable resources
        // get uploaded
        const sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<TextureSamplerScene>(TextureSamplerScene::EState_NoTextureSampler, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();
        const displayId_t display = createDisplayForWindow();
        ASSERT_TRUE(display != displayId_t::Invalid());

        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));

        testScenesAndRenderer.getScenesRegistry().setSceneState<TextureSamplerScene>(sceneId, TextureSamplerScene::EState_SetTextureSampler);
        testScenesAndRenderer.flush(sceneId, sceneVersionTag_t{ 1u });
        testRenderer.waitForFlush(sceneId, sceneVersionTag_t{ 1u });

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, confidenceTest_contextEnabledWhenCreatingAndDestroyingDisplaysOBsInParticularOrder)
    {
        testScenesAndRenderer.initializeRenderer();
        // create 2 displays, each with OB
        const auto display1 = createDisplayForWindow(1u);
        const auto ob1 = testRenderer.createOffscreenBuffer(display1, 16u, 16u, false);
        const auto display2 = createDisplayForWindow(2u);
        const auto ob2 = testRenderer.createOffscreenBuffer(display2, 16u, 16u, false);
        ASSERT_TRUE(display1.isValid() && display2.isValid() && ob1.isValid() && ob2.isValid());
        testRenderer.doOneLoop();

        // create another OB on 1st display
        const auto ob3 = testRenderer.createOffscreenBuffer(display1, 16u, 16u, false);
        ASSERT_TRUE(ob3.isValid());
        testRenderer.doOneLoop();

        // destroy 2nd display/OB
        testRenderer.destroyOffscreenBuffer(display2, ob2);
        testRenderer.destroyDisplay(display2);
        testRenderer.doOneLoop();

        // create another OB on 1st display
        // this requires context enabled on 1st display and would crash otherwise
        const auto ob4 = testRenderer.createOffscreenBuffer(display1, 16u, 16u, false);
        ASSERT_TRUE(ob4.isValid());
        testRenderer.doOneLoop();

        testScenesAndRenderer.destroyRenderer();
    }

#ifdef ramses_sdk_ENABLE_WINDOW_TYPE_WINDOWS
    TEST_F(ARendererLifecycleTest, CanCreateMultipleDisplaysWithDifferentDeviceTypes_GLES30_GL42)
    {
        const sceneId_t sceneId1 = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        const sceneId_t sceneId2 = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::TRIANGLES_REORDERED, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();

        ramses::DisplayConfig displayConfig1;
        displayConfig1.setDeviceType(EDeviceType::GLES_3_0);
        displayConfig1.setWindowRectangle(WindowX, WindowY, WindowWidth, WindowHeight);
        ramses::DisplayConfig displayConfig2;
        displayConfig2.setDeviceType(EDeviceType::GL_4_2);
        displayConfig2.setWindowRectangle(WindowX + WindowWidth, WindowY, WindowWidth, WindowHeight);

        const displayId_t display1 = testRenderer.createDisplay(displayConfig1);
        const displayId_t display2 = testRenderer.createDisplay(displayConfig2);
        ASSERT_TRUE(displayId_t::Invalid() != display1);
        ASSERT_TRUE(displayId_t::Invalid() != display2);

        testScenesAndRenderer.publish(sceneId1);
        testScenesAndRenderer.publish(sceneId2);
        testScenesAndRenderer.flush(sceneId1);
        testScenesAndRenderer.flush(sceneId2);
        testRenderer.setSceneMapping(sceneId1, display1);
        testRenderer.setSceneMapping(sceneId2, display2);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId1, RendererSceneState::Rendered));
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId2, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display1, "ARendererInstance_Three_Triangles"));
        ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_Triangles_reordered"));

        testScenesAndRenderer.unpublish(sceneId1);
        testScenesAndRenderer.unpublish(sceneId2);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, CanCreateMultipleDisplaysWithDifferentDeviceTypes_GLES_GL42_GL45)
    {
        const sceneId_t sceneId1 = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        const sceneId_t sceneId2 = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::TRIANGLES_REORDERED, glm::vec3(0.0f, 0.0f, 5.0f));
        const sceneId_t sceneId3 = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::TRIANGLES_REORDERED, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();

        ramses::DisplayConfig displayConfig1;
        displayConfig1.setDeviceType(EDeviceType::GLES_3_0);
        displayConfig1.setWindowRectangle(WindowX, WindowY, WindowWidth, WindowHeight);
        ramses::DisplayConfig displayConfig2;
        displayConfig2.setDeviceType(EDeviceType::GL_4_2);
        displayConfig2.setWindowRectangle(WindowX + WindowWidth, WindowY, WindowWidth, WindowHeight);
        ramses::DisplayConfig displayConfig3;
        displayConfig3.setDeviceType(EDeviceType::GL_4_5);
        displayConfig3.setWindowRectangle(WindowX, WindowY + WindowHeight, WindowWidth, WindowHeight);

        const displayId_t display1 = testRenderer.createDisplay(displayConfig1);
        const displayId_t display2 = testRenderer.createDisplay(displayConfig2);
        const displayId_t display3 = testRenderer.createDisplay(displayConfig3);
        ASSERT_TRUE(displayId_t::Invalid() != display1);
        ASSERT_TRUE(displayId_t::Invalid() != display2);
        ASSERT_TRUE(displayId_t::Invalid() != display3);

        testScenesAndRenderer.publish(sceneId1);
        testScenesAndRenderer.publish(sceneId2);
        testScenesAndRenderer.publish(sceneId3);
        testScenesAndRenderer.flush(sceneId1);
        testScenesAndRenderer.flush(sceneId2);
        testScenesAndRenderer.flush(sceneId3);
        testRenderer.setSceneMapping(sceneId1, display1);
        testRenderer.setSceneMapping(sceneId2, display2);
        testRenderer.setSceneMapping(sceneId3, display3);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId1, RendererSceneState::Rendered));
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId2, RendererSceneState::Rendered));
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId3, RendererSceneState::Rendered));

        ASSERT_TRUE(checkScreenshot(display1, "ARendererInstance_Three_Triangles"));
        ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_Triangles_reordered"));
        ASSERT_TRUE(checkScreenshot(display3, "ARendererInstance_Triangles_reordered"));

        testScenesAndRenderer.unpublish(sceneId1);
        testScenesAndRenderer.unpublish(sceneId2);
        testScenesAndRenderer.unpublish(sceneId3);
        testScenesAndRenderer.destroyRenderer();
    }
#endif

    TEST_F(ARendererLifecycleTest, CanCreateMultipleDisplaysWithDifferentWindowTypes)
    {
        const sceneId_t sceneId1 = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::THREE_TRIANGLES, glm::vec3(0.0f, 0.0f, 5.0f));
        const sceneId_t sceneId2 = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::TRIANGLES_REORDERED, glm::vec3(0.0f, 0.0f, 5.0f));
        const sceneId_t sceneId3 = createScene<MultipleTrianglesScene>(MultipleTrianglesScene::TRIANGLES_REORDERED, glm::vec3(0.0f, 0.0f, 5.0f));
        testScenesAndRenderer.initializeRenderer();

        testScenesAndRenderer.publish(sceneId1);
        testScenesAndRenderer.publish(sceneId2);
        testScenesAndRenderer.publish(sceneId3);
        testScenesAndRenderer.flush(sceneId1);
        testScenesAndRenderer.flush(sceneId2);
        testScenesAndRenderer.flush(sceneId3);

        displayId_t display1;
        displayId_t display2;
        displayId_t display3;

#ifdef ramses_sdk_ENABLE_WINDOW_TYPE_X11
        ramses::DisplayConfig displayConfig1;
        displayConfig1.setWindowType(EWindowType::X11);
        displayConfig1.setWindowRectangle(WindowX, WindowY, WindowWidth, WindowHeight);
        display1 = testRenderer.createDisplay(displayConfig1);
        ASSERT_TRUE(displayId_t::Invalid() != display1);
        testRenderer.setSceneMapping(sceneId1, display1);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId1, RendererSceneState::Rendered));
#endif

#ifdef ramses_sdk_ENABLE_WINDOW_TYPE_WAYLAND_WL_SHELL
        ramses::DisplayConfig displayConfig2;
        displayConfig2.setWindowType(EWindowType::Wayland_Shell);
        displayConfig2.setWindowRectangle(WindowX + WindowWidth, WindowY, WindowWidth, WindowHeight);
        display2 = testRenderer.createDisplay(displayConfig2);
        ASSERT_TRUE(displayId_t::Invalid() != display2);
        testRenderer.setSceneMapping(sceneId2, display2);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId2, RendererSceneState::Rendered));
#endif

#ifdef ramses_sdk_ENABLE_WINDOW_TYPE_WAYLAND_IVI
        ramses::DisplayConfig displayConfig3 = RendererTestUtils::CreateTestDisplayConfig(0u, true);
        // Dont create wayland ivi window, if the test is "originally" running for wayland shell
        // since the compositor used for that case does not support ivi_controller
        if(displayConfig3.getWindowType() != EWindowType::Wayland_Shell)
        {
            displayConfig3.setWindowType(EWindowType::Wayland_IVI);
            displayConfig3.setWindowRectangle(WindowX, WindowY + WindowHeight, WindowWidth, WindowHeight);
            display3 = testRenderer.createDisplay(displayConfig3);
            ASSERT_TRUE(displayId_t::Invalid() != display3);
            testRenderer.setSceneMapping(sceneId3, display3);
            ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId3, RendererSceneState::Rendered));
        }
#endif

        if(display1.isValid())
        {
            ASSERT_TRUE(checkScreenshot(display1, "ARendererInstance_Three_Triangles"));
        }
        if(display2.isValid())
        {
            ASSERT_TRUE(checkScreenshot(display2, "ARendererInstance_Triangles_reordered"));
        }
        if(display3.isValid())
        {
            ASSERT_TRUE(checkScreenshot(display3, "ARendererInstance_Triangles_reordered"));
        }

        testScenesAndRenderer.unpublish(sceneId1);
        testScenesAndRenderer.unpublish(sceneId2);
        testScenesAndRenderer.unpublish(sceneId3);
        testScenesAndRenderer.destroyRenderer();
    }

    TEST_F(ARendererLifecycleTest, ChangeResolutionOfRenderTargetUsingLogic)
    {
        const sceneId_t sceneId = createScene<RenderTargetScene>(RenderTargetScene::RENDERBUFFER_LOGIC_LOWRES);
        testScenesAndRenderer.initializeRenderer();
        displayId_t display = createDisplayForWindow();

        // render initial low res
        testScenesAndRenderer.publish(sceneId);
        testScenesAndRenderer.flush(sceneId);
        testRenderer.setSceneMapping(sceneId, display);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARenderTargetSizeChange_Low"));

        // unmap scene, change resolution of render target and render again
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Available));
        testScenesAndRenderer.getScenesRegistry().setSceneState<RenderTargetScene>(sceneId, RenderTargetScene::RENDERBUFFER_LOGIC_HIGHRES);
        testScenesAndRenderer.flush(sceneId);
        ASSERT_TRUE(testScenesAndRenderer.getSceneToState(sceneId, RendererSceneState::Rendered));
        ASSERT_TRUE(checkScreenshot(display, "ARenderTargetSizeChange_High"));

        // change back to lowres while scene rendered -> such change is not valid for already uploaded render buffer but it will change viewport
        testScenesAndRenderer.getScenesRegistry().setSceneState<RenderTargetScene>(sceneId, RenderTargetScene::RENDERBUFFER_LOGIC_LOWRES);
        testScenesAndRenderer.flush(sceneId);
        ASSERT_TRUE(checkScreenshot(display, "ARenderTargetSizeChange_Invalid"));

        testScenesAndRenderer.unpublish(sceneId);
        testScenesAndRenderer.destroyRenderer();
    }

    static const std::string_view SceneFileName = "multiInstanceTestScene.ramses";
    enum class RenderingMode { DoOneLoop, Threaded };
    class ARendererLifecycleTest_MultipleInstances : public ::testing::TestWithParam<RenderingMode>
    {
    protected:
        static void SetUpTestSuite()
        {
            // first create some scene and save it to file
            TestScenesAndRenderer testScenesAndRenderer{ ramses::RamsesFrameworkConfig{ EFeatureLevel_Latest } };
            const sceneId_t sceneId = testScenesAndRenderer.getScenesRegistry().createScene<LogicScene>(LogicScene::TRIANGLE_LOGIC,
                glm::vec3(0.0f, 0.0f, 5.0f), ARendererLifecycleTest::WindowWidth, ARendererLifecycleTest::WindowHeight);

            auto& scene = testScenesAndRenderer.getScenesRegistry().getScene(sceneId);
            ASSERT_TRUE(scene.saveToFile(SceneFileName));
        }

        static void TearDownTestSuite()
        {
            File file{ SceneFileName };
            if (file.exists())
                file.remove();
        }
    };

    class ARendererLifecycleTest_TestInstance
    {
    public:
        ARendererLifecycleTest_TestInstance(uint32_t instanceIdx, RenderingMode renderMode)
            : m_instanceIdx{ instanceIdx }
            , m_renderMode { renderMode }
        {
        }

        void createFrameworkAndLoadScene()
        {
            ramses::RamsesFrameworkConfig cfg{ EFeatureLevel_Latest };
            cfg.setLoggingInstanceName(fmt::format("testInstance{}", m_instanceIdx));
            m_framework = std::make_unique<ramses::RamsesFramework>(cfg);
            auto client = m_framework->createClient(fmt::format("testClient{}", m_instanceIdx));
            ASSERT_TRUE(client);
            m_scene = client->loadSceneFromFile(SceneFileName);
            ASSERT_TRUE(m_scene);
            m_scene->publish();
        }

        void createRendererAndDisplay()
        {
            m_renderer = std::make_unique<TestRenderer>();
            m_renderer->initializeRendererWithFramework(*m_framework, RendererTestUtils::CreateTestRendererConfig());
            if (m_renderMode == RenderingMode::Threaded)
                m_renderer->startRendererThread();

            auto displayConfig = RendererTestUtils::CreateTestDisplayConfig(m_instanceIdx, true);
            displayConfig.setWindowRectangle(ARendererLifecycleTest::WindowX, ARendererLifecycleTest::WindowY, ARendererLifecycleTest::WindowWidth, ARendererLifecycleTest::WindowHeight);
            m_display = m_renderer->createDisplay(displayConfig);
            ASSERT_TRUE(m_display != displayId_t::Invalid());

            m_renderer->setSceneMapping(m_scene->getSceneId(), m_display);
            ASSERT_TRUE(m_renderer->getSceneToState(*m_scene, RendererSceneState::Rendered));
        }

        void updateAndRenderFewFrames()
        {
            auto le = m_scene->findObject<ramses::LogicEngine>("le");
            ASSERT_TRUE(le);
            auto script = le->findObject<ramses::LuaScript>("script");
            ASSERT_TRUE(script);

            // update logic and render few frames with some changing values ending up at the correct one (one)
            for (int x = -4; x <= 1; ++x)
            {
                EXPECT_TRUE(script->getInputs()->getChild("translation_x")->set(float(x)));
                EXPECT_TRUE(le->update());
                EXPECT_TRUE(m_scene->flush());

                if (m_renderMode == RenderingMode::Threaded)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds{ 10 });
                }
                else
                {
                    m_renderer->doOneLoop();
                }
            }
        }

        void checkScreenshot()
        {
            EXPECT_TRUE(m_renderer->performScreenshotCheck(m_display, {}, 0u, 0u, ARendererLifecycleTest::WindowWidth, ARendererLifecycleTest::WindowHeight, "AMultipleInstances_TriangleWithLogic"));
        }

    private:
        uint32_t m_instanceIdx = 0u;
        RenderingMode m_renderMode = RenderingMode::DoOneLoop;
        std::unique_ptr<ramses::RamsesFramework> m_framework;
        ramses::Scene* m_scene = nullptr;
        std::unique_ptr<TestRenderer> m_renderer;
        ramses::displayId_t m_display;
    };

    INSTANTIATE_TEST_SUITE_P(
        ARendererLifecycleTest_MultipleInstances_TestInstances,
        ARendererLifecycleTest_MultipleInstances,
        ::testing::Values(
            RenderingMode::DoOneLoop,
            RenderingMode::Threaded)
    );

    TEST_P(ARendererLifecycleTest_MultipleInstances, MultipleRamsesInstancesInSequence)
    {
        for (uint32_t i = 0u; i < 3u; ++i)
        {
            ARendererLifecycleTest_TestInstance testInstance{ i, GetParam() };
            testInstance.createFrameworkAndLoadScene();
            testInstance.createRendererAndDisplay();
            testInstance.updateAndRenderFewFrames();
            testInstance.checkScreenshot();
        }
    }

    TEST_P(ARendererLifecycleTest_MultipleInstances, MultipleRamsesInstancesInParallel)
    {
        static constexpr uint32_t NumInstances = 3u;
        std::vector<std::thread> threads;
        threads.reserve(NumInstances);
        for (uint32_t i = 0u; i < NumInstances; ++i)
        {
            threads.emplace_back([i] {
                ARendererLifecycleTest_TestInstance testInstance{ i, GetParam() };
                testInstance.createFrameworkAndLoadScene();
                testInstance.createRendererAndDisplay();
                testInstance.updateAndRenderFewFrames();
                testInstance.checkScreenshot();
                });
        }

        for (auto& t : threads)
            t.join();
    }

    TEST_P(ARendererLifecycleTest_MultipleInstances, MultipleRamsesInstancesInParallel_WithBarriers)
    {
        static constexpr uint32_t NumInstances = 3u;
        ThreadBarrier loadBarrier{ NumInstances };
        ThreadBarrier initRendererBarrier{ NumInstances };
        ThreadBarrier loopBarrier{ NumInstances };
        ThreadBarrier checkScreenshortBarrier{ NumInstances };

        std::vector<std::thread> threads;
        threads.reserve(NumInstances);
        for (uint32_t i = 0u; i < NumInstances; ++i)
        {
            threads.emplace_back([&, i] {
                ARendererLifecycleTest_TestInstance testInstance{ i, GetParam() };

                loadBarrier.wait();
                testInstance.createFrameworkAndLoadScene();

                initRendererBarrier.wait();
                testInstance.createRendererAndDisplay();

                loopBarrier.wait();
                testInstance.updateAndRenderFewFrames();

                checkScreenshortBarrier.wait();
                testInstance.checkScreenshot();
                });
        }

        for (auto& t : threads)
            t.join();
    }
}
