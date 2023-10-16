//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/ramses-client.h"
#include "ramses/client/Scene.h"
#include "ramses/client/RenderPass.h"
#include "ramses/client/RenderGroup.h"
#include "ramses/client/Appearance.h"
#include "ramses/client/Geometry.h"
#include "ramses/client/MeshNode.h"
#include "ramses/client/EffectDescription.h"
#include "ramses/client/ArrayResource.h"
#include "ramses/client/SceneGraphIterator.h"
#include "ramses/client/SceneObjectIterator.h"
#include "ramses/client/UniformInput.h"
#include "ramses/client/AttributeInput.h"
#include "ramses/client/Effect.h"

#include <thread>
#include <iostream>

/**
 * @example ramses-example-basic-scenegraph/src/main.cpp
 * @brief Basic Scene Graph Example
 */

int main()
{
    // register at RAMSES daemon
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-basic-scenegraph"));
    framework.connect();

    // create a scene for distributing content
    const ramses::SceneConfig sceneConfig(ramses::sceneId_t{123}, ramses::EScenePublicationMode::LocalAndRemote);
    ramses::Scene* scene = ramses.createScene(sceneConfig, "basic scenegraph scene");

    // every scene needs a render pass with camera
    auto* camera = scene->createPerspectiveCamera("my camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    camera->setTranslation({0.0f, 0.0f, 5.0f});
    ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlag::None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    const std::array<ramses::vec3f, 3u> vertexPositionsData{ ramses::vec3f{-0.1f, 0.f, -0.1f}, ramses::vec3f{0.1f, 0.f, -0.1f}, ramses::vec3f{0.f, 0.1f, -0.1f} };
    ramses::ArrayResource* vertexPositions = scene->createArrayResource(3u, vertexPositionsData.data());
    const std::array<uint16_t, 3u> indexData{ 0, 1, 2 };
    ramses::ArrayResource* indices = scene->createArrayResource(3u, indexData.data());

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-basic-scenegraph.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-scenegraph.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effect = scene->createEffect(effectDesc, "glsl shader");
    ramses::Appearance* appearance = scene->createAppearance(*effect, "triangle appearance");
    ramses::Geometry* geometry = scene->createGeometry(*effect, "triangle geometry");

    geometry->setIndices(*indices);
    std::optional<ramses::AttributeInput> positionsInput = effect->findAttributeInput("a_position");
    assert(positionsInput.has_value());
    geometry->setInputBuffer(*positionsInput, *vertexPositions);

    // get input data of appearance and bind required data
    std::optional<ramses::UniformInput> colorInput = effect->findUniformInput("color");
    assert(colorInput.has_value());
    appearance->setInputValue(*colorInput, ramses::vec4f{ 0.5f, 1.0f, 0.3f, 1.0f });

    /// [Basic Scene Graph Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // create node as root
    ramses::Node* group = scene->createNode("group of triangles");

    // create grid of triangles
    for (int row = 0; row < 2; ++row)
    {
        for (int column = 0; column < 3; ++column)
        {
            // create a mesh node to define the triangle with chosen appearance
            ramses::MeshNode* meshNode = scene->createMeshNode("triangle mesh node");
            meshNode->setAppearance(*appearance);
            meshNode->setGeometry(*geometry);
            meshNode->setTranslation({static_cast<float>(column) * 0.2f, static_cast<float>(row) * 0.2f, 0.0f});
            meshNode->setParent(*group);
            // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
            renderGroup->addMeshNode(*meshNode);
        }
    }
    /// [Basic Scene Graph Example]

    // distribute the scene to RAMSES
    scene->publish(ramses::EScenePublicationMode::LocalAndRemote);

    //example: how to traverse scene graph
    ramses::SceneGraphIterator graphIterator(*group, ramses::ETreeTraversalStyle::DepthFirst);
    ramses::Node* nextNode = nullptr;
    std::cout <<  "Scene graph traversed in depth first order:" << std::endl;
    while ((nextNode = graphIterator.getNext()) != nullptr)
    {
        std::cout << "Node: " << nextNode->getName() << std::endl;
    }

    //example: how to iterate through objects of scene
    ramses::SceneObjectIterator iter(*scene, ramses::ERamsesObjectType::MeshNode);
    int numberOfMeshes = 0;
    while (iter.getNext() != nullptr)
    {
        ++numberOfMeshes;
    }
    std::cout <<  "Scene contains " << numberOfMeshes << " meshnodes" << std::endl;

    scene->flush();

    // application logic
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    scene->destroy(*vertexPositions);
    scene->destroy(*indices);
    ramses.destroy(*scene);
    framework.disconnect();

    return 0;
}
