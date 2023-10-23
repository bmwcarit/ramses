//  -------------------------------------------------------------------------
//  Copyright (C) 2022 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/RenderGroupBinding.h"
#include "ramses/client/logic/RenderGroupBindingElements.h"
#include "ramses/client/logic/Property.h"
#include "ramses/client/logic/EPropertyType.h"

#include "ramses/client/ramses-client.h"
#include "ramses/client/ramses-utils.h"

#include "SimpleRenderer.h"

#include <cassert>
#include <iostream>
#include <thread>
#include <cmath>

/**
* This example demonstrates how to use AnchorPoint to track
* Ramses scene content in 2D.
*/

struct SceneAndNodes
{
    ramses::Scene* scene;
    ramses::RenderGroup* renderGroup;
    ramses::MeshNode* triangle1;
    ramses::MeshNode* triangle2;
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
     * Create a simple Ramses scene with two alpha blended triangles.
     * We will control the render order of the triangles using the render group they are in.
     */
    auto [scene, renderGroup, triMesh1, triMesh2] = CreateSceneWithTriangles(*renderer.getClient());

    ramses::LogicEngine& logicEngine{ *scene->createLogicEngine() };

    /**
    * In order to create a render group binding we need to first specify what elements we want to expose for render order control,
    * in our case it is the two triangles.
    */
    ramses::RenderGroupBindingElements renderGroupElements;
    renderGroupElements.addElement(*triMesh1, "tri1");
    renderGroupElements.addElement(*triMesh2, "tri2");
    ramses::RenderGroupBinding* renderGroupBinding = logicEngine.createRenderGroupBinding(*renderGroup, renderGroupElements);

    /**
    * Show the scene on the renderer
    */
    renderer.showScene(scene->getSceneId());

    /**
     * Simulate an application loop.
     */
    uint32_t frameCounter = 0u;
    while (!renderer.isWindowClosed())
    {
        /**
        * Update the LogicEngine. This will apply changes to Ramses scene from any running animation.
        */
        logicEngine.update();

        /**
        * In this example we change the render order directly using input property API, we swap the render order of the two triangles
        * every few frames to show the difference. In real world application this would be managed by a more complex logic
        * (e.g. sorting based on distance to camera) in another logic node (e.g. LuaScript) that would link to these property inputs.
        */
        if (++frameCounter % 100 < 50)
        {
            renderGroupBinding->getInputs()->getChild("renderOrders")->getChild("tri1")->set(-1);
            renderGroupBinding->getInputs()->getChild("renderOrders")->getChild("tri2")->set(1);
        }
        else
        {
            renderGroupBinding->getInputs()->getChild("renderOrders")->getChild("tri1")->set(1);
            renderGroupBinding->getInputs()->getChild("renderOrders")->getChild("tri2")->set(-1);
        }

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
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }

    return 0;
}

SceneAndNodes CreateSceneWithTriangles(ramses::RamsesClient& client)
{
    ramses::Scene* scene = client.createScene(ramses::sceneId_t(123u), "red triangle scene");

    ramses::PerspectiveCamera* camera = scene->createPerspectiveCamera();
    camera->setFrustum(19.0f, float(SimpleRenderer::GetDisplaySize()[0])/float(SimpleRenderer::GetDisplaySize()[1]), 0.1f, 100.0f);
    camera->setViewport(0, 0, SimpleRenderer::GetDisplaySize()[0], SimpleRenderer::GetDisplaySize()[1]);
    camera->setTranslation({0.0f, 0.0f, 10.0f});

    ramses::RenderPass* renderPass = scene->createRenderPass();
    renderPass->setClearFlags(ramses::EClearFlag::None);
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

    const ramses::Effect* effect = scene->createEffect(effectDesc);
    std::optional<ramses::UniformInput> colorInput = effect->findUniformInput("color");
    assert(colorInput.has_value());
    ramses::Appearance* appearanceRed = scene->createAppearance(*effect);
    appearanceRed->setInputValue(*colorInput, ramses::vec4f{ 1.f, 0.f, 0.f, 0.5f });
    ramses::Appearance* appearanceWhite = scene->createAppearance(*effect);
    appearanceWhite->setInputValue(*colorInput, ramses::vec4f{ 1.f, 1.f, 1.f, 0.5f });

    // set up blending
    appearanceRed->setBlendingFactors(ramses::EBlendFactor::SrcAlpha, ramses::EBlendFactor::OneMinusSrcAlpha, ramses::EBlendFactor::One, ramses::EBlendFactor::Zero);
    appearanceRed->setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
    appearanceRed->setDepthFunction(ramses::EDepthFunc::Disabled);
    appearanceWhite->setBlendingFactors(ramses::EBlendFactor::SrcAlpha, ramses::EBlendFactor::OneMinusSrcAlpha, ramses::EBlendFactor::One, ramses::EBlendFactor::Zero);
    appearanceWhite->setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
    appearanceWhite->setDepthFunction(ramses::EDepthFunc::Disabled);

    ramses::Geometry* geometry = scene->createGeometry(*effect);
    std::optional<ramses::AttributeInput> positionsInput = effect->findAttributeInput("a_position");
    assert(positionsInput.has_value());
    geometry->setInputBuffer(*positionsInput, *vertexPositions);

    ramses::MeshNode* meshNode1 = scene->createMeshNode("triangle mesh node 1");
    meshNode1->setAppearance(*appearanceRed);
    meshNode1->setIndexCount(3);
    meshNode1->setGeometry(*geometry);
    renderGroup->addMeshNode(*meshNode1);

    ramses::MeshNode* meshNode2 = scene->createMeshNode("triangle mesh node 2");
    meshNode2->setAppearance(*appearanceWhite);
    meshNode2->setIndexCount(3);
    meshNode2->setGeometry(*geometry);
    meshNode2->setTranslation({0.5f, -0.2f, 0.7f});

    renderGroup->addMeshNode(*meshNode1);
    renderGroup->addMeshNode(*meshNode2);

    scene->flush();
    scene->publish();

    return SceneAndNodes{ scene, renderGroup, meshNode1, meshNode2 };
}
