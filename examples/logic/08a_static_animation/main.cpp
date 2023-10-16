//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/LuaScript.h"
#include "ramses/client/logic/NodeBinding.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/AnimationNode.h"
#include "ramses/client/logic/AnimationNodeConfig.h"
#include "ramses/client/logic/AnimationTypes.h"
#include "ramses/client/logic/EPropertyType.h"
#include "ramses/client/logic/DataArray.h"
#include "ramses/client/logic/TimerNode.h"

#include "ramses/client/ramses-client.h"
#include "ramses/client/ramses-utils.h"

#include "SimpleRenderer.h"

#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

/**
* This example demonstrates how to use animation to animate
* Ramses scene content.
*/

struct SceneAndNodes
{
    ramses::Scene* scene;
    ramses::Node* node1;
    ramses::Node* node2;
};

/**
* Helper method which creates a simple ramses scene. For more ramses
* examples, check the ramses docs at https://bmwcarit.github.io/ramses
*/
SceneAndNodes CreateSceneWithTriangles(ramses::RamsesClient& client);

int main()
{
    /**
    * Use simple class to create ramses framework objects which are not essential for this example.
    * For more info on those, please refer to the ramses docs: https://bmwcarit.github.io/ramses
    */
    SimpleRenderer renderer;

    /**
     * Create a test Ramses scene with two simple triangles to be animated separately.
     */
    auto [scene, tri1, tri2] = CreateSceneWithTriangles(*renderer.getClient());

    ramses::LogicEngine& logicEngine{ *scene->createLogicEngine() };

    /**
    * Create a binding object which serves as a bridge between logic nodes and animations on one end
    * and a Ramses scene on the other end.
    */
    ramses::NodeBinding* nodeBinding1 = logicEngine.createNodeBinding(*tri1);
    ramses::NodeBinding* nodeBinding2 = logicEngine.createNodeBinding(*tri2);

    /**
     * Create two simple animations (cubic and linear) by providing keyframes and timestamps.
     * Animations have a single key-frame channel in this example for simplicity.
     *
     * First, create the data arrays which contain the time stamp data, the key-frame data points, and tangent arrays for the cubic animation.
     */
    ramses::DataArray* animTimestamps = logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f, 2.f, 4.f }); // will be interpreted as seconds
    ramses::DataArray* animKeyframes = logicEngine.createDataArray(std::vector<ramses::vec3f>{ {0.f, 0.f, 0.f}, {0.f, 0.f, 90.f}, {0.f, 0.f, 180.f}, {0.f, 0.f, 360.f} });
    ramses::DataArray* cubicAnimTangentsIn = logicEngine.createDataArray(std::vector<ramses::vec3f>{ { 0.f, 0.f, -300.f }, { 0.f, 0.f, 300.f }, { 0.f, 0.f, -300.f }, { 0.f, 0.f, -300.f } });
    ramses::DataArray* cubicAnimTangentsOut = logicEngine.createDataArray(std::vector<ramses::vec3f>{ { 0.f, 0.f, -300.f }, { 0.f, 0.f, -300.f }, { 0.f, 0.f, 300.f }, { 0.f, 0.f, 300.f } });

    /**
     * Create a channel for each animation - cubic and linear.
     */
    const ramses::AnimationChannel cubicAnimChannel { "rotationZcubic", animTimestamps, animKeyframes, ramses::EInterpolationType::Cubic, cubicAnimTangentsIn, cubicAnimTangentsOut };
    const ramses::AnimationChannel linearAnimChannel { "rotationZlinear", animTimestamps, animKeyframes, ramses::EInterpolationType::Linear };

    /**
     * Create the animation nodes by passing in the channel data via config
     */
    ramses::AnimationNodeConfig animConfigCubic;
    animConfigCubic.addChannel(cubicAnimChannel);
    ramses::AnimationNode* cubicAnimNode = logicEngine.createAnimationNode(animConfigCubic);

    ramses::AnimationNodeConfig animConfigLinear;
    animConfigLinear.addChannel(linearAnimChannel);
    ramses::AnimationNode* linearAnimNode = logicEngine.createAnimationNode(animConfigLinear);

    /**
    * Connect the animation channel 'rotationZ' output with the rotation property of the NodeBinding object.
    * After this, the value computed in the animation output channel will be propagated to the ramses node's rotation property.
    */
    logicEngine.link(
        *cubicAnimNode->getOutputs()->getChild("rotationZcubic"),
        *nodeBinding1->getInputs()->getChild("rotation"));
    logicEngine.link(
        *linearAnimNode->getOutputs()->getChild("rotationZlinear"),
        *nodeBinding2->getInputs()->getChild("rotation"));

    /**
    * Create control script which uses simple logic to control the animations' progress
    */
    ramses::LuaConfig scriptConfig;
    scriptConfig.addStandardModuleDependency(ramses::EStandardModule::Math);
    ramses::LuaScript* controlScript = logicEngine.createLuaScript(R"(
        function init()
            GLOBAL.startTick = 0
        end

        function interface(IN,OUT)
            IN.ticker = Type:Int64()
            IN.anim1Duration = Type:Float()
            IN.anim2Duration = Type:Float()

            OUT.anim1Progress = Type:Float()
            OUT.anim2Progress = Type:Float()
        end

        function run(IN,OUT)
            if GLOBAL.startTick == 0 then
                GLOBAL.startTick = IN.ticker
            end

            local elapsedTime = IN.ticker - GLOBAL.startTick
            -- ticker from TimerNode is in microseconds, our animation duration is in seconds, conversion is needed
            elapsedTime = elapsedTime / 1000000

            -- play anim1 right away
            local anim1Progress = elapsedTime / IN.anim1Duration
            -- play anim2 after anim1
            local anim2Progress = math.max(0, (elapsedTime - IN.anim1Duration) / IN.anim2Duration)

            -- modulo progress to loop animations
            OUT.anim1Progress = anim1Progress % 1
            OUT.anim2Progress = anim2Progress % 1
        end
    )", scriptConfig);

    /**
    * We need to provide time information to the control script, we can either provide system or custom time ticker from application
    * or we can create a TimerNode which generates system time for us. Note that its 'ticker_us' output is in microseconds, control script needs
    * to convert it to whatever units are used in the animation timestamps (in this example seconds).
    */
    ramses::TimerNode* timer = logicEngine.createTimerNode();
    logicEngine.link(
        *timer->getOutputs()->getChild("ticker_us"),
        *controlScript->getInputs()->getChild("ticker"));

    /**
    * Set duration of both animations to control script, so it can calculate and manage their progress
    * Note that we could also link these properties but as this would form a cycle in the dependency graph, it would have to be a weak link
    * (see #ramses::LogicEngine::linkWeak). We know that the durations will not change so setting them here once is sufficient.
    **/
    controlScript->getInputs()->getChild("anim1Duration")->set(*cubicAnimNode->getOutputs()->getChild("duration")->get<float>());
    controlScript->getInputs()->getChild("anim2Duration")->set(*linearAnimNode->getOutputs()->getChild("duration")->get<float>());

    /**
    * And finally, link control script to animation nodes
    **/
    logicEngine.link(
        *controlScript->getOutputs()->getChild("anim1Progress"),
        *cubicAnimNode->getInputs()->getChild("progress"));
    logicEngine.link(
        *controlScript->getOutputs()->getChild("anim2Progress"),
        *linearAnimNode->getInputs()->getChild("progress"));

    /**
    * Show the scene on the renderer
    */
    renderer.showScene(scene->getSceneId());

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

    /**
    * Ramses logic objects are managed and will be automatically released with destruction of the LogicEngine instance,
    * however it is good practice to destroy objects if they are not going to be needed anymore.
    * When destroying manually, keep order in mind, any logic content referencing a Ramses scene should be destroyed
    * before the scene. Similarly objects using DataArray instances (e.g. AnimationNodes) should be destroyed before
    * the data arrays. Generally objects referencing other objects should always be destroyed first.
    */
    logicEngine.destroy(*cubicAnimNode);
    logicEngine.destroy(*linearAnimNode);
    logicEngine.destroy(*animTimestamps);
    logicEngine.destroy(*animKeyframes);
    logicEngine.destroy(*cubicAnimTangentsIn);
    logicEngine.destroy(*cubicAnimTangentsOut);
    logicEngine.destroy(*nodeBinding1);
    logicEngine.destroy(*nodeBinding2);
    renderer.getClient()->destroy(*scene);

    return 0;
}


SceneAndNodes CreateSceneWithTriangles(ramses::RamsesClient& client)
{
    ramses::Scene* scene = client.createScene(ramses::sceneId_t(123u), "red triangle scene");

    ramses::PerspectiveCamera* camera = scene->createPerspectiveCamera();
    camera->setFrustum(19.0f, 1280.f/800.f, 0.1f, 100.0f);
    camera->setViewport(0, 0, 1280, 800);
    camera->setTranslation({0.0f, 0.0f, 10.0f});
    ramses::RenderPass* renderPass = scene->createRenderPass();
    renderPass->setClearFlags(ramses::EClearFlag::None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    std::array<ramses::vec3f, 3u> vertexPositionsArray{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{1.f, 0.f, -1.f}, ramses::vec3f{0.f, 1.f, -1.f} };
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

        void main(void)
        {
            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
        )");

    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    const ramses::Effect* effect = scene->createEffect(effectDesc);
    ramses::Appearance* appearance = scene->createAppearance(*effect);

    ramses::Geometry* geometry = scene->createGeometry(*effect);
    std::optional<ramses::AttributeInput> positionsInput = effect->findAttributeInput("a_position");
    assert(positionsInput.has_value());
    geometry->setInputBuffer(*positionsInput, *vertexPositions);

    ramses::MeshNode* meshNode1 = scene->createMeshNode("triangle mesh node 1");
    ramses::MeshNode* meshNode2 = scene->createMeshNode("triangle mesh node 2");
    meshNode1->setAppearance(*appearance);
    meshNode1->setIndexCount(3);
    meshNode1->setGeometry(*geometry);
    meshNode2->setAppearance(*appearance);
    meshNode2->setIndexCount(3);
    meshNode2->setGeometry(*geometry);

    meshNode1->setTranslation({-1.f, -0.8f, 0.f});
    meshNode2->setTranslation({1.f, -0.8f, 0.f});

    renderGroup->addMeshNode(*meshNode1);
    renderGroup->addMeshNode(*meshNode2);

    scene->flush();
    scene->publish();

    return SceneAndNodes{ scene, meshNode1, meshNode2 };
}
