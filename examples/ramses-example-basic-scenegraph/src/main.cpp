//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/ArrayResource.h"
#include "ramses-client-api/SceneGraphIterator.h"
#include "ramses-client-api/SceneObjectIterator.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/AttributeInput.h"
#include "ramses-client-api/Effect.h"

#include <stdio.h>
#include <thread>

/**
 * @example ramses-example-basic-scenegraph/src/main.cpp
 * @brief Basic Scene Graph Example
 */

int main(int argc, char* argv[])
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-basic-scenegraph"));
    framework.connect();

    // create a scene for distributing content
    ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "basic scenegraph scene");

    // every scene needs a render pass with camera
    auto* camera = scene->createPerspectiveCamera("my camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    camera->setTranslation(0.0f, 0.0f, 5.0f);
    ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsData[] = { -0.1f, 0.f, -0.1f, 0.1f, 0.f, -0.1f, 0.f, 0.1f, -0.1f };
    ramses::ArrayResource* vertexPositions = scene->createArrayResource(ramses::EDataType::Vector3F, 3, vertexPositionsData);
    uint16_t indexData[] = { 0, 1, 2 };
    ramses::ArrayResource* indices = scene->createArrayResource(ramses::EDataType::UInt16, 3, indexData);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-basic-scenegraph.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-scenegraph.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effect = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = scene->createAppearance(*effect, "triangle appearance");
    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect, "triangle geometry");

    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    // get input data of appearance and bind required data
    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);
    appearance->setInputValueVector4f(colorInput, 0.5f, 1.0f, 0.3f, 1.0f);

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
            meshNode->setGeometryBinding(*geometry);
            meshNode->setTranslation(column * 0.2f, row * 0.2f, 0.0f);
            meshNode->setParent(*group);
            // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
            renderGroup->addMeshNode(*meshNode);
        }
    }
    /// [Basic Scene Graph Example]

    // distribute the scene to RAMSES
    scene->publish();

    //example: how to traverse scene graph
    ramses::SceneGraphIterator graphIterator(*group, ramses::ETreeTraversalStyle_DepthFirst);
    ramses::Node* nextNode;
    printf("Scene graph traversed in depth first order: \n");
    while ((nextNode = graphIterator.getNext()) != nullptr)
    {
        printf("Node: %s \n", nextNode->getName());
    }

    //example: how to iterate through objects of scene
    ramses::SceneObjectIterator iter(*scene, ramses::ERamsesObjectType_MeshNode);
    int numberOfMeshes = 0;
    while (iter.getNext() != nullptr)
    {
        ++numberOfMeshes;
    }
    printf("Scene contains %i meshnodes\n", numberOfMeshes);

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
