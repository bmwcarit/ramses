//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/AnimationNodeConfig.h"
#include "ramses-logic/AnimationTypes.h"
#include "ramses-logic/EPropertyType.h"
#include "ramses-logic/DataArray.h"
#include "ramses-logic/TimerNode.h"
#include "ramses-logic/AnchorPoint.h"

#include "ramses-client.h"
#include "ramses-utils.h"

#include "SimpleRenderer.h"

#include <cassert>
#include <iostream>
#include <thread>
#include <vector>
#include <cmath>

/**
* This example demonstrates how to use AnchorPoint to track
* Ramses scene content in 2D.
*/

struct SceneAndNodes
{
    ramses::Scene* scene;
    ramses::Node* node;
    ramses::PerspectiveCamera* mainCamera;
    ramses::OrthographicCamera* orthoCamera;
};

/**
* Helper method which creates a simple ramses scene. For more ramses
* examples, check the ramses docs at https://bmwcarit.github.io/ramses
*/
SceneAndNodes CreateSceneWithTriangles(ramses::RamsesClient& client, std::array<uint32_t, 2> displaySize);

/**
* Helper to create a simple animation for given node, see animation example for more details on how to create an animation.
*/
void CreateAnimationForNode(rlogic::LogicEngine& logicEngine, const rlogic::RamsesNodeBinding* nodeBinding);

int main()
{
    /**
    * Use simple class to create ramses framework objects which are not essential for this example.
    * For more info on those, please refer to the ramses docs: https://bmwcarit.github.io/ramses
    */
    SimpleRenderer renderer;

    /**
     * Create a simple Ramses scene with 3D mesh (a triangle) and a camera it is rendered with.
     * We will track the 3D mesh using AnchorPoint with the help of a 2D camera that will visualize
     * the tracking.
     */
    auto [scene, node3dToTrack, camera3d, cameraSimulating2d] = CreateSceneWithTriangles(*renderer.getClient(), SimpleRenderer::GetDisplaySize());

    rlogic::LogicEngine logicEngine{ ramses::EFeatureLevel_Latest };

    /**
    * First we need to create a Ramses Logic representation of the node we will track using Ramses binding.
    */
    rlogic::RamsesNodeBinding* nodeToTrackBinding = logicEngine.createRamsesNodeBinding(*node3dToTrack);

    /**
    * Create a simple animation for the 3D mesh we are about to track, see animation example for more details on how to create an animation.
    */
    CreateAnimationForNode(logicEngine, nodeToTrackBinding);

    /**
    * Show the scene on the renderer
    */
    renderer.showScene(scene->getSceneId());

    /**
    * Create camera binding for the camera that is used for rendering the 3D mesh we want to track
    */
    rlogic::RamsesCameraBinding* camera3dBinding = logicEngine.createRamsesCameraBinding(*camera3d);

    /**
    * Finally create anchor point, we must provide not only the node we want to track but also the camera that is used to render the 3D mesh
    * associated with that node.
    */
    const rlogic::AnchorPoint* anchorPoint = logicEngine.createAnchorPoint(*nodeToTrackBinding, *camera3dBinding);

    /**
     * Simulate an application loop.
     */
    while (!renderer.isWindowClosed())
    {
        /**
        * Update the LogicEngine. This will apply changes to Ramses scene from any running animation.
        */
        logicEngine.update();

        /**
        * In this example we manually extract the 2D coordinates and apply them to the special camera viewport to visualize
        * that the tracking of the 3D mesh (triangle) works. The result will be a white box snapped to origin of the triangle.
        * Alternatively the coordinates can be linked to an another logic node (e.g. LuaScript) to be processed further, perhaps
        * end up transforming another mesh (e.g. text or 2D element).
        */
        const rlogic::vec2f coords = *anchorPoint->getOutputs()->getChild("viewportCoords")->get<rlogic::vec2f>();
        const rlogic::vec2i coords2d{ static_cast<int32_t>(std::lround(coords[0])), static_cast<int32_t>(std::lround(coords[1])) };
        cameraSimulating2d->setViewport(coords2d[0], coords2d[1], cameraSimulating2d->getViewportWidth(), cameraSimulating2d->getViewportHeight());

        /**
        * In order to commit the changes to Ramses scene caused by animations logic we need to "flush" them.
        */
        scene->flush();

        /**
        * Process window events, check if window was closed
        */
        renderer.processEvents();

        /**
        * Throttle the simulation loop by sleeping for a bit.
        */
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}


SceneAndNodes CreateSceneWithTriangles(ramses::RamsesClient& client, std::array<uint32_t, 2> displaySize)
{
    ramses::Scene* scene = client.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "red triangle scene");

    ramses::PerspectiveCamera* camera = scene->createPerspectiveCamera();
    camera->setFrustum(19.0f, float(displaySize[0])/float(displaySize[1]), 0.1f, 100.0f);
    camera->setViewport(0, 0, displaySize[0], displaySize[1]);
    camera->setTranslation({0.0f, 0.0f, 10.0f});

    ramses::RenderPass* renderPass = scene->createRenderPass();
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    std::array<ramses::vec3f, 3u> vertexPositionsArray{ ramses::vec3f{-1.f, 0.f, 0.f}, ramses::vec3f{1.f, 0.f, 0.f}, ramses::vec3f{0.f, 1.f, 0.f} };
    ramses::ArrayResource* vertexPositions = scene->createArrayResource(3u, vertexPositionsArray.data());

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShader(R"(
        #version 100

        uniform highp mat4 mvpMatrix;

        attribute vec3 a_position;

        void main()
        {
            gl_Position = mvpMatrix * vec4(a_position, 1.0);
        }
        )");
    effectDesc.setFragmentShader(R"(
        #version 100

        uniform highp vec4 color;

        void main(void)
        {
            gl_FragColor = color;
        }
        )");

    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    const ramses::Effect* effect = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache);
    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);
    ramses::Appearance* appearanceRed = scene->createAppearance(*effect);
    appearanceRed->setInputValue(colorInput, ramses::vec4f{ 1.f, 0.f, 0.f, 1.f });
    ramses::Appearance* appearanceWhite = scene->createAppearance(*effect);
    appearanceWhite->setInputValue(colorInput, ramses::vec4f{ 1.f, 1.f, 1.f, 1.f });
    appearanceWhite->setDepthFunction(ramses::EDepthFunc_Always);

    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    ramses::MeshNode* meshNode1 = scene->createMeshNode("triangle mesh node 1");
    meshNode1->setAppearance(*appearanceRed);
    meshNode1->setIndexCount(3);
    meshNode1->setGeometryBinding(*geometry);

    renderGroup->addMeshNode(*meshNode1);

    // In order to simplify this example we use a small viewport filled with color to simulate a "2D element"
    // which will track the 3D triangle. This could be replaced with a 2D text or any other 2D HUD element
    // in real world scenario.
    auto orthoCamera = scene->createOrthographicCamera();
    orthoCamera->setViewport(0, 0, 32, 32);
    orthoCamera->setFrustum(-1, 1, -1, 1, 0.1f, 10.f);

    ramses::MeshNode* meshNode2 = scene->createMeshNode("triangle mesh node 2");
    meshNode2->setAppearance(*appearanceWhite);
    meshNode2->setIndexCount(3);
    meshNode2->setGeometryBinding(*geometry);
    meshNode2->setTranslation({0.f, -5.f, -1.f});
    meshNode2->setScaling({10.f, 10.f, 1.f});

    ramses::RenderPass* renderPassOrtho = scene->createRenderPass();
    renderPassOrtho->setClearFlags(ramses::EClearFlags_None);
    renderPassOrtho->setCamera(*orthoCamera);
    ramses::RenderGroup* renderGroupOrtho = scene->createRenderGroup();
    renderPassOrtho->addRenderGroup(*renderGroupOrtho);
    renderPassOrtho->setRenderOrder(1);
    renderGroupOrtho->addMeshNode(*meshNode2);
    //

    scene->flush();
    scene->publish();

    return SceneAndNodes{ scene, meshNode1, camera, orthoCamera };
}

void CreateAnimationForNode(rlogic::LogicEngine& logicEngine, const rlogic::RamsesNodeBinding* nodeBinding)
{
    rlogic::DataArray* animTimestamps = logicEngine.createDataArray(std::vector<float>{ 0.f, 5.f, 10.f, 15.f, 20.f }); // will be interpreted as seconds
    rlogic::DataArray* animKeyframes = logicEngine.createDataArray(std::vector<rlogic::vec3f>{ {-3.f, -1.f, -1.f}, { 3.f, -1.f, -1.f }, { 3.f, 0.5f, -1.f }, { -3.f, 0.5f, -1.f }, { -3.f, -1.f, -1.f } });
    const rlogic::AnimationChannel animChannel{ "translation", animTimestamps, animKeyframes, rlogic::EInterpolationType::Linear };

    rlogic::AnimationNodeConfig animConfig;
    animConfig.addChannel(animChannel);
    rlogic::AnimationNode* animNode = logicEngine.createAnimationNode(animConfig);

    logicEngine.link(
        *animNode->getOutputs()->getChild("translation"),
        *nodeBinding->getInputs()->getChild("translation"));

    rlogic::LuaConfig scriptConfig;
    scriptConfig.addStandardModuleDependency(rlogic::EStandardModule::Math);
    rlogic::LuaScript* controlScript = logicEngine.createLuaScript(R"(
        function init()
            GLOBAL.startTick = 0
        end

        function interface(IN,OUT)
            IN.ticker = Type:Int64()
            IN.animDuration = Type:Float()
            OUT.animProgress = Type:Float()
        end

        function run(IN,OUT)
            if GLOBAL.startTick == 0 then
                GLOBAL.startTick = IN.ticker
            end

            local elapsedTime = IN.ticker - GLOBAL.startTick
            -- ticker from TimerNode is in microseconds, our animation duration is in seconds, conversion is needed
            elapsedTime = elapsedTime / 1000000

            local animProgress = elapsedTime / IN.animDuration
            OUT.animProgress = animProgress % 1
        end
    )", scriptConfig);

    rlogic::TimerNode* timer = logicEngine.createTimerNode();
    logicEngine.link(
        *timer->getOutputs()->getChild("ticker_us"),
        *controlScript->getInputs()->getChild("ticker"));

    controlScript->getInputs()->getChild("animDuration")->set(*animNode->getOutputs()->getChild("duration")->get<float>());

    logicEngine.link(
        *controlScript->getOutputs()->getChild("animProgress"),
        *animNode->getInputs()->getChild("progress"));
}
