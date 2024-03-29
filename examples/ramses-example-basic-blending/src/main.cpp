//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/ramses-client.h"

#include <cstdlib>
#include <thread>

/**
 * @example ramses-example-basic-blending/src/main.cpp
 * @brief Basic Blending Example
 */

int main()
{
    // register at RAMSES daemon
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-basic-blending"));
    framework.connect();

    // create a scene for distributing content
    const ramses::SceneConfig sceneConfig(ramses::sceneId_t{123}, ramses::EScenePublicationMode::LocalAndRemote);
    ramses::Scene* scene = ramses.createScene(sceneConfig, "triangles scene");

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
    const std::array<ramses::vec3f, 3u> vertexPositionsData{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{1.f, 0.f, -1.f}, ramses::vec3f{0.f, 1.f, -1.f} };
    ramses::ArrayResource* vertexPositions = scene->createArrayResource(3u, vertexPositionsData.data());
    const std::array<uint16_t, 3u> indexData{0, 1, 2};
    ramses::ArrayResource* indices = scene->createArrayResource(3u, indexData.data());

    // initialize effect
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-basic-blending.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-blending.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    const ramses::Effect* effect = scene->createEffect(effectDesc, "glsl shader");

    // create an appearance for red triangle
    ramses::Appearance* appearanceRed = scene->createAppearance(*effect, "red triangle appearance");
    // create an appearance for green triangle
    ramses::Appearance* appearanceGreen = scene->createAppearance(*effect, "green triangle appearance");
    // create an appearance for blue triangle
    ramses::Appearance* appearanceBlue = scene->createAppearance(*effect, "blue triangle appearance");

    ramses::Geometry* geometry = scene->createGeometry(*effect, "triangle geometry");
    geometry->setIndices(*indices);
    std::optional<ramses::AttributeInput> positionsInput = effect->findAttributeInput("a_position");
    assert(positionsInput.has_value());
    geometry->setInputBuffer(*positionsInput, *vertexPositions);

    // create mesh nodes to define the triangles with different appearances
    ramses::MeshNode* meshNodeRed = scene->createMeshNode("red triangle mesh node");
    ramses::MeshNode* meshNodeGreen = scene->createMeshNode("green triangle mesh node");
    ramses::MeshNode* meshNodeBlue = scene->createMeshNode("blue triangle mesh node");

    // offset triangles so that they are not fully overlapping

    meshNodeRed->setTranslation({0.f, -0.2f, -12.f});
    meshNodeGreen->setTranslation({-0.2f, 0.f, -11.f});
    meshNodeBlue->setTranslation({0.2f, 0.2f, -10.f});

    // get handle to appearances' input and set color with alpha smaller than 1
    std::optional<ramses::UniformInput> colorInput = effect->findUniformInput("color");
    assert(colorInput.has_value());
    appearanceRed->setInputValue(*colorInput, ramses::vec4f{ 1.f, 0.f, 0.f, 0.9f });
    appearanceGreen->setInputValue(*colorInput, ramses::vec4f{ 0.f, 1.f, 0.f, 0.6f });
    appearanceBlue->setInputValue(*colorInput, ramses::vec4f{ 0.f, 0.f, 1.f, 0.3f });

    /// [Basic Blending Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // set blending states for alpha blending
    appearanceRed->setBlendingFactors(ramses::EBlendFactor::SrcAlpha, ramses::EBlendFactor::OneMinusSrcAlpha, ramses::EBlendFactor::One, ramses::EBlendFactor::One);
    appearanceRed->setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
    appearanceGreen->setBlendingFactors(ramses::EBlendFactor::SrcAlpha, ramses::EBlendFactor::OneMinusSrcAlpha, ramses::EBlendFactor::One, ramses::EBlendFactor::One);
    appearanceGreen->setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);
    appearanceBlue->setBlendingFactors(ramses::EBlendFactor::SrcAlpha, ramses::EBlendFactor::OneMinusSrcAlpha, ramses::EBlendFactor::One, ramses::EBlendFactor::One);
    appearanceBlue->setBlendingOperations(ramses::EBlendOperation::Add, ramses::EBlendOperation::Add);

    // set appearances to mesh nodes
    meshNodeRed->setAppearance(*appearanceRed);
    meshNodeGreen->setAppearance(*appearanceGreen);
    meshNodeBlue->setAppearance(*appearanceBlue);

    // set same geometry to all mesh nodes
    meshNodeRed->setGeometry(*geometry);
    meshNodeGreen->setGeometry(*geometry);
    meshNodeBlue->setGeometry(*geometry);

    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    // set render order so that triangles are rendered back to front
    renderGroup->addMeshNode(*meshNodeRed, 0);
    renderGroup->addMeshNode(*meshNodeGreen, 1);
    renderGroup->addMeshNode(*meshNodeBlue, 2);
    /// [Basic Blending Example]

    // signal the scene it is in a state that can be rendered
    scene->flush();

    // distribute the scene to RAMSES
    scene->publish(ramses::EScenePublicationMode::LocalAndRemote);

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
