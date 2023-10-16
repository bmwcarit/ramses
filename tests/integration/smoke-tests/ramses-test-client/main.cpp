//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/ramses-client.h"
#include "ramses/client/Scene.h"
#include "TestScenes/HierarchicalRedTrianglesScene.h"
#include "TestScenes/MultipleTrianglesScene.h"
#include "TestScenes/TextScene.h"
#include "TestScenes/MultiLanguageTextScene.h"
#include "TestScenes/FileLoadingScene.h"
#include "TestScenes/MultiTypeLinkScene.h"
#include "TestScenes/MultipleRenderTargetScene.h"
#include "TestScenes/SceneFromPath.h"
#include "ramses/framework/RamsesFramework.h"
#include "impl/RamsesFrameworkImpl.h"
#include "internal/Ramsh/RamshCommandExit.h"
#include "TestStepCommand.h"
#include "internal/Ramsh/Ramsh.h"
#include "ramses-cli.h"

using SceneVector = std::vector<ramses::Scene*>;
using IntegrationScenePtr = std::unique_ptr<ramses::internal::IntegrationScene>;

ramses::Scene& addAndReturnScene(ramses::RamsesClient& ramses, SceneVector& scenes, ramses::sceneId_t sceneId)
{
    const ramses::SceneConfig config{sceneId, ramses::EScenePublicationMode::LocalAndRemote};
    ramses::Scene* scene = ramses.createScene(config);

    scenes.push_back(scene);

    return *scene;
}

void publishAllScenes(SceneVector& scenes)
{
    for(auto scene : scenes)
    {
        scene->publish(ramses::EScenePublicationMode::LocalAndRemote);
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

static constexpr uint32_t DefaultViewportWidth = 1280u;
static constexpr uint32_t DefaultViewportHeight = 480u;

template <typename INTEGRATIONSCENE>
IntegrationScenePtr createSceneAndSetState(
    ramses::RamsesClient& ramses,
    std::vector<ramses::Scene*>& scenes,
    uint32_t testState,
    ramses::sceneId_t sceneId,
    const glm::vec3& cameraPosition = glm::vec3(0.0f),
    uint32_t vpWidth = DefaultViewportWidth,
    uint32_t vpHeight = DefaultViewportHeight
    )
{
    ramses::Scene& scene = addAndReturnScene(ramses, scenes, sceneId);
    IntegrationScenePtr integrationScene(new INTEGRATIONSCENE(scene, testState, cameraPosition, vpWidth, vpHeight));
    scene.flush();

    return integrationScene;
}

int main(int argc, const char* argv[])
{
    CLI::App cli;
    uint32_t testNr = 5u;
    uint32_t testState = 1u;
    float cameraX = 0.f;
    float cameraY = 0.f;
    float cameraZ = 0.f;
    ramses::RamsesFrameworkConfig frameworkConfig{ramses::EFeatureLevel_Latest};
    frameworkConfig.setRequestedRamsesShellType(ramses::ERamsesShellType::Console);
    std::string folder = ".";
    std::string filename;

    try
    {
        cli.add_option("--tn,--test-nr", testNr);
        cli.add_option("--ts,--test-state", testState);
        cli.add_option("--cx", cameraX, "Camera position x");
        cli.add_option("--cy", cameraY, "Camera position y");
        cli.add_option("--cz", cameraZ, "Camera position z");
        cli.add_option("--folder", folder);
        cli.add_option("--filename", filename);
        ramses::registerOptions(cli, frameworkConfig);
    }
    catch (const CLI::Error& error)
    {
        std::cerr << error.what();
        return -1;
    }
    CLI11_PARSE(cli, argc, argv);

    ramses::RamsesFramework framework(frameworkConfig);
    auto commandExit = std::make_shared<ramses::internal::RamshCommandExit>();
    framework.impl().getRamsh().add(commandExit);
    auto testStepCommand = std::make_shared<ramses::internal::TestStepCommand>();
    framework.impl().getRamsh().add(testStepCommand);

    ramses::RamsesClient* ramses(framework.createClient("ramses test client"));
    if (!ramses)
    {
        return 1;
    }

    framework.connect();

    // Default scene, when no parameters set
    // Used by integration tests which don't expect any particular rendered image

    glm::vec3 cameraPosParam(cameraX, cameraY, cameraZ);

    std::vector<ramses::Scene*> scenes;
    std::vector<IntegrationScenePtr> integrationScenes;

    switch (testNr)
    {
    case 2:
    {
        auto integrationScene1 = createSceneAndSetState<ramses::internal::MultipleTrianglesScene>(*ramses, scenes, ramses::internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses::sceneId_t(21u),
            cameraPosParam + glm::vec3(2.0f, 0.0f, 0.0f));
        auto integrationScene2 = createSceneAndSetState<ramses::internal::MultipleTrianglesScene>(*ramses, scenes, ramses::internal::MultipleTrianglesScene::TRIANGLES_REORDERED, ramses::sceneId_t(22u),
            cameraPosParam + glm::vec3(-2.0f, 0.0f, 0.0f));
        integrationScenes.push_back(std::move(integrationScene1));
        integrationScenes.push_back(std::move(integrationScene2));
        break;
    }
    case 3:
    {
        auto integrationScene1 = createSceneAndSetState<ramses::internal::MultipleTrianglesScene>(*ramses, scenes, ramses::internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses::sceneId_t(23u),
            cameraPosParam + glm::vec3(2.0f, 0.0f, 0.0f));
        auto integrationScene2 = createSceneAndSetState<ramses::internal::MultipleTrianglesScene>(*ramses, scenes, ramses::internal::MultipleTrianglesScene::TRIANGLES_REORDERED, ramses::sceneId_t(24u),
            cameraPosParam + glm::vec3(-2.0f, 0.0f, 0.0f));
        integrationScenes.push_back(std::move(integrationScene1));
        integrationScenes.push_back(std::move(integrationScene2));
        ramses::Scene* sceneToBeDestroyed = scenes.front();
        assert(framework.isConnected());
        sceneToBeDestroyed->publish(ramses::EScenePublicationMode::LocalAndRemote);
        ramses->destroy(*sceneToBeDestroyed);
        scenes.erase(scenes.begin());
        break;
    }
    case 4:
    {
        auto integrationScene = createSceneAndSetState<ramses::internal::HierarchicalRedTrianglesScene>(*ramses, scenes, testState, ramses::sceneId_t(25u), cameraPosParam);
        integrationScenes.push_back(std::move(integrationScene));
        break;
    }
    case 5:
    {
        auto integrationScene = createSceneAndSetState<ramses::internal::MultipleTrianglesScene>(*ramses, scenes, testState, ramses::sceneId_t(26u), cameraPosParam);
        integrationScenes.push_back(std::move(integrationScene));
        break;
    }
    case 7:
    {
        auto integrationScene1 = createSceneAndSetState<ramses::internal::MultipleTrianglesScene>(*ramses, scenes, ramses::internal::MultipleTrianglesScene::TRIANGLES_REORDERED, ramses::sceneId_t(29u),
                                                                                                 cameraPosParam + glm::vec3(30.0f, 0.0f, 0.0f));

        auto integrationScene2 = createSceneAndSetState<ramses::internal::MultipleTrianglesScene>(*ramses, scenes, ramses::internal::MultipleTrianglesScene::THREE_TRIANGLES, ramses::sceneId_t(30u),
                                                                                                 cameraPosParam + glm::vec3(10.0f, 0.0f, 0.0f));

        auto integrationScene3 = createSceneAndSetState<ramses::internal::HierarchicalRedTrianglesScene>(*ramses, scenes, ramses::internal::HierarchicalRedTrianglesScene::THREE_ROWS_TRIANGLES, ramses::sceneId_t(31u),
                                                                                                        cameraPosParam + glm::vec3(-10.0f, 0.0f, 0.0f));

        auto integrationScene4 = createSceneAndSetState<ramses::internal::HierarchicalRedTrianglesScene>(*ramses, scenes, ramses::internal::HierarchicalRedTrianglesScene::THREE_ROWS_TRIANGLES, ramses::sceneId_t(32u),
                                                                                                        cameraPosParam + glm::vec3(-30.0f, 0.0f, 0.0f));

        integrationScenes.push_back(std::move(integrationScene1));
        integrationScenes.push_back(std::move(integrationScene2));
        integrationScenes.push_back(std::move(integrationScene3));
        integrationScenes.push_back(std::move(integrationScene4));
        break;
    }
    case 13:
    {
        const ramses::sceneId_t sceneId(37u);
        ramses::internal::FileLoadingScene fileLoadingScene(*ramses, testState, sceneId, cameraPosParam, folder, frameworkConfig, DefaultViewportWidth, DefaultViewportHeight);
        scenes.push_back(fileLoadingScene.getCreatedScene());
        break;
    }
    case 15:
    {
        auto integrationScene1 = createSceneAndSetState<ramses::internal::MultiTypeLinkScene>(*ramses, scenes, ramses::internal::MultiTypeLinkScene::TRANSFORMATION_CONSUMER_DATA_AND_TEXTURE_PROVIDER, ramses::sceneId_t(39u), cameraPosParam + glm::vec3(1.f, -2.f, 0.f));
        auto integrationScene2 = createSceneAndSetState<ramses::internal::MultiTypeLinkScene>(*ramses, scenes, ramses::internal::MultiTypeLinkScene::TRANSFORMATION_PROVIDER_DATA_AND_TEXTURE_CONSUMER, ramses::sceneId_t(40u), cameraPosParam + glm::vec3(-1.f, 2.f, 0.f));
        integrationScenes.push_back(std::move(integrationScene1));
        integrationScenes.push_back(std::move(integrationScene2));
        break;
    }
    case 16:
    {
        auto integrationScene = createSceneAndSetState<ramses::internal::MultipleRenderTargetScene>(*ramses, scenes, ramses::internal::MultipleRenderTargetScene::TWO_COLOR_BUFFERS, ramses::sceneId_t(41u), cameraPosParam);
        integrationScenes.push_back(std::move(integrationScene));
        break;
    }
#if defined(RAMSES_TEXT_ENABLED)
    case 18:
    {
        auto integrationScene = createSceneAndSetState<ramses::internal::TextScene>(*ramses, scenes, testState, ramses::sceneId_t(43u), cameraPosParam);
        integrationScenes.push_back(std::move(integrationScene));
        break;
    }
#endif
    case 19:
    {
        ramses::internal::SceneFromPath sceneFromPath(*ramses, folder, filename);
        scenes.push_back(sceneFromPath.getCreatedScene());
        break;
    }
    case 20:
    {
        // for IVI layer test with custom resolution
        auto integrationScene = createSceneAndSetState<ramses::internal::HierarchicalRedTrianglesScene>(*ramses, scenes, testState, ramses::sceneId_t(25u), cameraPosParam, 640u, 480u);
        integrationScenes.push_back(std::move(integrationScene));
        break;
    }
    case 21:
    {
        // for IVI layer test with custom resolution
        auto integrationScene = createSceneAndSetState<ramses::internal::MultipleTrianglesScene>(*ramses, scenes, testState, ramses::sceneId_t(26u), cameraPosParam, 640u, 480u);
        integrationScenes.push_back(std::move(integrationScene));
        break;
    }
    default:
        std::cerr << "Unhandled test number :" << testNr << std::endl;
        return -1;
    }

    // handle the case that testscene disconnects framework
    if (framework.isConnected())
    {
        publishAllScenes(scenes);
    }

    while(!commandExit->exitRequested())
    {
        commandExit->waitForExitRequest(1000u);
        flushAllScenes(scenes);
        for (auto& scene : integrationScenes)
            scene->dispatchHandler();
    }
    integrationScenes.clear();

    if (framework.isConnected())
    {
        destroyAllScenes(*ramses, scenes);

        framework.disconnect();
    }

    return 0;
}
