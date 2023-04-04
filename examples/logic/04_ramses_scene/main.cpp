//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-logic/LogicEngine.h"
#include "ramses-logic/LuaScript.h"
#include "ramses-logic/RamsesNodeBinding.h"
#include "ramses-logic/Property.h"

#include "ramses-client.h"
#include "ramses-utils.h"

#include <cassert>
#include <iostream>
#include <thread>

/**
 * This example demonstrates how to link LogicEngine scripts and
 * Ramses scene content, so that whenever scripts are executed
 * their output is propagated to the linked Ramses scene(s).
 * This example only demonstrates code, for general explanations
 * and more context, check the doc pages at:
 * https://ramses-logic.readthedocs.io/en/latest/api.html#creating-links-between-scripts
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
     * Create Ramses framework and client objects. Ramses Logic does not manage
     * or encapsulate Ramses objects - it only interacts with existing Ramses objects.
     * The application must take special care to not destroy Ramses objects while a
     * LogicEngine instance is still referencing them!
     */
    ramses::RamsesFramework ramsesFramework;
    ramses::RamsesClient* ramsesClient = ramsesFramework.createClient("example client");

    /**
     * To keep this example simple, we don't include a Renderer, but only provide the scene
     * over network. Start a ramses daemon and a renderer additionally to see the visual result!
     * The connect() ensures the scene published in this example will be distributed over network
     */
    ramsesFramework.connect();

    /**
     * Create a test Ramses scene with just a single triangle
     */
    auto [scene, triangleNode] = CreateSceneWithTriangle(*ramsesClient);

    rlogic::LogicEngine logicEngine;

    /**
     * Create a binding object which serves as a bridge between logic scripts and animations on one end
     * and a Ramses scene on the other end
     */
    rlogic::RamsesNodeBinding* nodeBinding = logicEngine.createRamsesNodeBinding(*triangleNode, rlogic::ERotationType::Euler_XYZ, "binding to triangle mesh node");

    /**
     * Create a simple script which takes the current time in milliseconds
     * and rotates around the Z axis slowly based on how much time is passed
     */
    rlogic::LuaScript* script = logicEngine.createLuaScript(R"(
        function interface(IN,OUT)
            IN.time_msec = Type:Int32()
            OUT.rotationZ = Type:Vec3f()
        end

        function run(IN,OUT)
            -- Rotate around Z axis with 100 degrees per second
            OUT.rotationZ = {0, 0, IN.time_msec / 10}
        end
    )");

    /**
     * Connect the script output 'rotationZ' with the rotation property of the RamsesNodeBinding object.
     * After this, the value computed in the script will be propagated to the ramses node.
     */
    logicEngine.link(
        *script->getOutputs()->getChild("rotationZ"),
        *nodeBinding->getInputs()->getChild("rotation"));

    /**
     * Simulate an infinite animation and provide the time to the control script
     * so that it can use it to advance the rotation of the animated triangle.
     * Usually this would be replaced by an application loop and data.
     */
    for(int32_t simulatedTime = 0; simulatedTime < 100000; ++simulatedTime)
    {
        script->getInputs()->getChild("time_msec")->set<int32_t>(simulatedTime);

        /**
         * Update the LogicEngine. This will execute the script above, and will set the rotation of the
         * Ramses node bound to the output of the script.
         */
        logicEngine.update();

        /**
         * Tell ramses to "flush" the scene state and finish the frame.
         */
        scene->flush();

        /**
         * Throttle the "animation" by sleeping for a bit.
         */
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    /**
     * Destruction order is important! First destroy the logic engine content (created last) then the Ramses scene
     * (after nothing is referencing the scene any more)
     */
    logicEngine.destroy(*script);
    logicEngine.destroy(*nodeBinding);
    ramsesClient->destroy(*scene);

    return 0;
}


SceneAndNode CreateSceneWithTriangle(ramses::RamsesClient& client)
{
    ramses::Scene* scene = client.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "red triangle scene");

    ramses::PerspectiveCamera* camera = scene->createPerspectiveCamera();
    camera->setFrustum(19.0f, 1.0f, 0.1f, 100.0f);
    camera->setViewport(0, 0, 800, 800);
    camera->setTranslation(0.0f, 0.0f, 5.0f);
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

    renderGroup->addMeshNode(*meshNode);

    scene->flush();
    scene->publish();

    return SceneAndNode{ scene, meshNode };
}
