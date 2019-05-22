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
 * @example ramses-example-renderonce/src/main.cpp
 * @brief Render Once Example
 */

static const unsigned RTSize = 512u;

int main(int argc, char* argv[])
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient ramses("ramses-example-renderonce", framework);
    framework.connect();

    // prepare triangle geometry: vertex position array and index array
    const float vertexPositionsQuadArray[] = { -1.5f, -0.75f, -1.f, 1.5f, -0.75f, -1.f, -1.5f, 0.75f, -1.f, 1.5f, 0.75f, -1.f };
    const ramses::Vector3fArray* vertexPositionsQuad = ramses.createConstVector3fArray(4, vertexPositionsQuadArray);

    const float vertexPositionsTriangleArray[] = { -0.5f, -0.5f, -1.f, 0.5f, -0.5f, -1.f, -0.5f, 0.5f, -1.f };
    const ramses::Vector3fArray* vertexPositionsTriangle = ramses.createConstVector3fArray(3, vertexPositionsTriangleArray);

    const float textureCoordsArray[] = { -1.f, 0.f, 1.0f, 0.f, -1.f, 1.f, 1.0f, 1.f};
    const ramses::Vector2fArray* textureCoords = ramses.createConstVector2fArray(4, textureCoordsArray);

    uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
    const ramses::UInt16Array* indicesQuad = ramses.createConstUInt16Array(6, indicesArray);
    const ramses::UInt16Array* indicesTriangle = ramses.createConstUInt16Array(3, indicesArray);

    ramses::EffectDescription triangleEffectDesc;
    triangleEffectDesc.setVertexShaderFromFile("res/ramses-example-renderonce-simple-color.vert");
    triangleEffectDesc.setFragmentShaderFromFile("res/ramses-example-renderonce-simple-color.frag");
    triangleEffectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    const ramses::Effect* triangleEffect = ramses.createEffect(triangleEffectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");

    ramses::EffectDescription quadEffectDesc;
    quadEffectDesc.setVertexShaderFromFile("res/ramses-example-renderonce-texturing.vert");
    quadEffectDesc.setFragmentShaderFromFile("res/ramses-example-renderonce-texturing.frag");
    quadEffectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    const ramses::Effect* quadEffect = ramses.createEffect(quadEffectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");

    ramses::Scene* scene = ramses.createScene(123u, ramses::SceneConfig(), "basic rendertarget scene");

    // every render pass needs a camera to define rendering parameters
    ramses::Node* cameraTranslate = scene->createNode();
    cameraTranslate->setTranslation(0.0f, 0.0f, 5.0f);
    ramses::PerspectiveCamera* cameraA = scene->createPerspectiveCamera("camera of renderpass A");
    cameraA->setParent(*cameraTranslate);
    cameraA->setFrustum(-0.1f, 0.1f, -0.1f, 0.1f, 1.f, 10.f);
    // we want to render to the full extent of render target which has the resolution 64x64 (created later)
    cameraA->setViewport(0u, 0u, RTSize, RTSize);

    // usage of global renderer camera for the resolving render pass
    ramses::Camera* cameraB = scene->createRemoteCamera("camera of renderpass B");
    cameraB->setParent(*cameraTranslate);

    /// [Render Once Example Setup]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // create the render once renderpass
    ramses::RenderPass* renderPassRT = scene->createRenderPass("renderpass to render target");
    renderPassRT->setClearFlags(ramses::EClearFlags_All);
    renderPassRT->setClearColor(1.f, 1.f, 1.f, 1.f);
    renderPassRT->setCamera(*cameraA);
    renderPassRT->setRenderOnce(true);

    /// [Render Once Example Setup]

    // create a render target and assign it to renderpass A
    ramses::RenderBuffer* renderBuffer = scene->createRenderBuffer(RTSize, RTSize, ramses::ERenderBufferType_Color, ramses::ERenderBufferFormat_RGBA8, ramses::ERenderBufferAccessMode_ReadWrite);
    ramses::RenderTargetDescription rtDesc;
    rtDesc.addRenderBuffer(*renderBuffer);
    ramses::RenderTarget* renderTarget = scene->createRenderTarget(rtDesc);
    renderPassRT->setRenderTarget(renderTarget);

    // create final render pass
    ramses::RenderPass* renderPassFinal = scene->createRenderPass("renderpass to screen");
    renderPassFinal->setClearFlags(ramses::EClearFlags_None);
    renderPassFinal->setCamera(*cameraB);

    // create render group for each renderpass
    ramses::RenderGroup* renderGroupA = scene->createRenderGroup("rendergroup A");
    ramses::RenderGroup* renderGroupB = scene->createRenderGroup("rendergroup B");
    renderPassRT->addRenderGroup(*renderGroupA);
    renderPassFinal->addRenderGroup(*renderGroupB);

    // create triangle appearance, get input data and bind required data
    ramses::UniformInput color;
    ramses::Appearance* appearanceA = scene->createAppearance(*triangleEffect, "triangle appearance");
    {
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
            ramses::ETextureSamplingMethod_Linear,
            ramses::ETextureSamplingMethod_Linear,
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

    // create meshes
    ramses::MeshNode* meshNodeA = scene->createMeshNode("red triangle mesh node");
    meshNodeA->setAppearance(*appearanceA);
    meshNodeA->setGeometryBinding(*geometryA);

    ramses::MeshNode* meshNodeB = scene->createMeshNode("texture quad mesh node");
    meshNodeB->setAppearance(*appearanceB);
    meshNodeB->setGeometryBinding(*geometryB);

    // add triangle mesh to first pass and quad to second one
    renderGroupA->addMeshNode(*meshNodeA);
    renderGroupB->addMeshNode(*meshNodeB);

    // signal the scene it is in a state that can be rendered
    scene->flush();
    scene->publish();

    // application logic
    uint32_t loops = 10000;

    /// [Render Once Example Retrigger]
    while (--loops)
    {
        // change color once per second
        // this will re-render the mesh to the render target only once per color change
        std::this_thread::sleep_for(std::chrono::seconds(1));
        appearanceA->setInputValueVector4f(color, (loops % 2 ? 1.0f : 0.0f), 0.0f, (loops % 2 ? 0.0f : 1.0f), 1.0f);
        renderPassRT->retriggerRenderOnce();
        scene->flush();
    }
    /// [Render Once Example Retrigger]

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    ramses.destroy(*scene);
    ramses.destroy(*vertexPositionsTriangle);
    ramses.destroy(*vertexPositionsQuad);
    ramses.destroy(*textureCoords);
    ramses.destroy(*indicesQuad);
    ramses.destroy(*indicesTriangle);
    framework.disconnect();

    return 0;
}
