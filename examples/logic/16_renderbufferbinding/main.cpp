//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/logic/LogicEngine.h"
#include "ramses/client/logic/CameraBinding.h"
#include "ramses/client/logic/RenderBufferBinding.h"
#include "ramses/client/logic/LuaInterface.h"
#include "ramses/client/logic/Property.h"

#include "ramses/client/ramses-client.h"
#include "ramses/client/ramses-utils.h"

#include "SimpleRenderer.h"

#include <cassert>
#include <array>
#include <thread>
#include <cmath>

/**
* This example demonstrates how to change resolution of a render target/buffer using logic,
after the render buffer scene object was already created in a scene but was not created by the renderer yet.
* This can be useful if an asset is created and exported with a certain initial resolution but there is a need to adjust it
* in runtime upon loading to match platform display resolution or a certain level of detail.
*/

/// Helper method which creates a simple ramses scene with render target.
ramses::Scene* CreateSceneWithRenderTarget(ramses::RamsesClient& client);

int main()
{
    /// Use simple class to create ramses framework objects which are not essential for this example.
    SimpleRenderer renderer;

    /// Create a simple Ramses scene with a render target which gets a triangle rendered into and then its contents are displayed on a quad as texture.
    auto scene = CreateSceneWithRenderTarget(*renderer.getClient());

    ramses::LogicEngine& logicEngine{ *scene->createLogicEngine() };
    logicEngine.setStatisticsLoggingRate(0u);

    /// Create interface for logic which will control the resolution of the render target and viewport in scene created above
    ramses::LuaInterface* logicInterface = logicEngine.createLuaInterface(R"(
        function interface(IN)
            IN.resolution = Type:Int32()
        end
    )", "logicInterface");

    /**
    * Now we create CameraBinding and RenderBufferBinding which will be used to link the appropriate properties for the resolution change
    * to be correctly applied.
    * In order to change the resolution of our render target in our scene, we need to link the render buffer width and height and also
    * viewport witdth and height of the camera used for the render pass rendering into the render target.
    */
    ramses::CameraBinding* cameraBinding = logicEngine.createCameraBinding(*scene->findObject<ramses::Camera>("renderTargetCamera"), "cameraBinding");
    logicEngine.link(
        *logicInterface->getOutputs()->getChild("resolution"),
        *cameraBinding->getInputs()->getChild("viewport")->getChild("width"));
    logicEngine.link(
        *logicInterface->getOutputs()->getChild("resolution"),
        *cameraBinding->getInputs()->getChild("viewport")->getChild("height"));

    ramses::RenderBufferBinding* renderBufferBinding = logicEngine.createRenderBufferBinding(*scene->findObject<ramses::RenderBuffer>("renderBuffer"), "renderBufferBinding");
    logicEngine.link(
        *logicInterface->getOutputs()->getChild("resolution"),
        *renderBufferBinding->getInputs()->getChild("width"));
    logicEngine.link(
        *logicInterface->getOutputs()->getChild("resolution"),
        *renderBufferBinding->getInputs()->getChild("height"));
    /**
     * Note that we do not link all the RenderBufferBinding input properties ('sampleCount' is not used here),
     * meaning it will stay untouched and use the initial value given when RenderBuffer was created.
     */

    /**
     * Changing resolution of a render buffer via RenderBufferBinding has a very important limitation, it can only be done BEFORE the scene
     * is rendered (before it reaches RendererSceneState::Ready state to be precise), any attempt to change resolution later will have no effect
     * (only an error will be logged).
     */
    /// The actual change of resolution in our scene is then as simple as calling a setter on the logic interface we created
    logicInterface->getInputs()->getChild("resolution")->set(256);

    /// Show the scene on the renderer.
    renderer.showScene(scene->getSceneId());

    /// Simulate an application loop.
    while (!renderer.isWindowClosed())
    {
        /// Update the LogicEngine. This will apply changes to Ramses scene from any running script.
        logicEngine.update();

        /// In order to commit the changes to Ramses scene we need to "flush" them.
        scene->flush();

        /// Process window events, check if window was closed
        renderer.processEvents();

        /// Throttle the simulation loop by sleeping for a bit.
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

ramses::Scene* CreateSceneWithRenderTarget(ramses::RamsesClient& client)
{
    /// Initial resolution is intentionally very low so it is clearly visible when the change later on in example is applied
    constexpr uint32_t InitialRenderTargetRes = 8u;

    ramses::Scene* scene = client.createScene(ramses::sceneId_t{ 123u }, "scene");
    ramses::RenderBuffer* renderBuffer = scene->createRenderBuffer(
        InitialRenderTargetRes, InitialRenderTargetRes, ramses::ERenderBufferFormat::RGBA8, ramses::ERenderBufferAccessMode::ReadWrite, 0u, "renderBuffer");

    /// There are other examples which describe all the steps below in detail, therefore documentation here is reduced.

    /// Render pass rendering a triangle into a render target
    {
        const std::string_view vertSrc = R"(#version 100
        uniform highp mat4 mvpMatrix;
        attribute vec3 a_position;
        void main()
        {
            gl_Position = mvpMatrix * vec4(a_position, 1.0);
        })";
        const std::string_view fragSrc = R"(#version 100
        void main(void)
        {
            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        })";

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShader(vertSrc);
        effectDesc.setFragmentShader(fragSrc);
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

        auto effect = scene->createEffect(effectDesc);
        auto appearance = scene->createAppearance(*effect);
        auto geometry = scene->createGeometry(*effect);
        const std::array<uint16_t, 3> indicesData = { 0, 1, 2 };
        geometry->setIndices(*scene->createArrayResource(3u, indicesData.data()));
        const std::array<ramses::vec3f, 3u> vertexPositionsData{ ramses::vec3f{-1.f, 0.f, -1.f}, ramses::vec3f{1.f, 0.f, -1.f}, ramses::vec3f{0.f, 1.f, -1.f} };
        const auto vertexPositions = scene->createArrayResource(3u, vertexPositionsData.data());
        geometry->setInputBuffer(*effect->findAttributeInput("a_position"), *vertexPositions);

        auto meshNode = scene->createMeshNode();
        meshNode->setAppearance(*appearance);
        meshNode->setGeometry(*geometry);

        auto translateNode = scene->createNode();
        translateNode->addChild(*meshNode);
        translateNode->translate({ 0.0f, -0.5f, -5.0f });

        auto renderPass = scene->createRenderPass();
        auto renderGroup = scene->createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);
        renderGroup->addMeshNode(*meshNode);

        auto camera = scene->createOrthographicCamera("renderTargetCamera");
        camera->setFrustum(-1.f, 1.f, -1.f, 1.f, 1.f, 10.f);
        camera->setViewport(0, 0, InitialRenderTargetRes, InitialRenderTargetRes);

        renderPass->setCamera(*camera);

        ramses::RenderTargetDescription rtDesc;
        rtDesc.addRenderBuffer(*renderBuffer);
        ramses::RenderTarget* renderTarget = scene->createRenderTarget(rtDesc);
        renderPass->setRenderTarget(renderTarget);
        renderPass->setClearColor({ 0.f, 0.f, 1.f, 0.5f });
        renderPass->setClearFlags(ramses::EClearFlag::All);
    }

    /// Main render pass which renders contents of the render target created above as a texture on a quad
    {
        const std::string_view vertSrc = R"(#version 100
            uniform highp mat4 mvpMatrix;
            attribute vec3 a_position;
            attribute vec2 a_texcoord;
            varying vec2 v_texcoord;
            void main()
            {
                gl_Position = mvpMatrix * vec4(a_position, 1.0);
                v_texcoord = a_texcoord;
            })";
        const std::string_view fragSrc = R"(#version 100
            precision highp float;
            uniform sampler2D u_texture;
            varying lowp vec2 v_texcoord;
            void main(void)
            {
                vec4 color = texture2D(u_texture, v_texcoord);
                gl_FragColor = color;
            })";

        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShader(vertSrc);
        effectDesc.setFragmentShader(fragSrc);
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);
        auto effect = scene->createEffect(effectDesc);

        const std::array<uint16_t, 6u> indicesArray{ 0, 1, 2, 2, 1, 3 };
        const auto indices = scene->createArrayResource(6u, indicesArray.data());
        const std::array<ramses::vec3f, 4u> vertexPositionsArray{
            ramses::vec3f{ -0.5f, -0.5f, 0.f },
            ramses::vec3f{ 0.5f, -0.5f, 0.f },
            ramses::vec3f{ -0.5f, 0.5f, 0.f },
            ramses::vec3f{ 0.5f, 0.5f, 0.f } };
        const auto vertexPositions = scene->createArrayResource(4u, vertexPositionsArray.data());
        const std::array<ramses::vec2f, 4u> textureCoordsArray{ ramses::vec2f{0.f, 0.f}, ramses::vec2f{2.f, 0.f}, ramses::vec2f{0.f, 2.f}, ramses::vec2f{2.f, 2.f} };
        const auto textureCoords = scene->createArrayResource(4u, textureCoordsArray.data());

        auto appearance = scene->createAppearance(*effect);
        auto geometry = scene->createGeometry(*effect);
        geometry->setIndices(*indices);
        geometry->setInputBuffer(*effect->findAttributeInput("a_position"), *vertexPositions);
        geometry->setInputBuffer(*effect->findAttributeInput("a_texcoord"), *textureCoords);

        auto sampler = scene->createTextureSampler(
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureAddressMode::Repeat,
            ramses::ETextureSamplingMethod::Linear,
            ramses::ETextureSamplingMethod::Linear,
            *renderBuffer);
        appearance->setInputTexture(*effect->findUniformInput("u_texture"), *sampler);

        auto meshNode = scene->createMeshNode("quad");
        meshNode->setAppearance(*appearance);
        meshNode->setGeometry(*geometry);
        meshNode->setTranslation({ 0.f, 0.f, -4.f });

        auto camera = scene->createPerspectiveCamera();
        camera->setViewport(0, 0, SimpleRenderer::GetDisplaySize()[0], SimpleRenderer::GetDisplaySize()[1]);
        camera->setFrustum(19.0f, float(SimpleRenderer::GetDisplaySize()[0]) / float(SimpleRenderer::GetDisplaySize()[1]), 0.1f, 100.0f);
        auto renderPass = scene->createRenderPass();
        renderPass->setCamera(*camera);
        auto renderGroup = scene->createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);
        renderGroup->addMeshNode(*meshNode);
    }

    scene->flush();
    scene->publish();

    return scene;
}
