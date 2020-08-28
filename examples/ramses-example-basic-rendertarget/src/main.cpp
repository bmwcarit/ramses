//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"
#include "ramses-client-api/RenderTargetDescription.h"

#include <thread>

/**
 * @example ramses-example-basic-rendertarget/src/main.cpp
 * @brief Basic Rendertarget Example
 */

int main(int argc, char* argv[])
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-basic-rendertarget"));
    framework.connect();

    // create a scene
    ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "basic rendertarget scene");

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsQuadArray[] = { -1.5f, -0.75f, -1.f, 1.5f, -0.75f, -1.f, -1.5f, 0.75f, -1.f, 1.5f, 0.75f, -1.f };
    ramses::ArrayResource* vertexPositionsQuad = scene->createArrayResource(ramses::EDataType::Vector3F, 4, vertexPositionsQuadArray);

    float vertexPositionsTriangleArray[] = { -0.5f, -0.5f, -1.f, 0.5f, -0.5f, -1.f, -0.5f, 0.5f, -1.f };
    ramses::ArrayResource* vertexPositionsTriangle = scene->createArrayResource(ramses::EDataType::Vector3F, 3, vertexPositionsTriangleArray);

    float textureCoordsArray[] = { -1.f, 0.f, 1.0f, 0.f, -1.f, 1.f, 1.0f, 1.f};
    ramses::ArrayResource* textureCoords = scene->createArrayResource(ramses::EDataType::Vector2F, 4, textureCoordsArray);

    uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
    ramses::ArrayResource* indicesQuad = scene->createArrayResource(ramses::EDataType::UInt16, 6, indicesArray);
    ramses::ArrayResource* indicesTriangle = scene->createArrayResource(ramses::EDataType::UInt16, 3, indicesArray);

    ramses::EffectDescription triangleEffectDesc;
    triangleEffectDesc.setVertexShaderFromFile("res/ramses-example-basic-rendertarget-simple-color.vert");
    triangleEffectDesc.setFragmentShaderFromFile("res/ramses-example-basic-rendertarget-simple-color.frag");
    triangleEffectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    const ramses::Effect* triangleEffect = scene->createEffect(triangleEffectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");

    ramses::EffectDescription quadEffectDesc;
    quadEffectDesc.setVertexShaderFromFile("res/ramses-example-basic-rendertarget-texturing.vert");
    quadEffectDesc.setFragmentShaderFromFile("res/ramses-example-basic-rendertarget-texturing.frag");
    quadEffectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    const ramses::Effect* quadEffect = scene->createEffect(quadEffectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");

    /// [Basic Rendertarget Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // every render pass needs a camera to define rendering parameters
    // usage of a custom perspective camera for the render pass assigned to the render target
    ramses::Node* cameraTranslate = scene->createNode();
    cameraTranslate->setTranslation(0.0f, 0.0f, 5.0f);
    ramses::PerspectiveCamera* cameraA = scene->createPerspectiveCamera("camera of renderpass A");
    cameraA->setParent(*cameraTranslate);
    cameraA->setFrustum(-0.1f, 0.1f, -0.1f, 0.1f, 1.f, 10.f);
    // we want to render to the full extent of render target which has the resolution 64x64 (created later)
    cameraA->setViewport(0u, 0u, 64u, 64u);

    // usage of global renderer camera for the resolving render pass
    ramses::Camera* cameraB = scene->createRemoteCamera("camera of renderpass B");
    cameraB->setParent(*cameraTranslate);

    // create the renderpasses
    ramses::RenderPass* renderPassA = scene->createRenderPass("renderpass A");
    ramses::RenderPass* renderPassB = scene->createRenderPass("renderpass B");
    renderPassB->setClearFlags(ramses::EClearFlags_None);
    renderPassA->setClearColor(1.f, 1.f, 1.f, 1.f);

    // set valid cameras for the passes
    renderPassA->setCamera(*cameraA);
    renderPassB->setCamera(*cameraB);

    // create render group for each renderpass
    ramses::RenderGroup* renderGroupA = scene->createRenderGroup("rendergroup A");
    ramses::RenderGroup* renderGroupB = scene->createRenderGroup("rendergroup B");
    renderPassA->addRenderGroup(*renderGroupA);
    renderPassB->addRenderGroup(*renderGroupB);

    // create a render target and assign it to renderpass A
    ramses::RenderBuffer* renderBuffer = scene->createRenderBuffer(64u, 64u, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_ReadWrite);
    ramses::RenderTargetDescription rtDesc;
    rtDesc.addRenderBuffer(*renderBuffer);
    ramses::RenderTarget* renderTarget = scene->createRenderTarget(rtDesc);
    renderPassA->setRenderTarget(renderTarget);

    // create triangle appearance, get input data and bind required data
    ramses::Appearance* appearanceA = scene->createAppearance(*triangleEffect, "triangle appearance");
    {
        ramses::UniformInput color;
        triangleEffect->findUniformInput("color", color);
        appearanceA->setInputValueVector4f(color, 1.0f, 0.0f, 0.0f, 1.0f);
    }

    ramses::GeometryBinding* geometryA = scene->createGeometryBinding(*triangleEffect, "triangle geometry");
    {
        geometryA->setIndices(*indicesTriangle);
        ramses::AttributeInput positionsInput;
        triangleEffect->findAttributeInput("a_position", positionsInput);
        geometryA->setInputBuffer(positionsInput, *vertexPositionsTriangle);
    }

    // create quad appearance, get input data and bind required data
    ramses::Appearance* appearanceB = scene->createAppearance(*quadEffect, "quad appearance");
    {
        // create texture sampler with render target
        ramses::TextureSampler* sampler = scene->createTextureSampler(
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureAddressMode_Repeat,
            ramses::ETextureSamplingMethod_Nearest,
            ramses::ETextureSamplingMethod_Nearest,
            *renderBuffer);

        // set render target sampler as input
        ramses::UniformInput textureInput;
        quadEffect->findUniformInput("textureSampler", textureInput);
        appearanceB->setInputTexture(textureInput, *sampler);
    }

    ramses::GeometryBinding* geometryB = scene->createGeometryBinding(*quadEffect, "quad geometry");
    {
        geometryB->setIndices(*indicesQuad);
        ramses::AttributeInput positionsInput;
        ramses::AttributeInput texcoordsInput;
        quadEffect->findAttributeInput("a_position", positionsInput);
        quadEffect->findAttributeInput("a_texcoord", texcoordsInput);
        geometryB->setInputBuffer(positionsInput, *vertexPositionsQuad);
        geometryB->setInputBuffer(texcoordsInput, *textureCoords);
    }

    // create meshs
    ramses::MeshNode* meshNodeA = scene->createMeshNode("red triangle mesh node");
    meshNodeA->setAppearance(*appearanceA);
    meshNodeA->setGeometryBinding(*geometryA);

    ramses::MeshNode* meshNodeB = scene->createMeshNode("texture quad mesh node");
    meshNodeB->setAppearance(*appearanceB);
    meshNodeB->setGeometryBinding(*geometryB);

    // add triangle mesh to first pass and quad to second one
    renderGroupA->addMeshNode(*meshNodeA);
    renderGroupB->addMeshNode(*meshNodeB);

    /// [Basic Rendertarget Example]

    // signal the scene it is in a state that can be rendered
    scene->flush();

    scene->publish();

    // application logic
    uint32_t loops = 10000;

    while (--loops)
    {
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    scene->destroy(*vertexPositionsTriangle);
    scene->destroy(*vertexPositionsQuad);
    scene->destroy(*textureCoords);
    scene->destroy(*indicesQuad);
    scene->destroy(*indicesTriangle);
    ramses.destroy(*scene);
    framework.disconnect();

    return 0;
}
