//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/RamsesMeshNodeBinding.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/Property.h"
#include "ramses-logic/TimerNode.h"

#include "ramses-client.h"
#include "ramses-utils.h"

#include "SimpleRenderer.h"

#include <cassert>
#include <array>
#include <thread>
#include <cmath>

/**
* This example demonstrates how to use RamsesMeshNodeBinding in a simple scene.
*/

struct SceneAndMesh
{
    ramses::Scene* scene;
    ramses::MeshNode* mesh;
};

/**
* Helper method which creates a simple ramses scene. For more ramses
* examples, check the ramses docs at https://bmwcarit.github.io/ramses
*/
SceneAndMesh CreateSceneWithTriangles(ramses::RamsesClient& client);

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
    auto [scene, meshNode] = CreateSceneWithTriangles(*renderer.getClient());

    /**
    * Skin binding requires feature level 05 or higher.
    */
    rlogic::LogicEngine logicEngine{ rlogic::EFeatureLevel_05 };

    /**
    * Show the scene on the renderer.
    */
    renderer.showScene(scene->getSceneId());

    /**
    * Create control script which will output values meaningful for some of the RamsesMeshNodeBinding inputs.
    * This script takes in time ticker and adjusts mesh parameters so that it cycles through up to 3 instances
    * of it also cycles through subsets of vertices to be used to render it.
    */
    rlogic::LuaConfig scriptConfig;
    scriptConfig.addStandardModuleDependency(rlogic::EStandardModule::Math);
    rlogic::LuaScript* controlScript = logicEngine.createLuaScript(R"(
        function interface(IN,OUT)
            IN.ticker = Type:Int64()
            OUT.vertexOffset = Type:Int32()
            OUT.indexCount = Type:Int32()
            OUT.instanceCount = Type:Int32()
        end

        function run(IN,OUT)
            local tickerMs = IN.ticker / 1000
            OUT.vertexOffset = 4 - (math.floor(tickerMs / 333.333333) % 3) * 2
            OUT.indexCount = 8 - OUT.vertexOffset
            OUT.instanceCount = math.floor(tickerMs / 1000) % 3 + 1
        end
    )", scriptConfig);

    /**
    * As mentioned above the control script requires time ticker, here we create it and link it to the script.
    */
    rlogic::TimerNode* timer = logicEngine.createTimerNode();
    logicEngine.link(
        *timer->getOutputs()->getChild("ticker_us"),
        *controlScript->getInputs()->getChild("ticker"));

    /**
    * Finally we create the RamsesMeshNodeBinding which binds to our mesh.
    * Each control script output is linked to the corresponding RamsesMeshNodeBinding's input.
    */
    const auto meshNodeBinding = logicEngine.createRamsesMeshNodeBinding(*meshNode, "meshNodeBinding");
    logicEngine.link(
        *controlScript->getOutputs()->getChild("vertexOffset"),
        *meshNodeBinding->getInputs()->getChild("vertexOffset"));
    logicEngine.link(
        *controlScript->getOutputs()->getChild("indexCount"),
        *meshNodeBinding->getInputs()->getChild("indexCount"));
    logicEngine.link(
        *controlScript->getOutputs()->getChild("instanceCount"),
        *meshNodeBinding->getInputs()->getChild("instanceCount"));
    /**
     * Note that we do not link all the RamsesMeshNodeBinding input properties (e.g. 'indexOffset' is not used here),
     * meaning it will be initialized by Ramses and not modified in any way by Ramses logic.
     */

    /**
     * Simulate an application loop.
     */
    while (!renderer.isWindowClosed())
    {
        /**
        * Update the LogicEngine. This will apply changes to Ramses scene from any running script.
        */
        logicEngine.update();

        /**
        * In order to commit the changes to Ramses scene we need to "flush" them.
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

SceneAndMesh CreateSceneWithTriangles(ramses::RamsesClient& client)
{
    ramses::Scene* scene = client.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "skinning scene");

    ramses::PerspectiveCamera* camera = scene->createPerspectiveCamera();
    camera->setFrustum(19.0f, float(SimpleRenderer::GetDisplaySize()[0])/float(SimpleRenderer::GetDisplaySize()[1]), 0.1f, 100.0f);
    camera->setViewport(0, 0, SimpleRenderer::GetDisplaySize()[0], SimpleRenderer::GetDisplaySize()[1]);
    camera->setTranslation(0.0f, 1.0f, 10.0f);

    ramses::RenderPass* renderPass = scene->createRenderPass();
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    const std::array<ramses::vec3f, 8u> vertexPositionsArray{
        ramses::vec3f{ -0.5f, 0.0f, 0.f },
        ramses::vec3f{  0.5f, 0.0f, 0.f },
        ramses::vec3f{ -0.5f, 0.75f, 0.f },
        ramses::vec3f{  0.5f, 0.75f, 0.f },
        ramses::vec3f{ -0.5f, 1.5f, 0.f },
        ramses::vec3f{  0.5f, 1.5f, 0.f },
        ramses::vec3f{ -0.5f, 2.25f, 0.f },
        ramses::vec3f{  0.5f, 2.25f, 0.f } };
    const ramses::ArrayResource* vertexPositions = scene->createArrayResource(uint32_t(vertexPositionsArray.size()), vertexPositionsArray.data());

    const std::array<uint32_t, 8> indexArray = { 0, 1, 2, 3, 4, 5, 6, 7 };
    const ramses::ArrayResource* indices = scene->createArrayResource(uint32_t(indexArray.size()), indexArray.data());

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShader(R"(
        #version 300 es
        precision mediump float;

        in vec3 a_position;
        flat out int instanceID;

        uniform highp mat4 viewMatrix;
        uniform highp mat4 projMatrix;

        vec3 translations[3] = vec3[3] (
            vec3(-1.5, 0.0, 0.0),
            vec3( 0.0, 0.0, 0.0),
            vec3( 1.5, 0.0, 0.0));

        void main()
        {
            instanceID = gl_InstanceID;
            vec4 worldPosition = vec4(a_position + translations[instanceID], 1.0);
            gl_Position = projMatrix * viewMatrix * worldPosition;
        }
        )");

    effectDesc.setFragmentShader(R"(
        #version 300 es
        precision mediump float;

        flat in int instanceID;
        out vec4 fragColor;

        vec4 colors[3] = vec4[3] (
            vec4(0.8, 0.0, 0.0, 1.0),
            vec4(0.0, 0.8, 0.0, 1.0),
            vec4(0.0, 0.0, 0.8, 1.0));

        void main(void)
        {
            fragColor = colors[instanceID];
        }
        )");
    effectDesc.setUniformSemantic("viewMatrix", ramses::EEffectUniformSemantic::ViewMatrix);
    effectDesc.setUniformSemantic("projMatrix", ramses::EEffectUniformSemantic::ProjectionMatrix);

    const ramses::Effect* effect = scene->createEffect(effectDesc);
    ramses::Appearance* appearance = scene->createAppearance(*effect);
    appearance->setDrawMode(ramses::EDrawMode_TriangleStrip);

    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect);
    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    ramses::MeshNode* meshNode = scene->createMeshNode("mesh");
    meshNode->setAppearance(*appearance);
    meshNode->setIndexCount(uint32_t(indexArray.size()));
    meshNode->setGeometryBinding(*geometry);

    renderGroup->addMeshNode(*meshNode);

    scene->flush();
    scene->publish();

    return { scene, meshNode };
}
