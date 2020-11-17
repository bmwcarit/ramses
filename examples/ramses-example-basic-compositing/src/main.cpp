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
#include "ramses-framework-api/RamsesFrameworkTypes.h"

/**
 * @example ramses-example-basic-compositing/src/main.cpp
 * @brief Basic Compositing Example
 */

int main(int argc, char* argv[])
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-basic-compositing"));
    framework.connect();

    const ramses::waylandIviSurfaceId_t surfaceId(1u);
    printf("using stream-texture id: %u", surfaceId.getValue());

    // create a scene for distributing content
    ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(123u));

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
    float vertexPositionsArray[] = {-1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.f};
    ramses::ArrayResource* vertexPositions = scene->createArrayResource(ramses::EDataType::Vector3F, 4, vertexPositionsArray);

    float textureCoordsArray[] = {0.f, 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f};
    ramses::ArrayResource* textureCoords = scene->createArrayResource(ramses::EDataType::Vector2F, 4, textureCoordsArray);

    uint16_t indicesArray[] = {0, 1, 2, 2, 1, 3};
    ramses::ArrayResource* indices = scene->createArrayResource(ramses::EDataType::UInt16, 6, indicesArray);

    /// [Basic Compositing Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // texture
    ramses::Texture2D* texture =
        ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-basic-compositing-texture.png", *scene);

    // use Texture2D as fallback for StreamTexture
    ramses::StreamTexture* streamTexture = scene->createStreamTexture(*texture, surfaceId, "streamTexture");

    /// [Basic Compositing Example]

    ramses::TextureSampler* sampler =
        scene->createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat, ramses::ETextureSamplingMethod_Nearest, ramses::ETextureSamplingMethod_Nearest, *streamTexture);

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-basic-compositing.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-compositing.frag");

    ramses::Effect* effectTex = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
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
    scene->destroy(*vertexPositions);
    scene->destroy(*textureCoords);
    scene->destroy(*indices);
    ramses.destroy(*scene);
    framework.disconnect();

    return 0;
}
