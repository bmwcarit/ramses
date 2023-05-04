//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/SkinBinding.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/AnimationNode.h"
#include "ramses-logic/AnimationNodeConfig.h"
#include "ramses-logic/AnimationTypes.h"
#include "ramses-logic/TimerNode.h"

#include "ramses-client.h"
#include "ramses-utils.h"

#include "SimpleRenderer.h"

#include <cassert>
#include <array>
#include <thread>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

/**
* This example demonstrates how to use SkinBinding for basic bone animation.
* This example is inspired by the GLTF skin example
* https://github.com/KhronosGroup/glTF-Tutorials/blob/master/gltfTutorial/gltfTutorial_019_SimpleSkin.md
* It is recommended to go through the GLTF example first (or in parallel) and then see how to implement
* the same result in Ramses Logic.
*/

struct SceneAndAppearance
{
    ramses::Scene* scene;
    ramses::Appearance* appearance;
};

/**
* Helper method which creates a simple ramses scene. For more ramses
* examples, check the ramses docs at https://bmwcarit.github.io/ramses
*/
SceneAndAppearance CreateSceneWithSkinnableMesh(ramses::RamsesClient& client);

/**
* Helper method which sets up simple animation for a skeleton joint node.
* Details on how to set up animations are covered in the animation example.
*/
void SetupJointAnimation(const ramses::RamsesNodeBinding& node, ramses::LogicEngine& logicEngine);

int main()
{
    /**
    * Use simple class to create ramses framework objects which are not essential for this example.
    * For more info on those, please refer to the ramses docs: https://bmwcarit.github.io/ramses
    */
    SimpleRenderer renderer;

    /**
     * Create a simple Ramses scene with a simple mesh.
     * We will then apply vertex skinning on this mesh.
     */
    auto [scene, appearance] = CreateSceneWithSkinnableMesh(*renderer.getClient());

    ramses::LogicEngine logicEngine{ ramses::EFeatureLevel_Latest };

    /**
    * Show the scene on the renderer
    */
    renderer.showScene(scene->getSceneId());

    /**
    * First create skeleton joints, each skeleton joint is represented by a Ramses node and a node binding.
    * Joint 2 is slightly offset in Y axis so that both joints form a simple 2-bone skeleton.
    */
    auto skeletonJoint1 = scene->createNode();
    auto skeletonJoint2 = scene->createNode();
    skeletonJoint2->setTranslation({0.f, 1.f, 0.f});
    const auto skeletonJointBinding1 = logicEngine.createRamsesNodeBinding(*skeletonJoint1);
    const auto skeletonJointBinding2 = logicEngine.createRamsesNodeBinding(*skeletonJoint2);

    /**
    * Set up a simple rotation animation for joint 2.
    * Details on how to set up animations are covered in the animation example.
    */
    SetupJointAnimation(*skeletonJointBinding2, logicEngine);

    /**
    * Prepare inverse binding matrices, we will need an inverse matrix for each joint.
    * These are needed for skinning calculations, refer to #ramses::SkinBinding for details.
    * Inverse binding matrices often come with asset data but here we utilize Ramses
    * to calculate them for us and then convert to a data container suited for later use.
    */
    ramses::matrix44f inverseBindMatrix1{};
    ramses::matrix44f inverseBindMatrix2{};
    skeletonJoint1->getInverseModelMatrix(inverseBindMatrix1);
    skeletonJoint2->getInverseModelMatrix(inverseBindMatrix2);

    /**
    * Now prepare the inputs for #ramses::SkinBinding creation, we will need:
    *  - list of joints in form of node bindings
    *  - list of inverse binding matrices
    *  - appearance binding of the appearance used to render the mesh
    *  - uniform input of the appearance where joint matrices are expected
    */
    const std::vector<const ramses::RamsesNodeBinding*> skinBindingJoints = {
        skeletonJointBinding1,
        skeletonJointBinding2 };
    const std::vector<ramses::matrix44f> skinBindingInverseBindMatrices = {
        inverseBindMatrix1,
        inverseBindMatrix2 };
    ramses::RamsesAppearanceBinding* appearanceBinding = logicEngine.createRamsesAppearanceBinding(*appearance);

    ramses::UniformInput jointMatUniform;
    appearance->getEffect().findUniformInput("u_jointMat", jointMatUniform);

    /**
    * Finally create instance of skin binding using all the data.
    */
    logicEngine.createSkinBinding(
        skinBindingJoints,
        skinBindingInverseBindMatrices,
        *appearanceBinding,
        jointMatUniform);

    /**
    * Note that after this point there is no application logic needed, all the steps needed for skinning happen
    * automatically within the skin binding as central place to bind all the needed data.
    * On every update animation will rotate one of the skeleton joints, the joint's transformation affects
    * the calculation of the joint matrix which is then used in the vertex shader.
    */

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

    return 0;
}

SceneAndAppearance CreateSceneWithSkinnableMesh(ramses::RamsesClient& client)
{
    ramses::Scene* scene = client.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "skinning scene");

    ramses::PerspectiveCamera* camera = scene->createPerspectiveCamera();
    camera->setFrustum(19.0f, float(SimpleRenderer::GetDisplaySize()[0])/float(SimpleRenderer::GetDisplaySize()[1]), 0.1f, 100.0f);
    camera->setViewport(0, 0, SimpleRenderer::GetDisplaySize()[0], SimpleRenderer::GetDisplaySize()[1]);
    camera->setTranslation({0.0f, 1.0f, 10.0f});

    ramses::RenderPass* renderPass = scene->createRenderPass();
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    const std::array<ramses::vec3f, 10u> vertexPositionsArray{
        ramses::vec3f{ -0.5f, 0.0f, 0.f },
        ramses::vec3f{  0.5f, 0.0f, 0.f },
        ramses::vec3f{ -0.5f, 0.5f, 0.f },
        ramses::vec3f{  0.5f, 0.5f, 0.f },
        ramses::vec3f{ -0.5f, 1.0f, 0.f },
        ramses::vec3f{  0.5f, 1.0f, 0.f },
        ramses::vec3f{ -0.5f, 1.5f, 0.f },
        ramses::vec3f{  0.5f, 1.5f, 0.f },
        ramses::vec3f{ -0.5f, 2.0f, 0.f },
        ramses::vec3f{  0.5f, 2.0f, 0.f } };
    const ramses::ArrayResource* vertexPositions = scene->createArrayResource(uint32_t(vertexPositionsArray.size()), vertexPositionsArray.data());

    const std::array<uint32_t, 24> indexArray = {
        0, 1, 3,
        0, 3, 2,
        2, 3, 5,
        2, 5, 4,
        4, 5, 7,
        4, 7, 6,
        6, 7, 9,
        6, 9, 8 };
    const ramses::ArrayResource* indices = scene->createArrayResource(uint32_t(indexArray.size()), indexArray.data());

    // these represent the indices of joints that each vertex is affected by
    const std::array<ramses::vec4f, 10u> jointIndexArray{
        ramses::vec4f{ 0.f, 0.f, 0.f, 0.f },  // vtx 0 is affected by joint 0 only
        ramses::vec4f{ 0.f, 0.f, 0.f, 0.f },
        ramses::vec4f{ 0.f, 1.f, 0.f, 0.f },
        ramses::vec4f{ 0.f, 1.f, 0.f, 0.f },
        ramses::vec4f{ 0.f, 1.f, 0.f, 0.f },  // vtx 4 is affected by joint 0 and 1
        ramses::vec4f{ 0.f, 1.f, 0.f, 0.f },
        ramses::vec4f{ 0.f, 1.f, 0.f, 0.f },
        ramses::vec4f{ 0.f, 1.f, 0.f, 0.f },
        ramses::vec4f{ 0.f, 1.f, 0.f, 0.f },
        ramses::vec4f{ 0.f, 1.f, 0.f, 0.f } };
    const ramses::ArrayResource* jointIndices = scene->createArrayResource(uint32_t(jointIndexArray.size()), jointIndexArray.data());

    // these represent the weights of joints that each vertex is affected by
    const std::array<ramses::vec4f, 10u> weightArray{
        ramses::vec4f{ 1.00f,  0.00f,  0.f, 0.f },  // vtx 0 is affected 100% by joint 0
        ramses::vec4f{ 1.00f,  0.00f,  0.f, 0.f },
        ramses::vec4f{ 0.75f,  0.25f,  0.f, 0.f },  // vtx 2 is affected 75% by joint 0 and 25% by joint 1
        ramses::vec4f{ 0.75f,  0.25f,  0.f, 0.f },
        ramses::vec4f{ 0.50f,  0.50f,  0.f, 0.f }, // ... and so on
        ramses::vec4f{ 0.50f,  0.50f,  0.f, 0.f },
        ramses::vec4f{ 0.25f,  0.75f,  0.f, 0.f },
        ramses::vec4f{ 0.25f,  0.75f,  0.f, 0.f },
        ramses::vec4f{ 0.00f,  1.00f,  0.f, 0.f },
        ramses::vec4f{ 0.00f,  1.00f,  0.f, 0.f } };
    const ramses::ArrayResource* weights = scene->createArrayResource(uint32_t(weightArray.size()), weightArray.data());

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShader(R"(
        #version 100
        attribute vec3 a_position;
        attribute vec4 a_weight;
        attribute vec4 a_joint;

        uniform highp mat4 viewMatrix;
        uniform highp mat4 projMatrix;
        uniform highp mat4 u_jointMat[2];

        void main()
        {
            mat4 skinMat =
                a_weight.x * u_jointMat[int(a_joint.x)] +
                a_weight.y * u_jointMat[int(a_joint.y)] +
                a_weight.z * u_jointMat[int(a_joint.z)] +
                a_weight.w * u_jointMat[int(a_joint.w)];
            vec4 worldPosition = skinMat * vec4(a_position, 1.0);
            gl_Position = projMatrix * viewMatrix * worldPosition;
        }
        )");

    effectDesc.setFragmentShader(R"(
        #version 100
        void main(void)
        {
            gl_FragColor = vec4(1.0, 0.6, 0.0, 1.0);
        }
        )");
    effectDesc.setUniformSemantic("viewMatrix", ramses::EEffectUniformSemantic::ViewMatrix);
    effectDesc.setUniformSemantic("projMatrix", ramses::EEffectUniformSemantic::ProjectionMatrix);

    const ramses::Effect* effect = scene->createEffect(effectDesc);
    ramses::Appearance* appearance = scene->createAppearance(*effect);

    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect);
    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);
    ramses::AttributeInput weightsInput;
    effect->findAttributeInput("a_weight", weightsInput);
    geometry->setInputBuffer(weightsInput, *weights);
    ramses::AttributeInput jointIndicesInput;
    effect->findAttributeInput("a_joint", jointIndicesInput);
    geometry->setInputBuffer(jointIndicesInput, *jointIndices);

    ramses::MeshNode* meshNode = scene->createMeshNode("mesh");
    meshNode->setAppearance(*appearance);
    meshNode->setIndexCount(uint32_t(indexArray.size()));
    meshNode->setGeometryBinding(*geometry);

    renderGroup->addMeshNode(*meshNode);

    scene->flush();
    scene->publish();

    return { scene, appearance };
}

void SetupJointAnimation(const ramses::RamsesNodeBinding& node, ramses::LogicEngine& logicEngine)
{
    ramses::DataArray* animTimestamps = logicEngine.createDataArray(std::vector<float>{ 0.f, 2.f, 4.f }); // will be interpreted as seconds
    ramses::DataArray* animKeyframes = logicEngine.createDataArray(std::vector<ramses::vec3f>{ { 0.f, 0.f, -90.f }, { 0.f, 0.f, 90.f }, { 0.f, 0.f, -90.f } });
    const ramses::AnimationChannel animChannel{ "rotation", animTimestamps, animKeyframes, ramses::EInterpolationType::Linear };

    ramses::AnimationNodeConfig animConfig;
    animConfig.addChannel(animChannel);
    ramses::AnimationNode* animNode = logicEngine.createAnimationNode(animConfig);

    logicEngine.link(
        *animNode->getOutputs()->getChild("rotation"),
        *node.getInputs()->getChild("rotation"));

    ramses::LuaConfig scriptConfig;
    scriptConfig.addStandardModuleDependency(ramses::EStandardModule::Math);
    ramses::LuaScript* controlScript = logicEngine.createLuaScript(R"(
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

    ramses::TimerNode* timer = logicEngine.createTimerNode();
    logicEngine.link(
        *timer->getOutputs()->getChild("ticker_us"),
        *controlScript->getInputs()->getChild("ticker"));

    controlScript->getInputs()->getChild("animDuration")->set(*animNode->getOutputs()->getChild("duration")->get<float>());

    logicEngine.link(
        *controlScript->getOutputs()->getChild("animProgress"),
        *animNode->getInputs()->getChild("progress"));
}
