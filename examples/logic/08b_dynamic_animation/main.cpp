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

#include "ramses-client.h"
#include "ramses-utils.h"

#include "SimpleRenderer.h"

#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

/**
* This example demonstrates an 'animate to' functionality using a 'dynamic' AnimationNode.
* Dynamic animation node is a variant of AnimationNode which allows to change the animation keyframes in runtime. An 'animate to' functionality
* refers to a logic which is able to smoothly animate to any given value at any point in time, even if already in middle of animation to another value.
* This examples uses only 2 keyframes animation to achieve the 'animate to' functionality, this should be sufficient even for real world
* use cases but can be arbitrarily extended to more complex setups if needed.
*/

struct SceneAndNode
{
    ramses::Scene* scene;
    ramses::Node* node;
};

/**
* Helper method which creates a simple ramses scene. For more ramses
* examples, check the ramses docs at https://bmwcarit.github.io/ramses
*/
SceneAndNode CreateSceneWithTriangle(ramses::RamsesClient& client);

int main()
{
    /**
     * Use simple class to create ramses framework objects which are not essential for this example.
     * For more info on those, please refer to the ramses docs: https://bmwcarit.github.io/ramses
     */
    SimpleRenderer renderer;

    /**
     * Create a simple Ramses scene with a triangle.
     */
    auto [scene, tri] = CreateSceneWithTriangle(*renderer.getClient());

    rlogic::LogicEngine logicEngine;

    /**
    * Create a binding object which serves as a bridge between logic nodes and animations on one end
    * and a Ramses scene on the other end.
    */
    rlogic::RamsesNodeBinding* nodeBinding = logicEngine.createRamsesNodeBinding(*tri);

    /**
     * Create data arrays which contain the time stamp data and the keyframe data points.
     * Unlike with static animations the actual keyframe values will be set and dynamically changed from control script.
     */
    rlogic::DataArray* animTimestamps = logicEngine.createDataArray(std::vector<float>{ 0.f, 1.f });
    rlogic::DataArray* animKeyframes = logicEngine.createDataArray(std::vector<rlogic::vec3f>{ {0.f, 0.f, 0.f}, {1.f, 0.f, 0.f} });

    /**
     * Create a channel for animation.
     */
    const rlogic::AnimationChannel animChannel { "translation", animTimestamps, animKeyframes, rlogic::EInterpolationType::Linear };

    /**
     * Create a 'dynamic' animation node by passing in the channel data via config
     */
    rlogic::AnimationNodeConfig animConfig;
    animConfig.addChannel(animChannel);
    /// In order to implement 'animateTo' logic with dynamic changes of keyframes we need to create AnimationNode that allows
    /// modifications of animation data
    animConfig.setExposingOfChannelDataAsProperties(true);
    rlogic::AnimationNode* animNode = logicEngine.createAnimationNode(animConfig);

    /**
    * Connect the animation channel 'translation' output with the translation property of the RamsesNodeBinding object.
    * After this, the value computed in the animation output channel will be propagated to the ramses node's rotation property.
    */
    logicEngine.link(
        *animNode->getOutputs()->getChild("translation"),
        *nodeBinding->getInputs()->getChild("translation"));

    /**
    * Create an 'animate to' control script which will control not only the animation progress but also modify animation data when needed
    */
    rlogic::LuaConfig scriptConfig;
    scriptConfig.addStandardModuleDependency(rlogic::EStandardModule::Math);
    rlogic::LuaScript* controlScript = logicEngine.createLuaScript(R"(
        function init()
            GLOBAL.startTick = nil
            GLOBAL.currentDirection = 0
        end

        function interface(IN,OUT)
            IN.ticker = Type:Int64()
            IN.animationDirection = Type:Int32()    -- represents animateTo target, -1 for left, 1 for right direction in X axis translation animation
            IN.currentAnimationValue = Type:Vec3f() -- current animation value output (see below how it is used and how to get the value)

            OUT.animProgress = Type:Float()
            OUT.keyframeFrom = Type:Vec3f()  -- keyframe value to animate from (will be linked to animation node's data)
            OUT.keyframeTo = Type:Vec3f()    -- keyframe value to animate to (will be linked to animation node's data)
        end

        function run(IN,OUT)
            -- initialize internal and some output values
            if GLOBAL.startTick == nil then
                GLOBAL.startTick = IN.ticker
                OUT.keyframeFrom = {0, 0, 0}
                OUT.keyframeTo = {0, 0, 0}
            end

            -- core 'animateTo' logic, check if animation target changed
            -- (in this simplified case represented as 'direction' of triangle movement)
            if IN.animationDirection ~= GLOBAL.currentDirection then
                -- reset ticker to now (we reset animation and start from beginning
                -- (this is just one possible solution how to handle time in an animateTo script)
                GLOBAL.startTick = IN.ticker
                -- reset source keyframe to current value, this is crucial so that the animation continues
                -- where it was to new target (otherwise it would 'jump' to initial starting keyframe)
                OUT.keyframeFrom = IN.currentAnimationValue
                -- set new animation target (in this example we just animate to left or right edge of screen
                -- which is defined by X translation axis)
                OUT.keyframeTo = {IN.animationDirection, 0, 0}

                GLOBAL.currentDirection = IN.animationDirection
            end

            local elapsedTime = IN.ticker - GLOBAL.startTick
            -- ticker from TimerNode is in microseconds, our animation duration is in seconds, conversion is needed
            elapsedTime = elapsedTime / 1000000

            local duration = 5   -- means 5 seconds for animation to reach target value, adjust to make animation slower/faster
            local animProgress = elapsedTime / duration

            -- clamp normalized progress
            OUT.animProgress = math.min(math.max(animProgress, 0), 1)
        end
    )", scriptConfig);

    /**
    * We need to provide time information to the control script, we can either provide system or custom time ticker from application
    * or we can create a TimerNode which generates system time for us. Note that its 'ticker_us' output is in microseconds, control script needs
    * to convert it to whatever units are used in the animation timestamps (in this example seconds).
    */
    rlogic::TimerNode* timer = logicEngine.createTimerNode();
    logicEngine.link(
        *timer->getOutputs()->getChild("ticker_us"),
        *controlScript->getInputs()->getChild("ticker"));

    /**
    * Whenever changing a direction of the triangle animation the script logic needs to reset several values (see script code above),
    * one of them (keyframeFrom) depends on the current (or rather last produced) value of animation output.
    * Note that the default data flow in this example logic network is control script -> animation node -> binding/meshnode. In order to get
    * the last animation output we need to create a link which goes against this data flow and creates a loop in logic network dependency graph, this is not allowed
    * unless we use a 'weak' link, which this example is a perfect use case for. See rlogic::LogicEngine::linkWeak for more details and limitations.
    */
    logicEngine.linkWeak(
        *animNode->getOutputs()->getChild("translation"),
        *controlScript->getInputs()->getChild("currentAnimationValue"));

    /**
    * Link animation data from control script to animation node so that control script can dynamically change keyframes of the animation
    **/
    logicEngine.link(
        *controlScript->getOutputs()->getChild("keyframeFrom"),
        *animNode->getInputs()->getChild("channelsData")->getChild("translation")->getChild("keyframes")->getChild(0u)); // value of first keyframe (animate from)
    logicEngine.link(
        *controlScript->getOutputs()->getChild("keyframeTo"),
        *animNode->getInputs()->getChild("channelsData")->getChild("translation")->getChild("keyframes")->getChild(1u)); // value of second keyframe (animate to)

    /**
    * And finally, link control script to animation node to control its progress
    **/
    logicEngine.link(
        *controlScript->getOutputs()->getChild("animProgress"),
        *animNode->getInputs()->getChild("progress"));

    /**
    * Show the scene on the renderer
    */
    renderer.showScene(scene->getSceneId());

    /**
     * Simulate an application loop.
     */
    int nextChangeLoop = 0;
    int numChanges = 0;
    int loop = 0;
    while (!renderer.isWindowClosed())
    {
        // change direction every now and then
        if (loop == nextChangeLoop)
        {
            int32_t currentDirection = *controlScript->getInputs()->getChild("animationDirection")->get<int32_t>();
            if (currentDirection == 0)
                currentDirection = -1;
            controlScript->getInputs()->getChild("animationDirection")->set(-currentDirection);

            nextChangeLoop = loop + std::vector<int>{ 150, 100, 80, 120, 50 }[numChanges % 5];
            numChanges++;
        }

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
        std::this_thread::sleep_for(std::chrono::milliseconds(5));

        loop = (loop + 1) % 1000;
    }

    return 0;
}


SceneAndNode CreateSceneWithTriangle(ramses::RamsesClient& client)
{
    ramses::Scene* scene = client.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "triangle scene");

    ramses::PerspectiveCamera* camera = scene->createPerspectiveCamera();
    camera->setFrustum(19.0f, 1280.f/480.f, 0.1f, 100.0f);
    camera->setViewport(0, 0, 1280, 480);
    camera->setTranslation(0.0f, 0.0f, 15.0f);
    ramses::RenderPass* renderPass = scene->createRenderPass();
    renderPass->setClearFlags(ramses::EClearFlags_None);
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

    const ramses::Effect* effect = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache);
    ramses::Appearance* appearance = scene->createAppearance(*effect);

    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    ramses::MeshNode* meshNode = scene->createMeshNode("triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setIndexCount(3);
    meshNode->setGeometryBinding(*geometry);
    meshNode->setScaling(0.05f, 0.05f, 0.05f); // adjust triangle size
    renderGroup->addMeshNode(*meshNode);

    auto parentTranslationNode = scene->createNode();
    meshNode->setParent(*parentTranslationNode);

    // Scale scene so that translating node [-1, 1] roughly matches left/right edge of screen
    // If using other than default ramses standalone renderer display adjust this scaling
    // or camera transformation/frustum.
    auto parentScaleNode = scene->createNode();
    parentTranslationNode->setParent(*parentScaleNode);
    parentScaleNode->setScaling(6.f, 6.f, 6.f);

    scene->flush();
    scene->publish();

    return SceneAndNode{ scene, parentTranslationNode };
}
