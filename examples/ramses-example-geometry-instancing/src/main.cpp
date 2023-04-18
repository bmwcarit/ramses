//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include <cmath>
#include <thread>

/**
 * @example ramses-example-geometry-instancing/src/main.cpp
 * @brief Geometry Instancing Example
 */

int main()
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework;
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-geometry-instancing"));
    framework.connect();

    // create a scene for distributing content
    ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "geometry instancing scene");

    // every scene needs a render pass with camera
    auto* camera = scene->createPerspectiveCamera("my camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    camera->setTranslation({0.0f, 0.0f, 5.0f});
    ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    /// [Basic Geometry Instancing Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // prepare triangle geometry: vertex position array and index array
    const std::array<ramses::vec3f, 3u> vertexPositionsData{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{1.f, 0.f, -1.f}, ramses::vec3f{0.f, 1.f, -1.f} };
    ramses::ArrayResource* vertexPositions = scene->createArrayResource(3u, vertexPositionsData.data());
    const std::array<uint16_t, 3u> indexData{ 0, 1, 2 };
    ramses::ArrayResource* indices = scene->createArrayResource(3, indexData.data());

    // ------- Instancing with uniforms --------
    // create an appearance for red triangles
    ramses::EffectDescription effectDescUniform;
    effectDescUniform.setVertexShaderFromFile("res/ramses-example-geometry-instancing-uniform.vert");
    effectDescUniform.setFragmentShaderFromFile("res/ramses-example-geometry-instancing-uniform.frag");
    effectDescUniform.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    const ramses::Effect* uniformEffect = scene->createEffect(effectDescUniform, ramses::ResourceCacheFlag_DoNotCache, "uniform-instancing");
    ramses::Appearance* uniformAppearance = scene->createAppearance(*uniformEffect, "triangle_uniforms");

    // get input data of appearance and bind required data
    ramses::UniformInput uniformInstanceTranslationInput;
    uniformEffect->findUniformInput("translations", uniformInstanceTranslationInput);
    ramses::vec3f translations[10];

    ramses::UniformInput uniformInstanceColorInput;
    uniformEffect->findUniformInput("colors", uniformInstanceColorInput);
    ramses::vec4f colors[10];

    // set vertex positions directly in geometry
    ramses::GeometryBinding* uniformGeometry = scene->createGeometryBinding(*uniformEffect, "triangle geometry uniforms");
    uniformGeometry->setIndices(*indices);
    ramses::AttributeInput uniformPositionInput;
    uniformEffect->findAttributeInput("a_position", uniformPositionInput);
    uniformGeometry->setInputBuffer(uniformPositionInput, *vertexPositions);

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* uniformMeshNode = scene->createMeshNode("uniform-instanced triangle");
    uniformMeshNode->setAppearance(*uniformAppearance);
    uniformMeshNode->setGeometryBinding(*uniformGeometry);
    uniformMeshNode->setInstanceCount(10u);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*uniformMeshNode);


    // ------- Instancing with vertex arrays --------
    // create an appearance for red triangles
    ramses::EffectDescription effectDescVertex;
    effectDescVertex.setVertexShaderFromFile("res/ramses-example-geometry-instancing-vertex.vert");
    effectDescVertex.setFragmentShaderFromFile("res/ramses-example-geometry-instancing-vertex.frag");
    effectDescVertex.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    const ramses::Effect* vertexEffect = scene->createEffect(effectDescVertex, ramses::ResourceCacheFlag_DoNotCache, "vertex-instancing");
    ramses::Appearance* vertexAppearance = scene->createAppearance(*vertexEffect, "triangle_uniforms");

    // set vertex positions directly in geometry
    ramses::GeometryBinding* vertexGeometry = scene->createGeometryBinding(*vertexEffect, "triangle geometry uniforms");
    vertexGeometry->setIndices(*indices);
    ramses::AttributeInput vertexPositionInput;
    vertexEffect->findAttributeInput("a_position", vertexPositionInput);
    vertexGeometry->setInputBuffer(vertexPositionInput, *vertexPositions);

    // prepare triangle geometry: vertex position array and index array
    const std::array<ramses::vec3f, 4u> vertexInstanceTranslationArray{
        ramses::vec3f{-4.5f, .75f, -6.0f},
        ramses::vec3f{4.5f, .75f, -6.0f},
        ramses::vec3f{4.5f, -1.75f, -6.0f},
        ramses::vec3f{-4.5f, -1.75f, -6.0f}
    };
    const std::array<ramses::vec4f, 4u> vertexInstanceColorArray{
        ramses::vec4f{1.0f, 0.0f, 0.0f, 1.0f},
        ramses::vec4f{0.0f, 1.0f, 0.0f, 1.0f},
        ramses::vec4f{0.0f, 0.0f, 1.0f, 1.0f},
        ramses::vec4f{1.0f, 1.0f, 1.0f, 1.0f}
    };
    const ramses::ArrayResource* vertexTranslations = scene->createArrayResource(4u, vertexInstanceTranslationArray.data());
    const ramses::ArrayResource* vertexColors = scene->createArrayResource(4u, vertexInstanceColorArray.data());

    ramses::AttributeInput vertexTranslationInput;
    vertexEffect->findAttributeInput("translation", vertexTranslationInput);
    vertexGeometry->setInputBuffer(vertexTranslationInput, *vertexTranslations, 1u);
    ramses::AttributeInput vertexColorInput;
    vertexEffect->findAttributeInput("color", vertexColorInput);
    vertexGeometry->setInputBuffer(vertexColorInput, *vertexColors, 1u);

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* vertexMeshNode = scene->createMeshNode("vertex-instanced triangle");
    vertexMeshNode->setAppearance(*vertexAppearance);
    vertexMeshNode->setGeometryBinding(*vertexGeometry);
    vertexMeshNode->setInstanceCount(4u);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*vertexMeshNode);


    // distribute the scene to RAMSES
    scene->publish();

    // application logic
    uint32_t t = 0;
    while (t < 1000000)
    {
        // update translations of triangle instances
        for (uint32_t i = 0; i < 10; i++)
        {
            translations[i][0] = -3.0f + i * 0.7f;
            translations[i][1] = -0.5f + static_cast<float>(std::sin(i+0.05f*t));
            translations[i][2] = -5.0f;
        }
        uniformAppearance->setInputValue(uniformInstanceTranslationInput, 10, translations);

        // update color of triangle instances
        for (uint32_t i = 0; i < 10; i++)
        {
            colors[i][0] = 1.0f;
            colors[i][1] = 0.75f * static_cast<float>(std::sin(i+0.05f*t));
            colors[i][2] = 0.2f;
            colors[i][3] = 1.0f;
        }
        uniformAppearance->setInputValue(uniformInstanceColorInput, 10, colors);

        scene->flush();
        t++;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    /// [Basic Geometry Instancing Example]

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    scene->destroy(*vertexPositions);
    scene->destroy(*indices);
    ramses.destroy(*scene);
    framework.disconnect();

    return 0;
}
