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

int main(int argc, char* argv[])
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-geometry-instancing"));
    framework.connect();

    // create a scene for distributing content
    ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "geometry instancing scene");

    // every scene needs a render pass with camera
    ramses::Camera* camera = scene->createRemoteCamera("my camera");
    camera->setTranslation(0.0f, 0.0f, 5.0f);
    ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    /// [Basic Geometry Instancing Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsArray[] = { -1.f, 0.f, -1.f, 1.f, 0.f, -1.f, 0.f, 1.f, -1.f };
    ramses::ArrayResource* vertexPositions = scene->createArrayResource(ramses::EDataType::Vector3F, 3, vertexPositionsArray);
    uint16_t indicesArray[] = { 0, 1, 2 };
    ramses::ArrayResource* indices = scene->createArrayResource(ramses::EDataType::UInt16, 3, indicesArray);

    // ------- Instancing with uniforms --------
    // create an appearance for red triangles
    ramses::EffectDescription effectDescUniform;
    effectDescUniform.setVertexShaderFromFile("res/ramses-example-geometry-instancing-uniform.vert");
    effectDescUniform.setFragmentShaderFromFile("res/ramses-example-geometry-instancing-uniform.frag");
    effectDescUniform.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    const ramses::Effect* uniformEffect = scene->createEffect(effectDescUniform, ramses::ResourceCacheFlag_DoNotCache, "uniform-instancing");
    ramses::Appearance* uniformAppearance = scene->createAppearance(*uniformEffect, "triangle_uniforms");

    // get input data of appearance and bind required data
    ramses::UniformInput uniformInstanceTranslationInput;
    uniformEffect->findUniformInput("translations", uniformInstanceTranslationInput);
    float translations[30];

    ramses::UniformInput uniformInstanceColorInput;
    uniformEffect->findUniformInput("colors", uniformInstanceColorInput);
    float colors[40];

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
    effectDescVertex.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    const ramses::Effect* vertexEffect = scene->createEffect(effectDescVertex, ramses::ResourceCacheFlag_DoNotCache, "vertex-instancing");
    ramses::Appearance* vertexAppearance = scene->createAppearance(*vertexEffect, "triangle_uniforms");

    // set vertex positions directly in geometry
    ramses::GeometryBinding* vertexGeometry = scene->createGeometryBinding(*vertexEffect, "triangle geometry uniforms");
    vertexGeometry->setIndices(*indices);
    ramses::AttributeInput vertexPositionInput;
    vertexEffect->findAttributeInput("a_position", vertexPositionInput);
    vertexGeometry->setInputBuffer(vertexPositionInput, *vertexPositions);

    // prepare triangle geometry: vertex position array and index array
    float vertexInstanceTranslationArray[] = {
        -4.5f, .75f, -6.0f,
        4.5f, .75f, -6.0f,
        4.5f, -1.75f, -6.0f,
        -4.5f, -1.75f, -6.0f
    };
    float vertexInstanceColorArray[] = {
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };
    const ramses::ArrayResource* vertexTranslations = scene->createArrayResource(ramses::EDataType::Vector3F, 4, vertexInstanceTranslationArray);
    const ramses::ArrayResource* vertexColors = scene->createArrayResource(ramses::EDataType::Vector4F, 4, vertexInstanceColorArray);

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
            translations[i * 3] = -3.0f + i * 0.7f;
            translations[i * 3 + 1] = -0.5f + static_cast<float>(std::sin(i+0.05f*t));
            translations[i * 3 + 2] = -5.0f;
        }
        uniformAppearance->setInputValueVector3f(uniformInstanceTranslationInput, 10, translations);

        // update color of triangle instances
        for (uint32_t i = 0; i < 10; i++)
        {
            colors[i * 4] = 1.0f;
            colors[i * 4 + 1] = 0.75f * static_cast<float>(std::sin(i+0.05f*t));
            colors[i * 4 + 2] = 0.2f;
            colors[i * 4 + 3] = 1.0f;
        }
        uniformAppearance->setInputValueVector4f(uniformInstanceColorInput, 10, colors);

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
