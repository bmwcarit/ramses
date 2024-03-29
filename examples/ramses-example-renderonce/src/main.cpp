//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/ramses-client.h"
#include "ramses/client/RenderTargetDescription.h"

#include <thread>

/**
 * @example ramses-example-renderonce/src/main.cpp
 * @brief Render Once Example
 */

static const unsigned RTSize = 512u;

int main()
{
    // register at RAMSES daemon
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-renderonce"));
    framework.connect();

    const ramses::SceneConfig sceneConfig(ramses::sceneId_t{123}, ramses::EScenePublicationMode::LocalAndRemote);
    ramses::Scene* scene = ramses.createScene(sceneConfig, "basic rendertarget scene");

    // prepare triangle geometry: vertex position array and index array
    const std::array<ramses::vec3f, 4u> vertexPositionsQuadArray{ ramses::vec3f{-1.5f, -0.75f, -1.f}, ramses::vec3f{1.5f, -0.75f, -1.f}, ramses::vec3f{-1.5f, 0.75f, -1.f}, ramses::vec3f{1.5f, 0.75f, -1.f} };
    ramses::ArrayResource* vertexPositionsQuad = scene->createArrayResource(4u, vertexPositionsQuadArray.data());

    const std::array<ramses::vec3f, 3u> vertexPositionsTriangleArray{ ramses::vec3f{-0.5f, -0.5f, -1.f}, ramses::vec3f{0.5f, -0.5f, -1.f}, ramses::vec3f{-0.5f, 0.5f, -1.f} };
    ramses::ArrayResource* vertexPositionsTriangle = scene->createArrayResource(4u, vertexPositionsTriangleArray.data());

    const std::array<ramses::vec2f, 4u> textureCoordsArray{ ramses::vec2f{-1.f, 0.f}, ramses::vec2f{1.f, 0.f}, ramses::vec2f{-1.f, 1.f}, ramses::vec2f{1.f, 1.f} };
    ramses::ArrayResource* textureCoords = scene->createArrayResource(4u, textureCoordsArray.data());

    const std::array<uint16_t, 6u> indicesArray{ 0, 1, 2, 2, 1, 3 };
    ramses::ArrayResource* indices = scene->createArrayResource(6u, indicesArray.data());

    ramses::EffectDescription triangleEffectDesc;
    triangleEffectDesc.setVertexShaderFromFile("res/ramses-example-renderonce-simple-color.vert");
    triangleEffectDesc.setFragmentShaderFromFile("res/ramses-example-renderonce-simple-color.frag");
    triangleEffectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);
    const ramses::Effect* triangleEffect = scene->createEffect(triangleEffectDesc, "glsl shader");

    ramses::EffectDescription quadEffectDesc;
    quadEffectDesc.setVertexShaderFromFile("res/ramses-example-renderonce-texturing.vert");
    quadEffectDesc.setFragmentShaderFromFile("res/ramses-example-renderonce-texturing.frag");
    quadEffectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);
    const ramses::Effect* quadEffect = scene->createEffect(quadEffectDesc, "glsl shader");

    // every render pass needs a camera to define rendering parameters
    ramses::Node* cameraTranslate = scene->createNode();
    cameraTranslate->setTranslation({0.0f, 0.0f, 5.0f});
    ramses::PerspectiveCamera* cameraA = scene->createPerspectiveCamera("camera of renderpass A");
    cameraA->setParent(*cameraTranslate);
    cameraA->setFrustum(-0.1f, 0.1f, -0.1f, 0.1f, 1.f, 10.f);
    // we want to render to the full extent of render target which has the resolution 64x64 (created later)
    cameraA->setViewport(0u, 0u, RTSize, RTSize);

    // use another camera for the resolving render pass
    auto* cameraB = scene->createPerspectiveCamera("camera of renderpass B");
    cameraB->setViewport(0, 0, 1280u, 480u);
    cameraB->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    cameraB->setParent(*cameraTranslate);

    /// [Render Once Example Setup]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // create the render once renderpass
    ramses::RenderPass* renderPassRT = scene->createRenderPass("renderpass to render target");
    renderPassRT->setClearFlags(ramses::EClearFlag::All);
    renderPassRT->setClearColor({1.f, 1.f, 1.f, 1.f});
    renderPassRT->setCamera(*cameraA);
    renderPassRT->setRenderOnce(true);

    /// [Render Once Example Setup]

    // create a render target and assign it to renderpass A
    ramses::RenderBuffer* renderBuffer = scene->createRenderBuffer(RTSize, RTSize, ramses::ERenderBufferFormat::RGBA8, ramses::ERenderBufferAccessMode::ReadWrite);
    ramses::RenderTargetDescription rtDesc;
    rtDesc.addRenderBuffer(*renderBuffer);
    ramses::RenderTarget* renderTarget = scene->createRenderTarget(rtDesc);
    renderPassRT->setRenderTarget(renderTarget);

    // create final render pass
    ramses::RenderPass* renderPassFinal = scene->createRenderPass("renderpass to screen");
    renderPassFinal->setClearFlags(ramses::EClearFlag::None);
    renderPassFinal->setCamera(*cameraB);

    // create render group for each renderpass
    ramses::RenderGroup* renderGroupA = scene->createRenderGroup("rendergroup A");
    ramses::RenderGroup* renderGroupB = scene->createRenderGroup("rendergroup B");
    renderPassRT->addRenderGroup(*renderGroupA);
    renderPassFinal->addRenderGroup(*renderGroupB);

    // create triangle appearance, get input data and bind required data
    std::optional<ramses::UniformInput> color;
    ramses::Appearance* appearanceA = scene->createAppearance(*triangleEffect, "triangle appearance");
    {
        color = triangleEffect->findUniformInput("color");
        assert(color.has_value());
        appearanceA->setInputValue(*color, ramses::vec4f{ 1.0f, 0.0f, 0.0f, 1.0f });
    }

    ramses::Geometry* geometryA = scene->createGeometry(*triangleEffect, "triangle geometry");
    {
        geometryA->setIndices(*indices);
        std::optional<ramses::AttributeInput> positionsInput = triangleEffect->findAttributeInput("a_position");
        assert(positionsInput.has_value());
        geometryA->setInputBuffer(*positionsInput, *vertexPositionsTriangle);
    }

    // create quad appearance, get input data and bind required data
    ramses::Appearance* appearanceB = scene->createAppearance(*quadEffect, "quad appearance");
    {
        // create texture sampler with render target
        ramses::TextureSampler* sampler = scene->createTextureSampler(
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureSamplingMethod::Linear,
            ramses::ETextureSamplingMethod::Linear,
            *renderBuffer);

        // set render target sampler as input
        std::optional<ramses::UniformInput> textureInput = quadEffect->findUniformInput("textureSampler");
        appearanceB->setInputTexture(*textureInput, *sampler);
    }

    ramses::Geometry* geometryB = scene->createGeometry(*quadEffect, "quad geometry");
    {
        geometryB->setIndices(*indices);
        std::optional<ramses::AttributeInput> positionsInput = quadEffect->findAttributeInput("a_position");
        std::optional<ramses::AttributeInput> texcoordsInput = quadEffect->findAttributeInput("a_texcoord");
        assert(positionsInput.has_value() && texcoordsInput.has_value());
        geometryB->setInputBuffer(*positionsInput, *vertexPositionsQuad);
        geometryB->setInputBuffer(*texcoordsInput, *textureCoords);
    }

    // create meshes
    ramses::MeshNode* meshNodeA = scene->createMeshNode("red triangle mesh node");
    meshNodeA->setAppearance(*appearanceA);
    meshNodeA->setGeometry(*geometryA);
    meshNodeA->setIndexCount(3u); // using only 3 indices from index buffer which defines quad

    ramses::MeshNode* meshNodeB = scene->createMeshNode("texture quad mesh node");
    meshNodeB->setAppearance(*appearanceB);
    meshNodeB->setGeometry(*geometryB);

    // add triangle mesh to first pass and quad to second one
    renderGroupA->addMeshNode(*meshNodeA);
    renderGroupB->addMeshNode(*meshNodeB);

    // signal the scene it is in a state that can be rendered
    scene->flush();
    scene->publish(ramses::EScenePublicationMode::LocalAndRemote);

    // application logic
    uint32_t loops = 100;

    /// [Render Once Example Retrigger]
    while (--loops != 0u)
    {
        // change color once per second
        // this will re-render the mesh to the render target only once per color change
        std::this_thread::sleep_for(std::chrono::seconds(1));
        appearanceA->setInputValue(*color, ramses::vec4f{((loops % 2) != 0u ? 1.0f : 0.0f), 0.0f, ((loops % 2) != 0u ? 0.0f : 1.0f), 1.0f});
        renderPassRT->retriggerRenderOnce();
        scene->flush();
    }
    /// [Render Once Example Retrigger]

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    ramses.destroy(*scene);
    framework.disconnect();

    return 0;
}
