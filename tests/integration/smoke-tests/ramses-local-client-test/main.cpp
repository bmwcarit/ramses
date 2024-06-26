//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#include "ramses/client/RamsesClient.h"
#include "ramses/client/Scene.h"
#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "ramses/renderer/RendererSceneControl.h"
#include "ramses/framework/RamsesFramework.h"

#include "TestScenes/TransformationLinkScene.h"
#include "TestScenes/FileLoadingScene.h"
#include "TestScenes/StreamTextureScene.h"
#include "impl/RamsesFrameworkImpl.h"
#include "TestStepCommand.h"
#include "impl/RendererMate.h"
#include "internal/Ramsh/Ramsh.h"
#include "ramses-cli.h"

namespace
{
    void runTranformationLinkingTest(ramses::RamsesClient& client,
                                      ramses::RamsesRenderer& renderer,
                                      ramses::internal::RendererMate& rendererMate,
                                      ramses::internal::RendererMateAutoShowHandler& dmEventHandler,
                                      uint32_t displayWidth,
                                      uint32_t displayHeight,
                                      uint32_t testNr)
    {
        // host scene contains provider nodes
        const ramses::sceneId_t hostSceneId(12u);
        const ramses::SceneConfig config{hostSceneId, ramses::EScenePublicationMode::LocalAndRemote};
        ramses::Scene* hostScene = client.createScene(config);
        ramses::internal::TransformationLinkScene transformationProviderScene(*hostScene, ramses::internal::TransformationLinkScene::TRANSFORMATION_PROVIDER_WITHOUT_CONTENT, { 0.f, 0.f, 0.f }, displayWidth, displayHeight);
        hostScene->flush();
        hostScene->publish(ramses::EScenePublicationMode::LocalOnly);

        //client scene
        const ramses::SceneConfig localSceneConfig(ramses::sceneId_t(42u), ramses::EScenePublicationMode::LocalAndRemote);
        ramses::Scene* clientScene = client.createScene(localSceneConfig);
        ramses::internal::TransformationLinkScene redTriangleScene(*clientScene, ramses::internal::TransformationLinkScene::TRANSFORMATION_CONSUMER, { 0.f, 0.f, 12.f }, displayWidth, displayHeight);
        clientScene->flush();
        clientScene->publish(ramses::EScenePublicationMode::LocalOnly);

        const ramses::SceneConfig remoteSceneConfig(ramses::sceneId_t{67u}, ramses::EScenePublicationMode::LocalAndRemote);

        if (testNr == 1)
        {
            // 'sender' has two scenes, one locally shown, and one distributed remotely
            fmt::print("sender\n");

            // scene to distribute only
            ramses::Scene* remoteScene = client.createScene(remoteSceneConfig);
            ramses::internal::TransformationLinkScene blueTriangleScene(*remoteScene, ramses::internal::TransformationLinkScene::TRANSFORMATION_CONSUMER_OVERRIDEN, { 0.f, 0.f, 12.f }, displayWidth, displayHeight);
            remoteScene->flush();
            remoteScene->publish(ramses::EScenePublicationMode::LocalAndRemote);
        }
        else //if (testNrArgument == 2)
        {
            // 'receiver': has one local scene, and maps a remote scene additionally
            fmt::print("receiver mode\n");
        }

        while (rendererMate.isRunning())
        {
            rendererMate.dispatchAndFlush(dmEventHandler);
            renderer.doOneLoop();
            ramses::internal::PlatformThread::Sleep(16);
        }
    }

    void runFileLoadingTest(ramses::RamsesClient& client,
                              ramses::RamsesRenderer& renderer,
                              ramses::RamsesFrameworkConfig& frameworkConfig,
                              ramses::internal::RendererMate& rendererMate,
                              ramses::internal::RendererMateAutoShowHandler& dmEventHandler,
                              uint32_t displayWidth,
                              uint32_t displayHeight)
    {
        ramses::sceneId_t sceneId(13u);
        ramses::internal::FileLoadingScene fileLoadingScene(client, ramses::internal::FileLoadingScene::CREATE_SAVE_DESTROY_LOAD_USING_SAME_CLIENT, sceneId, { 0.f, 0.f, 5.f }, ".", frameworkConfig, displayWidth, displayHeight);
        ramses::Scene* scene = fileLoadingScene.getCreatedScene();
        scene->publish(ramses::EScenePublicationMode::LocalAndRemote);

        while (rendererMate.getLastReportedSceneState(sceneId) != ramses::RendererSceneState::Rendered)
        {
            renderer.doOneLoop();
            rendererMate.dispatchAndFlush(dmEventHandler);
            ramses::internal::PlatformThread::Sleep(16);
        }
    }

    void runNoBlackFrameTest(ramses::RamsesClient& client,
                            ramses::RamsesRenderer& renderer,
                            ramses::RamsesFramework& framework,
                            ramses::internal::RendererMate& rendererMate,
                            ramses::internal::RendererMateAutoShowHandler& dmEventHandler,
                            uint32_t displayWidth,
                            uint32_t displayHeight)
    {
        renderer.setLoopMode(ramses::ELoopMode::UpdateOnly);
        auto testStepCommand = std::make_shared<ramses::internal::TestStepCommand>();
        framework.impl().getRamsh().add(testStepCommand);

        const ramses::sceneId_t sceneId(1u);
        const ramses::SceneConfig config(sceneId, ramses::EScenePublicationMode::LocalAndRemote);
        ramses::Scene& clientScene = *client.createScene(config);
        ramses::internal::TransformationLinkScene redTriangleScene(clientScene, ramses::internal::TransformationLinkScene::TRANSFORMATION_CONSUMER, { 0.f, 0.f, 4.f }, displayWidth, displayHeight);

        clientScene.publish(ramses::EScenePublicationMode::LocalAndRemote);
        clientScene.flush();
        while (rendererMate.getLastReportedSceneState(sceneId) != ramses::RendererSceneState::Rendered)
        {
            renderer.doOneLoop();
            rendererMate.dispatchAndFlush(dmEventHandler);
            ramses::internal::PlatformThread::Sleep(16);
        }
        LOG_DEBUG(ramses::internal::CONTEXT_SMOKETEST, "Surface should be still invisible");
        // The integration test "test_run_no_initial_black_frame.py" checks here, if the surface is not yet
        // visible on the screen by doing a screenshot from the system compositor. It then sends "step 1" to
        // proceed with the test.
        while (testStepCommand->getCurrentTestStep() != 1)
        {
            renderer.doOneLoop();
            ramses::internal::PlatformThread::Sleep(50);
        }

        // Do only render a single frame, this makes the surface to become visible.
        // Before it was invisible, because it had no content.
        renderer.setLoopMode(ramses::ELoopMode::UpdateAndRender);
        renderer.doOneLoop();
        renderer.setLoopMode(ramses::ELoopMode::UpdateOnly);

        // The integration test checks here, if the surface with the scene is now visible on the screen
        // by doing a screenshot from the system compositor.

        // Don't render a frame anymore. Important for the test to really prove, that the scene
        // was rendered before the surface becomes visible the first time.
        while (true)
        {
            renderer.doOneLoop();
            ramses::internal::PlatformThread::Sleep(50);
        }
    }

    void runEmbeddedCompositingTest(ramses::RamsesClient& client,
                            ramses::RamsesRenderer& renderer,
                            ramses::displayId_t displayId,
                            ramses::internal::RendererMate& rendererMate,
                            ramses::internal::RendererMateAutoShowHandler& dmEventHandler,
                            uint32_t displayWidth,
                            uint32_t displayHeight,
                            uint32_t testState)
    {
        constexpr ramses::sceneId_t sceneId(34u);
        ramses::SceneConfig         config{sceneId, ramses::EScenePublicationMode::LocalAndRemote};
        ramses::Scene& scene = *client.createScene(config);
        ramses::internal::StreamTextureScene streamTextureScene(scene, testState, { 0.f, 0.f, 5.f }, displayWidth, displayHeight);
        scene.flush();
        scene.publish(ramses::EScenePublicationMode::LocalAndRemote);

        while (rendererMate.getLastReportedSceneState(sceneId) != ramses::RendererSceneState::Rendered)
        {
            renderer.doOneLoop();
            rendererMate.dispatchAndFlush(dmEventHandler);
            ramses::internal::PlatformThread::Sleep(16);
        }

        std::array<ramses::streamBufferId_t, 6> streamBuffers;
        streamBuffers[0] = renderer.createStreamBuffer(displayId, ramses::waylandIviSurfaceId_t{ 1u });
        streamBuffers[1] = renderer.createStreamBuffer(displayId, ramses::waylandIviSurfaceId_t{ 2u });
        streamBuffers[2] = renderer.createStreamBuffer(displayId, ramses::waylandIviSurfaceId_t{ 3u });
        streamBuffers[3] = renderer.createStreamBuffer(displayId, ramses::waylandIviSurfaceId_t{ 4u });
        streamBuffers[4] = renderer.createStreamBuffer(displayId, ramses::waylandIviSurfaceId_t{ 5u });
        streamBuffers[5] = renderer.createStreamBuffer(displayId, ramses::waylandIviSurfaceId_t{ 6u });

        const auto linkingIndices = ramses::internal::StreamTextureScene::getConsumersLinkingIndices(testState);
        for(uint32_t idx = 0u; idx < 6u; ++idx)
        {
            const auto streamBuffer = streamBuffers[linkingIndices[idx]];
            const auto textureConsumer = ramses::internal::StreamTextureScene::TextureConsumers[idx];
            renderer.getSceneControlAPI()->linkStreamBuffer(streamBuffer, sceneId, textureConsumer);
        }

        renderer.getSceneControlAPI()->flush();
        renderer.flush();

        while (true)
        {
            renderer.doOneLoop();
            rendererMate.dispatchAndFlush(dmEventHandler);
            ramses::internal::PlatformThread::Sleep(16);
        }
    }
}

int main(int argc, const char* argv[])
{
    CLI::App cli;
    ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
    frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType::Console);
    ramses::RendererConfig rendererConfig;
    ramses::DisplayConfig  displayConfig;
    uint32_t testNr = 1u;
    uint32_t testState = 0u;

    bool     disableAutoMapping = false;

    try
    {
        cli.add_option("--tn,--test-nr", testNr);
        cli.add_option("--ts,--test-state", testState);
        cli.add_flag("--no-auto-show", disableAutoMapping);

        ramses::registerOptions(cli, frameworkConfig);
        ramses::registerOptions(cli, rendererConfig);
        ramses::registerOptions(cli, displayConfig);
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
        displayConfig.setClearColor({0.0, 0.0, 1.0, 1.0});
    }
    const auto displayId = renderer.createDisplay(displayConfig);
    renderer.flush();

    int x = 0;
    uint32_t displayWidth = 0;
    uint32_t displayHeight = 0;
    displayConfig.getWindowRectangle(x, x, displayWidth, displayHeight);

    ramses::internal::RendererMate rendererMate(renderer.impl(), framework.impl());
    ramses::internal::RendererMateAutoShowHandler dmEventHandler(rendererMate, !disableAutoMapping);

    switch(testNr)
    {
    case 1:
    case 2:
        runTranformationLinkingTest(client, renderer, rendererMate, dmEventHandler, displayWidth, displayHeight, testNr);
        break;
    case 3:
        runFileLoadingTest(client, renderer, frameworkConfig, rendererMate, dmEventHandler, displayWidth, displayHeight);
        break;
    case 4:
        runNoBlackFrameTest(client, renderer, framework, rendererMate, dmEventHandler, displayWidth, displayHeight);
        break;
    case 10:
        runEmbeddedCompositingTest(client, renderer, displayId, rendererMate, dmEventHandler, displayWidth, displayHeight, testState);
        break;
    case 22:
        // for SC test with custom resolution
        runEmbeddedCompositingTest(client, renderer, displayId, rendererMate, dmEventHandler, 150u, 200u, testState);
        break;
    default:
        std::cerr << "Unsupported test case number :" << testNr << std::endl;
        exit(1);
    }

    return 0;
}
