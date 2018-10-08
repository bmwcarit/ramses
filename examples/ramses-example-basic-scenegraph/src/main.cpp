//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/RemoteCamera.h"
#include "ramses-client-api/RenderPass.h"
#include "ramses-client-api/RenderGroup.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/GeometryBinding.h"
#include "ramses-client-api/MeshNode.h"
#include "ramses-client-api/GroupNode.h"
#include "ramses-client-api/TranslateNode.h"
#include "ramses-client-api/EffectDescription.h"
#include "ramses-client-api/UInt16Array.h"
#include "ramses-client-api/Vector2fArray.h"
#include "ramses-client-api/Vector3fArray.h"
#include "ramses-client-api/Vector4fArray.h"
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
    ramses::RamsesClient ramses("ramses-example-basic-scenegraph", framework);
    framework.connect();

    // create a scene for distributing content
    ramses::Scene* scene = ramses.createScene(123u, ramses::SceneConfig(), "basic scenegraph scene");

    // every scene needs a render pass with camera
    ramses::TranslateNode* cameraTranslate = scene->createTranslateNode();
    cameraTranslate->setTranslation(0.0f, 0.0f, 5.0f);
    ramses::Camera* camera = scene->createRemoteCamera("my camera");
    camera->setParent(*cameraTranslate);
    ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsData[] = { -0.1f, 0.f, -0.1f, 0.1f, 0.f, -0.1f, 0.f, 0.1f, -0.1f };
    const ramses::Vector3fArray* vertexPositions = ramses.createConstVector3fArray(3, vertexPositionsData);
    uint16_t indexData[] = { 0, 1, 2 };
    const ramses::UInt16Array* indices = ramses.createConstUInt16Array(3, indexData);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-basic-scenegraph.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-scenegraph.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    ramses::Effect* effect = ramses.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
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

    // create group node as root
    ramses::GroupNode* group = scene->createGroupNode("group of triangles");

    // create grid of triangles
    for (int row = 0; row < 2; ++row)
    {
        for (int column = 0; column < 3; ++column)
        {
            // translate each mesh node
            ramses::TranslateNode* translation = scene->createTranslateNode("translate node");
            translation->setTranslation(column * 0.2f, row * 0.2f, 0.0f);
            translation->setParent(*group);

            // create a mesh node to define the triangle with chosen appearance
            ramses::MeshNode* meshNode = scene->createMeshNode("triangle mesh node");
            meshNode->setAppearance(*appearance);
            meshNode->setGeometryBinding(*geometry);
            meshNode->setParent(*translation);
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
    while ((nextNode = graphIterator.getNext()) != 0)
    {
        printf("Node: %s \n", nextNode->getName());
    }

    //example: how to iterate through objects of scene
    ramses::SceneObjectIterator iter(*scene, ramses::ERamsesObjectType_MeshNode);
    int numberOfMeshes = 0;
    while (iter.getNext() != 0)
    {
        ++numberOfMeshes;
    }
    printf("Scene contains %i meshnodes\n", numberOfMeshes);

    scene->flush();

    // application logic
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    ramses.destroy(*scene);
    ramses.destroy(*vertexPositions);
    ramses.destroy(*indices);
    framework.disconnect();

    return 0;
}
