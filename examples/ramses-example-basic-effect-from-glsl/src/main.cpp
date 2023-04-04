//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include <thread>

/**
* @example ramses-example-basic-effect-from-glsl/src/main.cpp
* @brief Basic GLSL Import Example
*/

int main()
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework;
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-basic-effect-from-glsl"));
    framework.connect();

    // create a scene for distributing content
    ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "basic glsl effect scene");

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
    const std::array<ramses::vec3f, 3u> vertexPositionsData{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{1.f, 0.f, -1.f}, ramses::vec3f{0.f, 1.f, -1.f} };
    ramses::ArrayResource* vertexPositions = scene->createArrayResource(3u, vertexPositionsData.data());
    const std::array<uint16_t, 3u> indexData{ 0, 1, 2 };
    ramses::ArrayResource* indices = scene->createArrayResource(3u, indexData.data());

    /// [Basic GLSL Import Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-basic-effect-from-glsl.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-effect-from-glsl.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    const ramses::Effect* effect = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = scene->createAppearance(*effect, "triangle appearance");

    // set vertex positions directly in geometry
    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect, "triangle geometry");
    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    ramses::UniformInput scaleAndShearInput;
    effect->findUniformInput("u_transformations", scaleAndShearInput);

    const ramses::vec4f scaleAndShearArrayData[2] = { ramses::vec4f{0.3f, 0.6f, 0.0f, 0.0f}, ramses::vec4f{0.3f, 0.6f, 0.0f, 0.0f} };
    appearance->setInputValue(scaleAndShearInput, 2, scaleAndShearArrayData);

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = scene->createMeshNode("triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);

    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);
    appearance->setInputValue(colorInput, ramses::vec4f{ 0.1f, 0.5f, 0.2f, 1.f });

    /// [Basic GLSL Import Example]

    // signal the scene it is in a state that can be rendered
    scene->flush();

    // distribute the scene to RAMSES
    scene->publish();

    // application logic
    std::this_thread::sleep_for(std::chrono::seconds(100));

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    scene->destroy(*vertexPositions);
    scene->destroy(*indices);
    ramses.destroy(*scene);
    framework.disconnect();

    return 0;
}
