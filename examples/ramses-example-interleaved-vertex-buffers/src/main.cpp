//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include <thread>

/**
 * @example ramses-example-interleaved-vertex-buffers/src/main.cpp
 * @brief Interleaved vertex buffers Example
 */

int main(int argc, char* argv[])
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-interleaved-vertex-buffers"));
    framework.connect();

    // create a scene for distributing content
    ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "interleaved vertex buffers scene");

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

    /// [Interleaved Vertex Buffers Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // prepare triangle geometry: interleaved position and color data for each vertex
    const float vertexData[] = {
        -1.f, 0.f, -1.f, 1.f,   //vertex 1 position vec4
        1.f, 0.f, 0.f,          //vertex 1 color vec3

        1.f, 0.f, -1.f, 1.f,    //vertex 2 position vec4
        0.f, 1.f, 0.f,          //vertex 2 color vec3

        0.f, 1.f, -1.f, 1.f,    //vertex 3 position vec4
        0.f, 0.f, 1.f,          //vertex 3 color vec3
    };
    ramses::ArrayBuffer* vertexDataBuffer = scene->createArrayBuffer(ramses::EDataType::ByteBlob, sizeof(vertexData));
    vertexDataBuffer->updateData(0u, sizeof(vertexData), vertexData);

    // create an appearance for triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-interleaved-vertex-buffers.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-interleaved-vertex-buffers.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    const ramses::Effect* effect = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = scene->createAppearance(*effect, "appearance");

    // set vertex positions and color in geometry
    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect, "geometry");
    ramses::AttributeInput positionsInput;
    ramses::AttributeInput colorsInput;
    effect->findAttributeInput("a_position", positionsInput);
    effect->findAttributeInput("a_color", colorsInput);

    //positions have Zero offset
    constexpr uint16_t positionsOffset = 0u;
    //colors have offset equal to size of 4 floats, because for every vertex color data starts (directly) after position which is vec4
    constexpr uint16_t colorsOffset = 4 * sizeof(float);
    //Stride is equal to size of 7 floats (vec4 position + vec3 color)
    constexpr uint16_t stride = 7 * sizeof(float);

    //HINT:
    //In this "specific" example another way to safely calculate offsets and stride would be to add the size of the previous data type to previous offset:
    //constexpr uint16_t positionsOffset  = 0u                + 0u; //vertex data starts by position
    //constexpr uint16_t colorsOffset     = positionsOffset   + uint16_t(ramses::GetSizeOfDataType(ramses::EDataType::Vector4F));
    //constexpr uint16_t stride           = colorsOffset      + uint16_t(ramses::GetSizeOfDataType(ramses::EDataType::Vector3F));

    geometry->setInputBuffer(positionsInput, *vertexDataBuffer, positionsOffset, stride);
    geometry->setInputBuffer(colorsInput, *vertexDataBuffer, colorsOffset, stride);

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = scene->createMeshNode("triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setIndexCount(3);
    meshNode->setGeometryBinding(*geometry);
    /// [Interleaved Vertex Buffers Example]
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);
    /// [Interleaved vertex buffers Example]

    // signal the scene it is in a state that can be rendered
    scene->flush();

    // distribute the scene to RAMSES
    scene->publish();

    // application logic
    std::this_thread::sleep_for(std::chrono::seconds(10));

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    scene->destroy(*vertexDataBuffer);
    ramses.destroy(*scene);
    framework.disconnect();

    return 0;
}
