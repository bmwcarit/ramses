//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#include "ramses-client-api/RamsesClient.h"
#include "ramses-client-api/Scene.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-framework-api/RamsesFramework.h"

#include "TestScenes/TransformationLinkScene.h"
#include "TestScenes/FileLoadingScene.h"
#include "RamsesFrameworkImpl.h"
#include "TestStepCommand.h"
#include "Utils/Argument.h"
#include "Math3d/Vector3.h"
#include "DisplayManager/DisplayManager.h"

int main(int argc, const char* argv[])
{
    ramses_internal::CommandLineParser parser(argc, argv);
    ramses_internal::ArgumentUInt32 testNrArgument(parser, "tn", "test-nr", 1u);
    ramses_internal::ArgumentBool disableAutoMapping(parser, "nomap", "disableAutoMapping", false);

    //Ramses client
    ramses::RamsesFrameworkConfig frameworkConfig(argc, argv);
    frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
    ramses::RamsesFramework framework(frameworkConfig);

    ramses::RamsesClient client("ramses-local-client-test", framework);

    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer renderer(framework, rendererConfig);
    ramses_display_manager::DisplayManager displayManager(renderer, framework, !disableAutoMapping.wasDefined());

    framework.connect();

    ramses::DisplayConfig displayConfig(argc, argv);

    if (testNrArgument == 4)
    {
        // Set background color to blue, to distinguish an possible rendered empty frame from the black background
        // of the integration test "test_run_no_initial_black_frame.py".
        displayConfig.setClearColor(0.0, 0.0, 1.0, 1.0);
    }
    renderer.createDisplay(displayConfig);

    if (testNrArgument == 1 || testNrArgument == 2)
    {
        // host scene contains provider nodes
        const ramses::sceneId_t hostSceneId = 12u;
        ramses::Scene* hostScene = client.createScene(hostSceneId);
        ramses_internal::TransformationLinkScene transformationProviderScene(client, *hostScene, ramses_internal::TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT, ramses_internal::Vector3(0.0f));
        hostScene->flush();
        hostScene->publish(ramses::EScenePublicationMode_LocalOnly);

        //client scene
        const ramses::sceneId_t localSceneId = 42u;
        ramses::Scene* clientScene = client.createScene(localSceneId);
        ramses_internal::TransformationLinkScene redTriangleScene(client, *clientScene, ramses_internal::TransformationLinkScene::TRANSFORMATION_CONSUMER, ramses_internal::Vector3(0.0f, 0.0f, 12.0f));
        clientScene->flush();
        clientScene->publish(ramses::EScenePublicationMode_LocalOnly);

        const ramses::sceneId_t remoteSceneId = 67u;

        if (testNrArgument == 1)
        {
            // 'sender' has two scenes, one locally shown, and one distributed remotely
            printf("sender\n");

            // scene to distribute only
            ramses::Scene* remoteScene = client.createScene(remoteSceneId);
            ramses_internal::TransformationLinkScene blueTriangleScene(client, *remoteScene, ramses_internal::TransformationLinkScene::TRANSFORMATION_CONSUMER_OVERRIDEN, ramses_internal::Vector3(0.0f, 0.0f, 12.0f));
            remoteScene->flush();
            remoteScene->publish();
        }
        else //if (testNrArgument == 2)
        {
            // 'receiver': has one local scene, and maps a remote scene additionally
            printf("receiver mode\n");
        }

        while (displayManager.isRunning())
        {
            displayManager.dispatchAndFlush();
            renderer.doOneLoop();
            ramses_internal::PlatformThread::Sleep(16);
        }
    }
    else
    {
        switch (testNrArgument)
        {
            case 3:
            {
                ramses::sceneId_t sceneId(13u);
                ramses_internal::FileLoadingScene fileLoadingScene(client, ramses_internal::FileLoadingScene::CREATE_SAVE_DESTROY_LOAD_USING_SAME_CLIENT, sceneId, ramses_internal::Vector3(0.0, 0.0, 5.0), ".", ramses::RamsesFrameworkConfig(argc, argv));
                ramses::Scene* scene = fileLoadingScene.getCreatedScene();
                scene->publish();

                while (displayManager.getLastReportedSceneState(sceneId) != ramses_display_manager::SceneState::Rendered)
                {
                    renderer.doOneLoop();
                    displayManager.dispatchAndFlush();
                    ramses_internal::PlatformThread::Sleep(16);
                }
            }
            break;
            case 4:
            {
                renderer.setLoopMode(ramses::ELoopMode_UpdateOnly);
                ramses_internal::TestStepCommand testStepCommand;
                framework.impl.getRamsh().add(testStepCommand);

                const ramses::sceneId_t sceneId     = 1u;
                ramses::Scene&          clientScene = *client.createScene(sceneId);

                ramses_internal::TransformationLinkScene redTriangleScene(
                    client,
                    clientScene,
                    ramses_internal::TransformationLinkScene::TRANSFORMATION_CONSUMER,
                    ramses_internal::Vector3(0.0f, 0.0f, 4.0f));

                clientScene.publish();
                clientScene.flush();
                while (displayManager.getLastReportedSceneState(sceneId) != ramses_display_manager::SceneState::Rendered)
                {
                    renderer.doOneLoop();
                    displayManager.dispatchAndFlush();
                    ramses_internal::PlatformThread::Sleep(16);
                }
                LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Surface should be still invisible");
                // The integration test "test_run_no_initial_black_frame.py" checks here, if the surface is not yet
                // visible on the screen by doing a screenshot from the system compositor. It then sends "step 1" to
                // proceed with the test.
                while (testStepCommand.getCurrentTestStep() != 1)
                {
                    renderer.doOneLoop();
                    ramses_internal::PlatformThread::Sleep(50);
                }

                // Do only render a single frame, this makes the surface to become visible.
                // Before it was invisible, because it had no content.
                renderer.setLoopMode(ramses::ELoopMode_UpdateAndRender);
                renderer.doOneLoop();
                renderer.setLoopMode(ramses::ELoopMode_UpdateOnly);

                LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Surface with scene should now be visible");
                // The integration test checks here, if the surface with the scene is now visible on the screen
                // by doing a screenshot from the system compositor.

                // Don't render a frame anymore. Important for the test to really prove, that the scene
                // was rendered before the surface becomes visible the first time.
                while (true)
                {
                    renderer.doOneLoop();
                    ramses_internal::PlatformThread::Sleep(50);
                }
            }
            break;
        }
    }
}
