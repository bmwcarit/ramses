//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"
#include "ramses-utils.h"

#include <cstdio>
#include <thread>

/**
 * @example ramses-example-basic-compositing/src/main.cpp
 * @brief Basic Compositing Example
 */

int main(int argc, char* argv[])
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient ramses("ramses-example-basic-compositing", framework);
    framework.connect();

    const uint32_t streamId = 1u;
    printf("using stream-texture id: %u", streamId);

    // create a scene for distributing content
    ramses::Scene* scene = ramses.createScene(123u);

    // every scene needs a render pass with camera
    ramses::Camera* camera = scene->createRemoteCamera("my camera");
    camera->setTranslation(0.0f, 0.0f, 5.0f);
    ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsArray[] = {-1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.f};
    const ramses::Vector3fArray* vertexPositions = ramses.createConstVector3fArray(4, vertexPositionsArray);

    float textureCoordsArray[] = {0.f, 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f};
    const ramses::Vector2fArray* textureCoords = ramses.createConstVector2fArray(4, textureCoordsArray);

    uint16_t indicesArray[] = {0, 1, 2, 2, 1, 3};
    const ramses::UInt16Array* indices = ramses.createConstUInt16Array(6, indicesArray);

    /// [Basic Compositing Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // texture
    ramses::Texture2D* texture =
        ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-basic-compositing-texture.png", ramses);

    // use Texture2D as fallback for StreamTexture
    ramses::StreamTexture* streamTexture = scene->createStreamTexture(*texture, static_cast<ramses::streamSource_t>(streamId), "streamTexture");

    /// [Basic Compositing Example]

    ramses::TextureSampler* sampler =
        scene->createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat, ramses::ETextureSamplingMethod_Nearest, ramses::ETextureSamplingMethod_Nearest, *streamTexture);

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-basic-compositing.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-compositing.frag");

    const ramses::Effect* effectTex = ramses.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = scene->createAppearance(*effectTex, "triangle appearance");

    // set vertex positions directly in geometry
    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effectTex, "triangle geometry");
    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    ramses::AttributeInput texcoordsInput;
    effectTex->findAttributeInput("a_position", positionsInput);
    effectTex->findAttributeInput("a_texcoord", texcoordsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);
    geometry->setInputBuffer(texcoordsInput, *textureCoords);

    ramses::UniformInput textureInput;
    effectTex->findUniformInput("textureSampler", textureInput);
    appearance->setInputTexture(textureInput, *sampler);

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = scene->createMeshNode("textured triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);

    // signal the scene it is in a state that can be rendered
    scene->flush();

    // distribute the scene to RAMSES
    scene->publish();

    // application logic
    std::this_thread::sleep_for(std::chrono::seconds(100));

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    ramses.destroy(*scene);
    ramses.destroy(*vertexPositions);
    ramses.destroy(*textureCoords);
    ramses.destroy(*indices);
    framework.disconnect();

    return 0;
}
