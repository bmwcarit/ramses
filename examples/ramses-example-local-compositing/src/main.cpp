//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/ramses-client.h"
#include "ramses/client/ramses-utils.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/renderer/RendererConfig.h"
#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/RendererSceneControl.h"
#include "ramses/renderer/IRendererEventHandler.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "ramses/framework/RamsesFrameworkTypes.h"

#include "ramses-cli.h"

#include <thread>
#include <iostream>

/**
 * @example ramses-example-local-compositing/src/main.cpp
 * @brief Local Compositing Example
 */

class RendererEventHandler : public ramses::RendererEventHandlerEmpty, public ramses::RendererSceneControlEventHandlerEmpty
{
public:
    RendererEventHandler(ramses::RamsesRenderer& renderer, ramses::displayId_t display, ramses::waylandIviSurfaceId_t iviSurface)
        : m_renderer(renderer)
        , m_display(display)
        , m_iviSurface(iviSurface)
    {
    }

    void displayCreated(ramses::displayId_t /*displayId*/, ramses::ERendererEventResult result) override
    {
        if (result != ramses::ERendererEventResult::Ok)
        {
            std::cout << "Failed creating display!" << std::endl;
            return;
        }

        m_streamBuffer = m_renderer.createStreamBuffer(m_display, m_iviSurface);
        m_renderer.flush();

        if (canLinkStreamBuffer())
        {
            linkStreamBuffer();
        }
    }

    void dataConsumerCreated(ramses::sceneId_t sceneId, ramses::dataConsumerId_t dataConsumerId) override
    {
        m_textureConsumer = dataConsumerId;
        m_consumerScene = sceneId;

        if (canLinkStreamBuffer())
        {
            linkStreamBuffer();
        }
    }

    void windowClosed(ramses::displayId_t /*displayId*/) override
    {
        m_windowClosed = true;
    }

    [[nodiscard]] bool isWindowClosed() const
    {
        return m_windowClosed;
    }

private:
    [[nodiscard]] bool canLinkStreamBuffer() const
    {
        return m_streamBuffer.isValid() && m_consumerScene.isValid() && m_textureConsumer.isValid();
    }

    void linkStreamBuffer()
    {
        if (m_streamBufferNeedsLinking)
        {
            m_renderer.getSceneControlAPI()->linkStreamBuffer(m_streamBuffer, m_consumerScene, m_textureConsumer);
            m_renderer.getSceneControlAPI()->flush();

            m_streamBufferNeedsLinking = false;
        }
    }

    ramses::RamsesRenderer& m_renderer;
    ramses::displayId_t m_display;
    ramses::waylandIviSurfaceId_t m_iviSurface;
    bool m_windowClosed = false;

    ramses::streamBufferId_t m_streamBuffer;
    ramses::sceneId_t m_consumerScene;
    ramses::dataConsumerId_t m_textureConsumer;

    bool m_streamBufferNeedsLinking = true;
};

int main(int argc, char* argv[])
{
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    ramses::RendererConfig rendererConfig;
    ramses::DisplayConfig displayConfig;

    config.setRequestedRamsesShellType(ramses::ERamsesShellType::Console);  //needed for automated test of examples

    ramses::waylandIviSurfaceId_t surfaceId(1u);

    CLI::App cli;
    try
    {
        cli.add_option("--stream-buffer-ivi-id", surfaceId.getReference(), "set ivi surface id for the stream buffer")->default_val(1u);

        ramses::registerOptions(cli, config);
        ramses::registerOptions(cli, rendererConfig);
        ramses::registerOptions(cli, displayConfig);
    }
    catch (const CLI::Error& error)
    {
        // configuration error
        std::cerr << error.what();
        return -1;
    }

    CLI11_PARSE(cli, argc, argv);

    std::cout <<  "using wayland ivi surface id: " << surfaceId.getValue() << " for stream buffer" << std::endl;

    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& client(*framework.createClient("ramses-local-compositing-example"));
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    ramses::RendererSceneControl& sceneControlAPI = *renderer.getSceneControlAPI();

    renderer.startThread();

    const ramses::displayId_t display = renderer.createDisplay(displayConfig);
    renderer.flush();

    // create a scene for local content
    constexpr ramses::sceneId_t sceneId(123u);
    ramses::Scene* scene = client.createScene(sceneId);

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
    const std::array<ramses::vec3f, 4u> vertexPositionsData{ ramses::vec3f{-1.0f, -1.0f, -1.0f}, ramses::vec3f{1.0f, -1.0f, -1.0f}, ramses::vec3f{-1.0f, 1.0f, -1.0f}, ramses::vec3f{1.0f, 1.0f, -1.f} };
    ramses::ArrayResource* vertexPositions = scene->createArrayResource(4u, vertexPositionsData.data());

    const std::array<ramses::vec2f, 4u> textureCoordsArray{ ramses::vec2f{0.f, 1.f}, ramses::vec2f{1.f, 1.f}, ramses::vec2f{0.f, 0.f}, ramses::vec2f{1.f, 0.f} };
    ramses::ArrayResource* textureCoords = scene->createArrayResource(4u, textureCoordsArray.data());

    const std::array<uint16_t, 6u> indicesArray{ 0, 1, 2, 2, 1, 3 };
    ramses::ArrayResource* indices = scene->createArrayResource(6u, indicesArray.data());

    /// [Local Compositing Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // texture
    ramses::Texture2D* texture =
        ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-local-compositing-texture.png", *scene);

    ramses::TextureSampler* sampler =
        scene->createTextureSampler(ramses::ETextureAddressMode::Repeat, ramses::ETextureAddressMode::Repeat, ramses::ETextureSamplingMethod::Nearest, ramses::ETextureSamplingMethod::Nearest, *texture);

    constexpr ramses::dataConsumerId_t textureConsumerId(567u);
    scene->createTextureConsumer(*sampler, textureConsumerId);

    /// [Local Compositing Example]

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-local-compositing.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-local-compositing.frag");

    ramses::Effect* effectTex = scene->createEffect(effectDesc, "glsl shader");
    ramses::Appearance* appearance = scene->createAppearance(*effectTex, "triangle appearance");

    // set vertex positions directly in geometry
    ramses::Geometry* geometry = scene->createGeometry(*effectTex, "triangle geometry");
    geometry->setIndices(*indices);
    std::optional<ramses::AttributeInput> positionsInput = effectTex->findAttributeInput("a_position");
    std::optional<ramses::AttributeInput> texcoordsInput = effectTex->findAttributeInput("a_texcoord");
    std::optional<ramses::UniformInput>   textureInput   = effectTex->findUniformInput("textureSampler");
    assert(positionsInput.has_value() && texcoordsInput.has_value() && textureInput.has_value());
    geometry->setInputBuffer(*positionsInput, *vertexPositions);
    geometry->setInputBuffer(*texcoordsInput, *textureCoords);
    appearance->setInputTexture(*textureInput, *sampler);

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = scene->createMeshNode("textured triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometry(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);

    // distribute the scene to RAMSES
    scene->publish(ramses::EScenePublicationMode::LocalOnly);

    // show the scene on the renderer
    sceneControlAPI.setSceneMapping(sceneId, display);
    sceneControlAPI.setSceneState(sceneId, ramses::RendererSceneState::Rendered);
    sceneControlAPI.flush();

    // application logic
    RendererEventHandler eventHandler(renderer, display, surfaceId);
    for (uint32_t i = 0u; i < 6000u; ++i)
    {
        scene->flush(); // flush scene periodically so its data can be transferred to renderer

        renderer.dispatchEvents(eventHandler);
        sceneControlAPI.dispatchEvents(eventHandler);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // shutdown: stop distribution, free resources, unregister
    scene->unpublish();
    scene->destroy(*vertexPositions);
    scene->destroy(*textureCoords);
    scene->destroy(*indices);
    client.destroy(*scene);
    framework.disconnect();

    return 0;
}
