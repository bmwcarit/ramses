//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include <thread>

/**
 * @example ramses-example-basic-renderpasses/src/main.cpp
 * @brief Basic Renderpasses Example
 */

int main(int argc, char* argv[])
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-basic-renderpasses"));
    framework.connect();

    // create a scene
    ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "basic renderpasses scene");

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsArray[] = { -0.5f, 0.f, -1.f, 0.5f, 0.f, -1.f, -0.5f, 1.f, -1.f, 0.5f, 1.f, -1.f };
    ramses::ArrayResource* vertexPositions = scene->createArrayResource(ramses::EDataType::Vector3F, 4, vertexPositionsArray);

    uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
    ramses::ArrayResource* indices = scene->createArrayResource(ramses::EDataType::UInt16, 6, indicesArray);

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-basic-renderpasses.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-renderpasses.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    ramses::Effect* effectTex = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");

    /// [Basic Renderpasses Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // every render pass needs a camera to define rendering parameters
    // create camera with perspective projection
    ramses::Node* cameraTranslate = scene->createNode();
    cameraTranslate->setTranslation(0.0f, 0.0f, 5.0f);
    ramses::PerspectiveCamera* cameraA = scene->createPerspectiveCamera("perspective camera of renderpass A");
    cameraA->setParent(*cameraTranslate);
    cameraA->setFrustum(45.f, 640.f / 480.f, 0.1f, 100.f);
    // use left side of the viewport
    cameraA->setViewport(0u, 0u, 640u, 480u);

    // create camera with orthographic projection
    ramses::OrthographicCamera* cameraB = scene->createOrthographicCamera("ortho camera of renderpass B");
    cameraB->setParent(*cameraTranslate);
    cameraB->setFrustum(-2.f, 2.f, -2.f, 2.f, 0.1f, 100.f);
    // use right side of the viewport
    cameraB->setViewport(640u, 0u, 640u, 480u);

    // create two renderpasses
    ramses::RenderPass* renderPassA = scene->createRenderPass("renderpass A");
    ramses::RenderPass* renderPassB = scene->createRenderPass("renderpass B");
    renderPassA->setClearFlags(ramses::EClearFlags_None);
    renderPassB->setClearFlags(ramses::EClearFlags_None);

    // set valid cameras for the passes
    renderPassA->setCamera(*cameraA);
    renderPassB->setCamera(*cameraB);

    // create render group for each renderpass
    ramses::RenderGroup* renderGroupA = scene->createRenderGroup("rendergroup A");
    ramses::RenderGroup* renderGroupB = scene->createRenderGroup("rendergroup B");
    renderPassA->addRenderGroup(*renderGroupA);
    renderPassB->addRenderGroup(*renderGroupB);

    ramses::Appearance* appearanceA = scene->createAppearance(*effectTex, "triangle appearance");
    ramses::Appearance* appearanceB = scene->createAppearance(*effectTex, "triangle appearance");

    // set vertex positions directly in geometry
    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effectTex, "triangle geometry");
    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effectTex->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    // use two appearances, each with a different color for distinguishing the meshes
    ramses::UniformInput color;
    effectTex->findUniformInput("color", color);
    appearanceA->setInputValueVector4f(color, 1.0f,0.0f,0.0f,1.0f);
    appearanceB->setInputValueVector4f(color, 0.0f,1.0f,0.0f,1.0f);

    ramses::MeshNode* meshNodeA = scene->createMeshNode("red triangle mesh node");
    meshNodeA->setAppearance(*appearanceA);
    meshNodeA->setGeometryBinding(*geometry);

    ramses::MeshNode* meshNodeB = scene->createMeshNode("green triangle mesh node");
    meshNodeB->setAppearance(*appearanceB);
    meshNodeB->setGeometryBinding(*geometry);

    // add each mesh to one pass
    renderGroupA->addMeshNode(*meshNodeA);
    renderGroupB->addMeshNode(*meshNodeB);

    /// [Basic Renderpasses Example]

    // signal the scene it is in a state that can be rendered
    scene->flush();

    scene->publish();
    // distribute the scene to RAMSES
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // application logic

    uint32_t loops = 10000;

    while (--loops)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    scene->destroy(*vertexPositions);
    scene->destroy(*indices);
    ramses.destroy(*scene);
    framework.disconnect();

    return 0;
}
