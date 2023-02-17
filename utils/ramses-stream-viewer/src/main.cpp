//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"
#include "ramses-utils.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "ramses-renderer-api/RendererSceneControl.h"

#include <thread>
#include <vector>
#include <algorithm>
#include <cassert>
#include <unordered_map>
#include <iostream>
#include "CLI/CLI.hpp"


constexpr const char* const vertexShader = R"##(
#version 300 es

in vec2 a_position;
void main()
{
    gl_Position = vec4(a_position, 0.0, 1.0);
}
)##";

constexpr const char* const fragmentShader = R"##(
#version 300 es

uniform sampler2D textureSampler;
out highp vec4 fragColor;
void main(void)
{
    highp vec2 ts = vec2(textureSize(textureSampler, 0));
    if(gl_FragCoord.x < ts.x && gl_FragCoord.y < ts.y)
    {
        fragColor = texelFetch(textureSampler,
#ifdef FLIP_Y
                                ivec2(gl_FragCoord.xy),
#else
                                ivec2(gl_FragCoord.x, ts.y-gl_FragCoord.y),
#endif
                                0);
    }
    else
    {
        fragColor = vec4(0.5, 0.3, 0.1, 0.2);
    }
}
)##";


class StreamSourceViewer
{
public:
    StreamSourceViewer(ramses::RamsesClient& ramsesClient, ramses::sceneId_t sceneId, bool flipY, ramses::EScenePublicationMode publicationMode, uint32_t displayWidth, uint32_t displayHeight)
        : m_ramsesClient(ramsesClient)
    {
        ramses::SceneConfig conf;
        m_scene = m_ramsesClient.createScene(sceneId, conf);
        auto camera = m_scene->createPerspectiveCamera("my camera");
        camera->setViewport(0, 0, displayWidth, displayHeight);
        camera->setFrustum(19.f, static_cast<float>(displayWidth) / static_cast<float>(displayHeight), 0.1f, 1500.f);
        camera->setTranslation(0.0f, 0.0f, 5.0f);
        m_renderPass = m_scene->createRenderPass("my render pass");
        m_renderPass->setClearFlags(ramses::EClearFlags_None);
        m_renderPass->setCamera(*camera);
        m_renderGroup = m_scene->createRenderGroup();
        m_renderPass->addRenderGroup(*m_renderGroup);

        // prepare triangle geometry: vertex position array and index array
        float vertexPositionsArray[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};
        m_vertexPositions = m_scene->createArrayResource(ramses::EDataType::Vector2F, 4, vertexPositionsArray);

        uint16_t indicesArray[] = {0, 1, 2, 2, 1, 3};
        m_indices = m_scene->createArrayResource(ramses::EDataType::UInt16, 6, indicesArray);


        static const uint8_t textureData[] = {1u, 1u, 1u, 1u};
        const ramses::MipLevelData mipLevelData(sizeof(textureData), textureData);
        m_texture = m_scene->createTexture2D(ramses::ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache, "");

        ramses::EffectDescription effectDesc;

        effectDesc.setVertexShader(vertexShader);
        effectDesc.setFragmentShader(fragmentShader);
        if (flipY)
            effectDesc.addCompilerDefine("FLIP_Y");
        m_effect = m_scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
        m_scene->flush();
        m_scene->publish(publicationMode);
    }

    void createMesh(ramses::waylandIviSurfaceId_t streamSource)
    {
        const auto meshEntry = createMeshEntry(streamSource);
        addMesh(meshEntry);
        m_scene->flush();
    }

    void removeMesh(ramses::waylandIviSurfaceId_t streamSource)
    {
        auto iter = std::find_if(m_meshes.begin(), m_meshes.end(), [streamSource](const MeshEntry& e){ return e.streamSource == streamSource;});
        assert(m_meshes.end() != iter);

        MeshEntry& meshEntry = *iter;

        m_renderGroup->removeMeshNode(*meshEntry.meshNode);
        m_scene->destroy(*meshEntry.meshNode);
        m_scene->destroy(*meshEntry.appearance);
        m_scene->destroy(*meshEntry.geometryBinding);
        m_scene->destroy(*meshEntry.textureSampler);
        m_scene->destroy(*meshEntry.streamTexture);

        m_meshes.erase(iter);

        m_scene->flush();
    }

private:
    struct MeshEntry
    {
        ramses::waylandIviSurfaceId_t      streamSource{0u};
        ramses::MeshNode*           meshNode = nullptr;
        ramses::Appearance*         appearance = nullptr;
        ramses::GeometryBinding*    geometryBinding = nullptr;
        ramses::StreamTexture*      streamTexture = nullptr;
        ramses::TextureSampler*     textureSampler = nullptr;
    };

    MeshEntry createMeshEntry(ramses::waylandIviSurfaceId_t streamSource)
    {
        MeshEntry meshEntry;
        meshEntry.streamSource = streamSource;
        meshEntry.streamTexture = m_scene->createStreamTexture(*m_texture, streamSource);
        meshEntry.textureSampler = m_scene->createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat, ramses::ETextureSamplingMethod_Nearest, ramses::ETextureSamplingMethod_Nearest, *meshEntry.streamTexture);

        meshEntry.appearance = m_scene->createAppearance(*m_effect);

        meshEntry.geometryBinding = m_scene->createGeometryBinding(*m_effect);
        meshEntry.geometryBinding->setIndices(*m_indices);
        ramses::AttributeInput positionsInput;
        m_effect->findAttributeInput("a_position", positionsInput);
        meshEntry.geometryBinding->setInputBuffer(positionsInput, *m_vertexPositions);

        ramses::UniformInput textureInput;
        m_effect->findUniformInput("textureSampler", textureInput);
        meshEntry.appearance->setInputTexture(textureInput, *meshEntry.textureSampler);

        // create a mesh node to define the triangle with chosen appearance
        meshEntry.meshNode = m_scene->createMeshNode();
        meshEntry.meshNode->setAppearance(*meshEntry.appearance);
        meshEntry.meshNode->setGeometryBinding(*meshEntry.geometryBinding);

        return meshEntry;
    }

    void addMesh(const MeshEntry& meshEntry)
    {
        assert(m_meshes.cend() == std::find_if(m_meshes.cbegin(), m_meshes.cend(), [&meshEntry](const MeshEntry& e){ return e.streamSource == meshEntry.streamSource;}));

        m_meshes.push_back(meshEntry);
        m_renderGroup->addMeshNode(*meshEntry.meshNode);
    }

    using MeshEntries=std::vector<MeshEntry>;

    ramses::RamsesClient& m_ramsesClient;

    ramses::Scene* m_scene                          = nullptr;
    ramses::RenderPass* m_renderPass                = nullptr;
    ramses::RenderGroup* m_renderGroup              = nullptr;
    const ramses::ArrayResource* m_vertexPositions  = nullptr;
    const ramses::ArrayResource* m_indices          = nullptr;
    const ramses::Texture2D* m_texture              = nullptr;
    ramses::Effect* m_effect                        = nullptr;

    MeshEntries m_meshes;
};

class RendererEventHandler : public ramses::RendererEventHandlerEmpty
{
public:
    void windowClosed(ramses::displayId_t /*displayId*/) override
    {
        m_windowClosed = true;
    }

    [[nodiscard]] bool isWindowClosed() const
    {
        return m_windowClosed;
    }

private:
    bool m_windowClosed = false;
};

class RendererSceneControlEventHandler : public ramses::RendererSceneControlEventHandlerEmpty
{
public:
    explicit RendererSceneControlEventHandler(StreamSourceViewer& sceneCreator)
        : m_sceneCreator(sceneCreator)
    {
    }

    virtual void streamAvailabilityChanged(ramses::waylandIviSurfaceId_t streamId, bool available) override
    {
        if(available)
        {
            std::cout << std::endl << std::endl << "Stream " << streamId.getValue() << " available !" << std::endl;
            m_sceneCreator.createMesh(streamId);
        }
        else
        {
            std::cout << std::endl << std::endl << "Stream " << streamId.getValue() << " unavailable !" << std::endl;
            m_sceneCreator.removeMesh(streamId);
        }
    }

private:
    StreamSourceViewer& m_sceneCreator;
};

int main(int argc, char* argv[])
{
    // default configuration
    float maxFps = 60.0f;
    bool  flipY  = false;
    ramses::RamsesFrameworkConfig config;
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
    ramses::RendererConfig rendererConfig;
    ramses::DisplayConfig  displayConfig;
    displayConfig.setClearColor(0.5f, 0.f, 0.f, 1.f);

    CLI::App cli;
    try
    {
        cli.add_option("--fps", maxFps, "Frames per second")->default_val(maxFps);
        cli.add_flag("-y,--flip-y", flipY, "flip received stream vertically (on y-axis)");
        config.registerOptions(cli);
        rendererConfig.registerOptions(cli);
        displayConfig.registerOptions(cli);
    }
    catch (const CLI::Error& error)
    {
        // configuration error
        std::cerr << error.what();
        return -1;
    }

    CLI11_PARSE(cli, argc, argv);

    ramses::RamsesFramework framework(config);
    ramses::RamsesClient* ramsesClient(framework.createClient("stream viewer"));

    ramses::RamsesRenderer* renderer(framework.createRenderer(rendererConfig));
    auto sceneControlAPI = renderer->getSceneControlAPI();

    if (!ramsesClient || !renderer)
    {
        std::cerr << std::endl << "Failed to create either ramses client or ramses renderer." << std::endl;
        return 1;
    }

    renderer->setMaximumFramerate(maxFps);
    renderer->startThread();

    const ramses::displayId_t display = renderer->createDisplay(displayConfig);
    renderer->flush();

    const ramses::sceneId_t sceneId{1u};
    int32_t x;
    int32_t y;
    uint32_t width;
    uint32_t height;
    displayConfig.getWindowRectangle(x, y, width, height);
    StreamSourceViewer sceneCreator(*ramsesClient, sceneId, flipY, ramses::EScenePublicationMode_LocalOnly, width, height);

    sceneControlAPI->setSceneMapping(sceneId, display);
    sceneControlAPI->setSceneState(sceneId, ramses::RendererSceneState::Rendered);
    sceneControlAPI->flush();

    auto eventHandler = std::make_unique<RendererSceneControlEventHandler>(sceneCreator);

    RendererEventHandler rendererEventHandler;
    while (!rendererEventHandler.isWindowClosed())
    {
        renderer->dispatchEvents(rendererEventHandler);
        sceneControlAPI->dispatchEvents(*eventHandler);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
