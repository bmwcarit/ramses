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
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "ramses-framework-api/RamsesFramework.h"

#include "TestScenes/TransformationLinkScene.h"
#include "TestScenes/FileLoadingScene.h"
#include "RamsesFrameworkImpl.h"
#include "TestStepCommand.h"
#include "Math3d/Vector3.h"
#include "RendererMate.h"
#include "Ramsh/Ramsh.h"
#include "CLI/CLI.hpp"

int main(int argc, const char* argv[])
{
    CLI::App cli;
    ramses::RamsesFrameworkConfig frameworkConfig;
    frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
    ramses::RendererConfig rendererConfig;
    ramses::DisplayConfig  displayConfig;
    uint32_t testNr = 1u;
    bool     disableAutoMapping = false;

    try
    {
        cli.add_option("--tn,--test-nr", testNr);
        cli.add_flag("--no-auto-show", disableAutoMapping);
        frameworkConfig.registerOptions(cli);
        rendererConfig.registerOptions(cli);
        displayConfig.registerOptions(cli);
    }
    catch (const CLI::Error& error)
    {
        std::cerr << error.what();
        return -1;
    }
    CLI11_PARSE(cli, argc, argv);


    //Ramses client
    ramses::RamsesFramework framework(frameworkConfig);
    ramses::RamsesClient& client(*framework.createClient("ramses-local-client-test"));
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    framework.connect();

    if (testNr == 4)
    {
        // Set background color to blue, to distinguish an possible rendered empty frame from the black background
        // of the integration test "test_run_no_initial_black_frame.py".
        displayConfig.setClearColor(0.0, 0.0, 1.0, 1.0);
    }
    renderer.createDisplay(displayConfig);
    renderer.flush();

    int x = 0;
    uint32_t displayWidth = 0;
    uint32_t displayHeight = 0;
    displayConfig.getWindowRectangle(x, x, displayWidth, displayHeight);

    ramses::RendererMate rendererMate(renderer.impl, framework.impl);
    ramses::RendererMateAutoShowHandler dmEventHandler(rendererMate, !disableAutoMapping);

    if (testNr == 1 || testNr == 2)
    {
        // host scene contains provider nodes
        const ramses::sceneId_t hostSceneId(12u);
        ramses::Scene* hostScene = client.createScene(hostSceneId);
        ramses_internal::TransformationLinkScene transformationProviderScene(*hostScene, ramses_internal::TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT, { 0.f, 0.f, 0.f }, displayWidth, displayHeight);
        hostScene->flush();
        hostScene->publish(ramses::EScenePublicationMode_LocalOnly);

        //client scene
        const ramses::sceneId_t localSceneId(42u);
        ramses::Scene* clientScene = client.createScene(localSceneId);
        ramses_internal::TransformationLinkScene redTriangleScene(*clientScene, ramses_internal::TransformationLinkScene::TRANSFORMATION_CONSUMER, { 0.f, 0.f, 12.f }, displayWidth, displayHeight);
        clientScene->flush();
        clientScene->publish(ramses::EScenePublicationMode_LocalOnly);

        const ramses::sceneId_t remoteSceneId(67u);

        if (testNr == 1)
        {
            // 'sender' has two scenes, one locally shown, and one distributed remotely
            printf("sender\n");

            // scene to distribute only
            ramses::Scene* remoteScene = client.createScene(remoteSceneId);
            ramses_internal::TransformationLinkScene blueTriangleScene(*remoteScene, ramses_internal::TransformationLinkScene::TRANSFORMATION_CONSUMER_OVERRIDEN, { 0.f, 0.f, 12.f }, displayWidth, displayHeight);
            remoteScene->flush();
            remoteScene->publish();
        }
        else //if (testNrArgument == 2)
        {
            // 'receiver': has one local scene, and maps a remote scene additionally
            printf("receiver mode\n");
        }

        while (rendererMate.isRunning())
        {
            rendererMate.dispatchAndFlush(dmEventHandler);
            renderer.doOneLoop();
            ramses_internal::PlatformThread::Sleep(16);
        }
    }
    else
    {
        switch (testNr)
        {
            case 3:
            {
                ramses::sceneId_t sceneId(13u);
                ramses_internal::FileLoadingScene fileLoadingScene(client, ramses_internal::FileLoadingScene::CREATE_SAVE_DESTROY_LOAD_USING_SAME_CLIENT, sceneId, { 0.f, 0.f, 5.f }, ".", frameworkConfig, displayWidth, displayHeight);
                ramses::Scene* scene = fileLoadingScene.getCreatedScene();
                scene->publish();

                while (rendererMate.getLastReportedSceneState(sceneId) != ramses::RendererSceneState::Rendered)
                {
                    renderer.doOneLoop();
                    rendererMate.dispatchAndFlush(dmEventHandler);
                    ramses_internal::PlatformThread::Sleep(16);
                }
            }
            break;
            case 4:
            {
                renderer.setLoopMode(ramses::ELoopMode_UpdateOnly);
                auto testStepCommand = std::make_shared<ramses_internal::TestStepCommand>();
                framework.impl.getRamsh().add(testStepCommand);

                const ramses::sceneId_t sceneId(1u);
                ramses::Scene& clientScene = *client.createScene(sceneId);
                ramses_internal::TransformationLinkScene redTriangleScene(clientScene, ramses_internal::TransformationLinkScene::TRANSFORMATION_CONSUMER, { 0.f, 0.f, 4.f }, displayWidth, displayHeight);

                clientScene.publish();
                clientScene.flush();
                while (rendererMate.getLastReportedSceneState(sceneId) != ramses::RendererSceneState::Rendered)
                {
                    renderer.doOneLoop();
                    rendererMate.dispatchAndFlush(dmEventHandler);
                    ramses_internal::PlatformThread::Sleep(16);
                }
                LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Surface should be still invisible");
                // The integration test "test_run_no_initial_black_frame.py" checks here, if the surface is not yet
                // visible on the screen by doing a screenshot from the system compositor. It then sends "step 1" to
                // proceed with the test.
                while (testStepCommand->getCurrentTestStep() != 1)
                {
                    renderer.doOneLoop();
                    ramses_internal::PlatformThread::Sleep(50);
                }

                // Do only render a single frame, this makes the surface to become visible.
                // Before it was invisible, because it had no content.
                renderer.setLoopMode(ramses::ELoopMode_UpdateAndRender);
                renderer.doOneLoop();
                renderer.setLoopMode(ramses::ELoopMode_UpdateOnly);

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
