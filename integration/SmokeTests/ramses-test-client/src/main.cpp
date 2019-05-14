//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"
#include "ramses-client-api/Scene.h"
#include "ramses-capu/os/Thread.h"
#include "Utils/CommandLineParser.h"
#include "Utils/Argument.h"
#include "TestScenes/HierarchicalRedTrianglesScene.h"
#include "TestScenes/MultipleTrianglesScene.h"
#include "TestScenes/AnimatedTrianglesScene.h"
#include "TestScenes/CubeTextureScene.h"
#include "TestScenes/TextScene.h"
#include "TestScenes/StreamTextureScene.h"
#include "TestScenes/MultiLanguageTextScene.h"
#include "TestScenes/DistributedScene.h"
#include "TestScenes/FileLoadingScene.h"
#include "TestScenes/TransformationLinkScene.h"
#include "TestScenes/MultiTypeLinkScene.h"
#include "TestScenes/MultiTextureConsumerScene.h"
#include "TestScenes/MultipleRenderTargetScene.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "RamsesFrameworkImpl.h"
#include "Ramsh/RamshCommandExit.h"
#include "TestStepCommand.h"


typedef std::vector<ramses::Scene*> SceneVector;

ramses::Scene& addAndReturnScene(ramses::RamsesClient& ramses, SceneVector& scenes, ramses_internal::UInt32 sceneId)
{
    ramses::Scene* scene = ramses.createScene(sceneId);

    scenes.push_back(scene);

    return *scene;
}

void publishAllScenes(SceneVector& scenes)
{
    for(auto scene : scenes)
    {
        scene->publish();
    }
}

void destroyAllScenes(ramses::RamsesClient& ramses, SceneVector& scenes)
{
    for(auto scene : scenes)
    {
        ramses.destroy(*scene);
    }
}

void flushAllScenes(SceneVector& scenes)
{
    for(auto scene : scenes)
    {
        scene->flush();
    }
}

template <typename INTEGRATIONSCENE>
void createSceneAndSetState(
    ramses::RamsesClient& ramses,
    std::vector<ramses::Scene*>& scenes,
    ramses_internal::UInt32 testState,
    ramses_internal::UInt32 sceneId,
    const ramses_internal::Vector3& cameraPosition = ramses_internal::Vector3(0.0f)
    )
{
    ramses::Scene& scene = addAndReturnScene(ramses, scenes, sceneId);
    INTEGRATIONSCENE integrationScene(ramses, scene, testState, cameraPosition);
    scene.flush();
}

int main(int argc, const char* argv[])
{
    ramses::RamsesFrameworkConfig frameworkConfig(argc, argv);
    frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
    ramses::RamsesFramework framework(frameworkConfig);
    ramses_internal::RamshCommandExit commandExit;
    framework.impl.getRamsh().add(commandExit);
    ramses_internal::TestStepCommand testStepCommand;
    framework.impl.getRamsh().add(testStepCommand);

    ramses::RamsesClient ramses("ramses-test-client", framework);

    framework.connect();

    ramses_internal::CommandLineParser parser(argc, argv);
    // Default scene, when no parameters set
    // Used by integration tests which don't expect any particular rendered image
    ramses_internal::ArgumentUInt32 testNrArgument(parser, "tn", "test-nr", 5u);
    ramses_internal::ArgumentUInt32 testStateArgument(parser, "ts", "test-state-nr", 1u);
    ramses_internal::ArgumentFloat testCameraPosXArgument(parser, "cx", "camPosX", 0.0f);
    ramses_internal::ArgumentFloat testCameraPosYArgument(parser, "cy", "camPosY", 0.0f);
    ramses_internal::ArgumentFloat testCameraPosZArgument(parser, "cz", "camPosZ", 0.0f);

    ramses_internal::UInt32 testNr = testNrArgument;
    ramses_internal::UInt32 testState = testStateArgument;
    ramses_internal::Vector3 cameraPosParam(testCameraPosXArgument, testCameraPosYArgument, testCameraPosZArgument);

    std::vector<ramses::Scene*> scenes;

    switch (testNr)
    {
    case 1:
    {
        createSceneAndSetState<ramses_internal::DistributedScene>(ramses, scenes, testState, 20u, cameraPosParam);
        break;
    }
    case 2:
    {
        createSceneAndSetState<ramses_internal::MultipleTrianglesScene>(ramses, scenes, ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, 21u,
            cameraPosParam + ramses_internal::Vector3(2.0f, 0.0f, 0.0f));
        createSceneAndSetState<ramses_internal::MultipleTrianglesScene>(ramses, scenes, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED, 22u,
            cameraPosParam + ramses_internal::Vector3(-2.0f, 0.0f, 0.0f));
        break;
    }
    case 3:
    {
        createSceneAndSetState<ramses_internal::MultipleTrianglesScene>(ramses, scenes, ramses_internal::MultipleTrianglesScene::THREE_TRIANGLES, 23u,
            cameraPosParam + ramses_internal::Vector3(2.0f, 0.0f, 0.0f));
        createSceneAndSetState<ramses_internal::MultipleTrianglesScene>(ramses, scenes, ramses_internal::MultipleTrianglesScene::TRIANGLES_REORDERED, 24u,
            cameraPosParam + ramses_internal::Vector3(-2.0f, 0.0f, 0.0f));
        ramses::Scene* sceneToBeDestroyed = scenes.front();
        assert(framework.isConnected());
        sceneToBeDestroyed->publish();
        ramses.destroy(*sceneToBeDestroyed);
        scenes.erase(scenes.begin());
        break;
    }
    case 4:
    {
        createSceneAndSetState<ramses_internal::HierarchicalRedTrianglesScene>(ramses, scenes, testState, 25u, cameraPosParam);
        break;
    }
    case 5:
    {
        createSceneAndSetState<ramses_internal::MultipleTrianglesScene>(ramses, scenes, testState, 26u, cameraPosParam);
        break;
    }
    case 6:
    {
        createSceneAndSetState<ramses_internal::AnimatedTrianglesScene>(ramses, scenes, testState, 27u, cameraPosParam);
        break;
    }
    case 7:
    {
        {
            createSceneAndSetState<ramses_internal::AnimatedTrianglesScene>(
                ramses, scenes, ramses_internal::AnimatedTrianglesScene::ANIMATION_POINT1, 28u, cameraPosParam + ramses_internal::Vector3(50.0f, 0.0f, 0.0f));
        }
        {
            createSceneAndSetState<ramses_internal::AnimatedTrianglesScene>(
                ramses, scenes, ramses_internal::AnimatedTrianglesScene::ANIMATION_POINT2, 29u, cameraPosParam + ramses_internal::Vector3(25.0f, 0.0f, 0.0f));
        }
        {
            createSceneAndSetState<ramses_internal::AnimatedTrianglesScene>(
                ramses, scenes, ramses_internal::AnimatedTrianglesScene::ANIMATION_POINT3, 30u, cameraPosParam + ramses_internal::Vector3(0.0f, 0.0f, 0.0f));
        }
        {
            createSceneAndSetState<ramses_internal::HierarchicalRedTrianglesScene>(
                ramses, scenes, ramses_internal::HierarchicalRedTrianglesScene::THREE_ROWS_TRIANGLES, 31u, cameraPosParam + ramses_internal::Vector3(-25.0f, 0.0f, 0.0f));
        }
        {
            createSceneAndSetState<ramses_internal::HierarchicalRedTrianglesScene>(
                ramses, scenes, ramses_internal::HierarchicalRedTrianglesScene::THREE_ROWS_TRIANGLES, 32u, cameraPosParam + ramses_internal::Vector3(-50.0f, 0.0f, 0.0f));
        }
        break;
    }
    case 8:
    {
        createSceneAndSetState<ramses_internal::CubeTextureScene>(ramses, scenes, testState, 33u, cameraPosParam);
        break;
    }
    case 10:
    {
        createSceneAndSetState<ramses_internal::StreamTextureScene>(ramses, scenes, testState, 34u, cameraPosParam);
        break;
    }
    case 11:
    {
        // do not occupy, should be an empty scene
        break;
    }
    case 13:
    {
        ramses_internal::ArgumentString folderArgument(parser, "folder", "folder", ".");
        const ramses::sceneId_t sceneId(37u);
        ramses_internal::FileLoadingScene fileLoadingScene(ramses, testState, sceneId, cameraPosParam, folderArgument, ramses::RamsesFrameworkConfig(argc, argv));
        scenes.push_back(fileLoadingScene.getCreatedScene());
        break;
    }
    case 14:
    {
        createSceneAndSetState<ramses_internal::TransformationLinkScene>(ramses, scenes, testState, 38u, cameraPosParam);
        break;
    }
    case 15:
    {
        createSceneAndSetState<ramses_internal::MultiTypeLinkScene>(ramses, scenes, ramses_internal::MultiTypeLinkScene::TRANSFORMATION_CONSUMER_DATA_AND_TEXTURE_PROVIDER, 39u, cameraPosParam + ramses_internal::Vector3(1.f, -2.f, 0.f));
        createSceneAndSetState<ramses_internal::MultiTypeLinkScene>(ramses, scenes, ramses_internal::MultiTypeLinkScene::TRANSFORMATION_PROVIDER_DATA_AND_TEXTURE_CONSUMER, 40u, cameraPosParam + ramses_internal::Vector3(-1.f, 2.f, 0.f));
        break;
    }
    case 16:
    {
        createSceneAndSetState<ramses_internal::MultipleRenderTargetScene>(ramses, scenes, ramses_internal::MultipleRenderTargetScene::TWO_COLOR_BUFFERS, 41u, cameraPosParam);
        break;
    }
    case 17:
    {
        createSceneAndSetState<ramses_internal::MultiTextureConsumerScene>(ramses, scenes, ramses_internal::MultiTextureConsumerScene::THREE_CONSUMERS, 42u, cameraPosParam);
        break;
    }
    }

    // handle the case that testscene disconnects framework
    if (framework.isConnected())
    {
        publishAllScenes(scenes);
    }

    switch (testNr)
    {
    case 1:
    {
        assert(framework.isConnected());
        ramses::Scene* scene = scenes.front();
        while (testStepCommand.getCurrentTestStep() != 1 && !commandExit.exitRequested())
        {
            testStepCommand.waitForTestStepSetEvent(100);
        }
        scene->unpublish();

        break;
    }
    }

    while(!commandExit.exitRequested())
    {
        commandExit.waitForExitRequest(1000u);
        flushAllScenes(scenes);
    }

    if (framework.isConnected())
    {
        destroyAllScenes(ramses, scenes);

        framework.disconnect();
    }

    return 0;
}
