//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include <cstdlib>
#include <thread>

/**
 * @example ramses-example-basic-blending/src/main.cpp
 * @brief Basic Blending Example
 */

int main(int argc, char* argv[])
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework{ ramses::RamsesFrameworkConfig{ argc, argv } };
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-basic-blending"));
    framework.connect();

    // create a scene for distributing content
    ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "triangles scene");

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
    float vertexPositionsData[] = { -1.f, 0.f, -1.f, 1.f, 0.f, -1.f, 0.f, 1.f, -1.f };
    ramses::ArrayResource* vertexPositions = scene->createArrayResource(ramses::EDataType::Vector3F, 3, vertexPositionsData);
    uint16_t indexData[] = {0, 1, 2};
    ramses::ArrayResource* indices = scene->createArrayResource(ramses::EDataType::UInt16, 3, indexData);

    // initialize effect
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-basic-blending.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-blending.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    const ramses::Effect* effect = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");

    // create an appearance for red triangle
    ramses::Appearance* appearanceRed = scene->createAppearance(*effect, "red triangle appearance");
    // create an appearance for green triangle
    ramses::Appearance* appearanceGreen = scene->createAppearance(*effect, "green triangle appearance");
    // create an appearance for blue triangle
    ramses::Appearance* appearanceBlue = scene->createAppearance(*effect, "blue triangle appearance");

    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect, "triangle geometry");
    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    // create mesh nodes to define the triangles with different appearances
    ramses::MeshNode* meshNodeRed = scene->createMeshNode("red triangle mesh node");
    ramses::MeshNode* meshNodeGreen = scene->createMeshNode("green triangle mesh node");
    ramses::MeshNode* meshNodeBlue = scene->createMeshNode("blue triangle mesh node");

    // offset triangles so that they are not fully overlapping

    meshNodeRed->setTranslation(0.f, -0.2f, -12.f);
    meshNodeGreen->setTranslation(-0.2f, 0.f, -11.f);
    meshNodeBlue->setTranslation(0.2f, 0.2f, -10.f);

    // get handle to appearances' input and set color with alpha smaller than 1
    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);
    appearanceRed->setInputValueVector4f(colorInput, 1.f, 0.f, 0.f, 0.9f);
    appearanceGreen->setInputValueVector4f(colorInput, 0.f, 1.f, 0.f, 0.6f);
    appearanceBlue->setInputValueVector4f(colorInput, 0.f, 0.f, 1.f, 0.3f);

    /// [Basic Blending Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // set blending states for alpha blending
    appearanceRed->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
    appearanceRed->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
    appearanceGreen->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
    appearanceGreen->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);
    appearanceBlue->setBlendingFactors(ramses::EBlendFactor_SrcAlpha, ramses::EBlendFactor_OneMinusSrcAlpha, ramses::EBlendFactor_One, ramses::EBlendFactor_One);
    appearanceBlue->setBlendingOperations(ramses::EBlendOperation_Add, ramses::EBlendOperation_Add);

    // set appearances to mesh nodes
    meshNodeRed->setAppearance(*appearanceRed);
    meshNodeGreen->setAppearance(*appearanceGreen);
    meshNodeBlue->setAppearance(*appearanceBlue);

    // set same geometry to all mesh nodes
    meshNodeRed->setGeometryBinding(*geometry);
    meshNodeGreen->setGeometryBinding(*geometry);
    meshNodeBlue->setGeometryBinding(*geometry);

    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    // set render order so that triangles are rendered back to front
    renderGroup->addMeshNode(*meshNodeRed, 0);
    renderGroup->addMeshNode(*meshNodeGreen, 1);
    renderGroup->addMeshNode(*meshNodeBlue, 2);
    /// [Basic Blending Example]

    // signal the scene it is in a state that can be rendered
    scene->flush();

    // distribute the scene to RAMSES
    scene->publish();

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
