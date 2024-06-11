//  -------------------------------------------------------------------------
//  Copyright (C) 2024 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/framework/EFeatureLevel.h"
#include "ramses/framework/DataTypes.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/framework/RamsesObjectTypes.h"
#include "ramses/client/ramses-client.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/client/Node.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderTarget.h"
#include "ramses/client/SceneObjectIterator.h"
#include "ramses/client/logic/AppearanceBinding.h"
#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/Property.h"
#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/IRendererEventHandler.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/renderer/RendererConfig.h"
#include "ramses/renderer/RendererSceneControl.h"
#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <string>
#include <string_view>
#include <thread>


const ramses::sceneId_t SCENE_ID{123};
constexpr int SCENE_COUNT = 3;
const std::array<std::string, SCENE_COUNT> FILE_NAMES{"tempfileA.ramses", "tempfileB.ramses", "tempfileC.ramses"};
const std::array<ramses::vec4f, SCENE_COUNT> COLORS{ramses::vec4f{1.0f, 0.0f, 0.0f, 1.0f}, ramses::vec4f{0.0f, 1.0f, 0.0f, 1.0f}, ramses::vec4f{0.0f, 0.0f, 1.0f, 1.0f}};
const std::array<ramses::vec3f, SCENE_COUNT> TRANSLATIONS{ramses::vec3f{-1.0f, 0.0f, 0.0f}, ramses::vec3f{1.0f, 0.0f, 0.0f}, ramses::vec3f{0.0f, 0.0f, 0.0f}};
const std::array<float, SCENE_COUNT> ROTATIONS{45.0f, -20.0f, 5.0f};
const std::chrono::seconds DELAY{5};


bool createScene(int sceneNumber);
bool mergeScenes(const std::vector<std::string>& files);


class RendererEventHandler : public ramses::RendererEventHandlerEmpty
{
public:
    void windowClosed(ramses::displayId_t /*displayId*/) override
    {
        m_windowClosed = true;
    }

    [[nodiscard]] bool isWindowClosed() const
    {
        return m_windowClosed;
    }

private:
    bool m_windowClosed = false;
};


/**
 * @example ramses-example-local-scene-merge/src/main.cpp
 * @brief Scene Merging Example
 */
int main()
{
    std::vector<std::string> fileNames;

    for (int i = 0; i < SCENE_COUNT; ++i)
    {
        if (!createScene(i))
        {
            return EXIT_FAILURE;
        }
        fileNames.push_back(FILE_NAMES[i]);
    }

    if (!mergeScenes(fileNames))
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

std::string generateName(std::string const & str, int sceneNumber)
{
    return str + "_" + std::to_string(sceneNumber);
}

bool createScene(int sceneNumber)
{
    constexpr uint32_t width = 1280u;
    constexpr uint32_t height = 480u;
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    config.setLogLevelConsole(ramses::ELogLevel::Trace);
    config.setLoggingInstanceName(generateName("Scene", sceneNumber));
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-local-scene-merge"));
    const ramses::SceneConfig sceneConfig(SCENE_ID);
    ramses::Scene* scene = ramses.createScene(sceneConfig, "scene merging from file");

    // every scene needs a render pass with camera
    auto* camera = scene->createPerspectiveCamera(generateName("my camera", sceneNumber));
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    camera->setTranslation({0.0f, 0.0f, 5.0f});
    ramses::RenderPass* renderPass = scene->createRenderPass(generateName("my render pass", sceneNumber));
    renderPass->setClearFlags(ramses::EClearFlag::None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup(generateName("group", sceneNumber));
    renderPass->addRenderGroup(*renderGroup);

    const std::array<ramses::vec3f, 4u> vertexPositionsArray{ ramses::vec3f{-0.5f, -0.5f, -1.f}, ramses::vec3f{0.5f, -0.5f, -1.f}, ramses::vec3f{-0.5f, 0.5f, -1.f}, ramses::vec3f{0.5f, 0.5f, -1.f} };
    ramses::ArrayResource* vertexPositions = scene->createArrayResource(4u, vertexPositionsArray.data());

    const std::array<ramses::vec2f, 4u> textureCoordsArray{ ramses::vec2f{0.f, 1.f}, ramses::vec2f{1.f, 1.f}, ramses::vec2f{0.f, 0.f}, ramses::vec2f{1.f, 0.f} };
    ramses::ArrayResource* textureCoords = scene->createArrayResource(4u, textureCoordsArray.data());

    const std::array<uint16_t, 6u> indicesArray{ 0, 1, 2, 2, 1, 3 };
    ramses::ArrayResource* indices = scene->createArrayResource(6u, indicesArray.data());

    char const * const fileNameTexture = "res/ramses-example-local-scene-merge-texture.png";
    char const * const fileNameTextureInverted = "res/ramses-example-local-scene-merge-texture-inverted.png";

    ramses::Texture2D* texture = ramses::RamsesUtils::CreateTextureResourceFromPng(sceneNumber == 0 ? fileNameTexture : fileNameTextureInverted, *scene);
    ramses::TextureSampler* sampler = scene->createTextureSampler(
        ramses::ETextureAddressMode::Repeat,
        ramses::ETextureAddressMode::Repeat,
        ramses::ETextureSamplingMethod::Linear,
        ramses::ETextureSamplingMethod::Linear,
        *texture);

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-local-scene-merge-texturing.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-local-scene-merge-texturing.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effectTex = scene->createEffect(effectDesc, generateName("glsl shader", sceneNumber));

    ramses::Appearance* appearance = scene->createAppearance(*effectTex, generateName("triangle appearance", sceneNumber));
    auto colorUniform = effectTex->findUniformInput("uniformBlock.color");
    appearance->setInputValue(colorUniform.value(), ramses::vec4f{1.0f, 1.0f, 0.0f, 1.0f});

    ramses::Geometry* geometry = scene->createGeometry(*effectTex, generateName("triangle geometry", sceneNumber));

    geometry->setIndices(*indices);
    std::optional<ramses::AttributeInput> positionsInput = effectTex->findAttributeInput("a_position");
    std::optional<ramses::AttributeInput> texcoordsInput = effectTex->findAttributeInput("a_texcoord");
    std::optional<ramses::UniformInput>   textureInput   = effectTex->findUniformInput("textureSampler");
    assert(positionsInput.has_value() && texcoordsInput.has_value() && textureInput.has_value());
    geometry->setInputBuffer(*positionsInput, *vertexPositions);
    geometry->setInputBuffer(*texcoordsInput, *textureCoords);
    appearance->setInputTexture(*textureInput, *sampler);

    ramses::Node* scaleNode = scene->createNode(generateName("scale node", sceneNumber));
    scaleNode->setScaling({2.0f, 2.0f, 2.0f});

    ramses::MeshNode* meshNode = scene->createMeshNode(generateName("textured triangle mesh node", sceneNumber));
    meshNode->setAppearance(*appearance);
    meshNode->setGeometry(*geometry);

    meshNode->setTranslation(TRANSLATIONS[sceneNumber]);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);

    scaleNode->addChild(*meshNode);

    scene->flush();

    // create logic engine that sets the rotation of the meshes
    ramses::LogicEngine& logicEngine{ *scene->createLogicEngine(generateName("example logic", sceneNumber))};
    ramses::NodeBinding* nodeBinding = logicEngine.createNodeBinding(*meshNode, ramses::ERotationType::Euler_XYZ, "link to node");

    auto* appearanceBinding = logicEngine.createAppearanceBinding(*appearance, generateName("appearance binding", sceneNumber));

    ramses::LuaScript* simpleScript = logicEngine.createLuaScript(R"(
            function interface(IN,OUT)
                IN.rotationZ = Type:Float()
                OUT.rotationZ = Type:Vec3f()
            end

            function run(IN,OUT)
                -- Rotate around Z axis with 100 degrees per second
                OUT.rotationZ = {0, 0, IN.rotationZ}
            end
        )", {}, "simple rotation script");

    const std::string_view interfaceSrc = R"(
        function interface(inout_params)
            inout_params.rotationZ = Type:Float()
            inout_params.color = Type:Vec4f()
        end
    )";

    ramses::LuaInterface* intf = logicEngine.createLuaInterface(interfaceSrc, "Interface");
    logicEngine.link(*simpleScript->getOutputs()->getChild("rotationZ"), *nodeBinding->getInputs()->getChild("rotation"));
    logicEngine.link(
        *intf->getOutputs()->getChild("rotationZ"),
        *simpleScript->getInputs()->getChild("rotationZ"));

    logicEngine.link(*intf->getOutputs()->getChild("color"), *appearanceBinding->getInputs()->getChild("uniformBlock.color"));

    // Let's initialize the interface's input with some value
    intf->getInputs()->getChild("rotationZ")->set<float>(ROTATIONS[sceneNumber]);
    intf->getInputs()->getChild("color")->set(COLORS[sceneNumber]);

    /**
    * Call update() before saving to ensure the ramses scene is in a state where all settings (in this case, the node's rotation)
    * have been set once before saving
    */
    logicEngine.update();

    bool result = scene->saveToFile(FILE_NAMES[sceneNumber], {});

    ramses.destroy(*scene);

    return result;
}

bool mergeScenes(const std::vector<std::string>& files)
{
    if (files.empty())
    {
        return false;
    }

    ramses::EFeatureLevel featureLevel = ramses::EFeatureLevel_Latest;
    if (!ramses::RamsesClient::GetFeatureLevelFromFile(files[0], featureLevel))
    {
        return false;
    }

    // load the saved file
    ramses::RamsesFrameworkConfig config{featureLevel};
    config.setRequestedRamsesShellType(ramses::ERamsesShellType::Console);
    config.setLogLevel(ramses::ELogLevel::Info);
    config.setPeriodicLogInterval(std::chrono::seconds(0));
    config.setLoggingInstanceName("Merge");
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-local-scene-merge"));

    ramses::RendererConfig rendererConfig;
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    ramses::RendererSceneControl& sceneControlAPI = *renderer.getSceneControlAPI();
    renderer.startThread();

    ramses::DisplayConfig displayConfig;
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);
    renderer.flush();

    RendererEventHandler eventHandler;
    ramses::RendererSceneControlEventHandlerEmpty sceneControlEventHandler;

    ramses::Scene* loadedScene = nullptr;
    for (size_t sceneIndex = 0; sceneIndex < files.size(); ++sceneIndex)
    {
        if (sceneIndex == 0)
        {
            // load scene from first file
            loadedScene = ramses.loadSceneFromFile(files[0], ramses::SceneConfig(SCENE_ID, ramses::EScenePublicationMode::LocalOnly));
            if (!loadedScene)
            {
                return false;
            }

            loadedScene->publish(ramses::EScenePublicationMode::LocalOnly);

            // show the scene on the renderer
            sceneControlAPI.setSceneMapping(SCENE_ID, display);
            sceneControlAPI.setSceneState(SCENE_ID, ramses::RendererSceneState::Rendered);
            sceneControlAPI.flush();
        }
        else
        {
            // merge the remaining files into the scene one by one
            ramses.mergeSceneFromFile(*loadedScene, files[sceneIndex]);
            loadedScene->flush();
        }

        const auto startTime = std::chrono::steady_clock::now();
        while (!eventHandler.isWindowClosed() && (std::chrono::steady_clock::now() - startTime) < DELAY)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(16));
            loadedScene->flush();
            renderer.dispatchEvents(eventHandler);
            sceneControlAPI.dispatchEvents(sceneControlEventHandler);
        }
    }

    if (!loadedScene)
    {
        return false;
    }

    loadedScene->unpublish();
    ramses.destroy(*loadedScene);

    return true;
}
