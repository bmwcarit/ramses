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
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include <unordered_set>

#include <Utils/CommandLineParser.h>
#include <Utils/Argument.h>
#include "RendererLib/RendererConfigUtils.h"

#include <thread>
#include <vector>
#include <algorithm>
#include <assert.h>


class StreamSourceViewer
{
public:
    StreamSourceViewer(ramses::RamsesClient& ramsesClient, ramses::sceneId_t sceneId)
        : m_ramsesClient(ramsesClient)
    {
        m_scene = m_ramsesClient.createScene(sceneId);
        m_camera = m_scene->createRemoteCamera("my camera");
        m_camera->setTranslation(0.0f, 0.0f, 5.0f);
        m_renderPass = m_scene->createRenderPass("my render pass");
        m_renderPass->setClearFlags(ramses::EClearFlags_None);
        m_renderPass->setCamera(*m_camera);
        m_renderGroup = m_scene->createRenderGroup();
        m_renderPass->addRenderGroup(*m_renderGroup);

        // prepare triangle geometry: vertex position array and index array
        float vertexPositionsArray[] = {-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f};
        m_vertexPositions = m_ramsesClient.createConstVector2fArray(4, vertexPositionsArray);

        uint16_t indicesArray[] = {0, 1, 2, 2, 1, 3};
        m_indices = m_ramsesClient.createConstUInt16Array(6, indicesArray);


        static const uint8_t textureData[] = {1u, 1u, 1u, 1u};
        const ramses::MipLevelData mipLevelData(sizeof(textureData), textureData);
        m_texture = ramsesClient.createTexture2D(1u, 1u, ramses::ETextureFormat_RGBA8, 1, &mipLevelData, false, ramses::ResourceCacheFlag_DoNotCache, "");

        ramses::EffectDescription effectDesc;

        effectDesc.setVertexShaderFromFile("res/ramses-stream-viewer.vert");
        effectDesc.setFragmentShaderFromFile("res/ramses-stream-viewer.frag");
        m_effect = m_ramsesClient.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");

        if(nullptr == m_effect)
        {
            printf("\nFailed to create effect using provided shader files. Will use default shaders.\n");

            effectDesc.setVertexShader(R"shaderCode("

                                        #version 300 es
                                        in vec2 a_position;
                                        void main()
                                        {
                                            gl_Position = vec4(a_position, 0.0, 1.0);
                                        }

                                        )shaderCode");

            effectDesc.setFragmentShader(R"shaderCode(

                                        #version 300 es
                                        uniform sampler2D textureSampler;
                                        out highp vec4 fragColor;
                                        void main(void)
                                        {
                                            highp vec2 ts = vec2(textureSize(textureSampler, 0));
                                            if(gl_FragCoord.x < ts.x && gl_FragCoord.y < ts.y)
                                            {
                                                fragColor = texelFetch(textureSampler, ivec2(gl_FragCoord.xy), 0);
                                            }
                                            else
                                            {
                                                fragColor = vec4(0.5, 0.3, 0.1, 0.2);
                                            }
                                        }


                                        )shaderCode");

            m_effect = m_ramsesClient.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
        }

        m_scene->flush();
        m_scene->publish();
    }

    void createMesh(ramses::streamSource_t streamSource)
    {
        const auto meshEntry = createMeshEntry(streamSource);
        addMesh(meshEntry);
        m_scene->flush();
    }

    void removeMesh(ramses::streamSource_t streamSource)
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
        ramses::streamSource_t      streamSource{0u};
        ramses::MeshNode*           meshNode = nullptr;
        ramses::Appearance*         appearance = nullptr;
        ramses::GeometryBinding*    geometryBinding = nullptr;
        ramses::StreamTexture*      streamTexture = nullptr;
        ramses::TextureSampler*     textureSampler = nullptr;
    };

    MeshEntry createMeshEntry(ramses::streamSource_t streamSource)
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
    ramses::Camera* m_camera                        = nullptr;
    ramses::RenderPass* m_renderPass                = nullptr;
    ramses::RenderGroup* m_renderGroup              = nullptr;
    const ramses::Vector2fArray* m_vertexPositions  = nullptr;
    const ramses::UInt16Array* m_indices            = nullptr;
    const ramses::Texture2D* m_texture              = nullptr;
    ramses::Effect* m_effect                        = nullptr;

    MeshEntries m_meshes;
};

class RendererEventHandler : public ramses::RendererEventHandlerEmpty
{
public:
    RendererEventHandler(ramses::RamsesRenderer& renderer, StreamSourceViewer& sceneCreator)
        : m_renderer(renderer)
        , m_sceneCreator(sceneCreator)
    {
    }

    virtual void scenePublished(ramses::sceneId_t sceneId) override
    {
        m_publishedScenes.insert(sceneId);
    }

    virtual void sceneUnpublished(ramses::sceneId_t sceneId) override
    {
        m_publishedScenes.erase(sceneId);
    }

    virtual void sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_subscribedScenes.insert(sceneId);
        }
    }

    virtual void sceneUnsubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_FAIL != result)
        {
            m_subscribedScenes.erase(sceneId);
        }
    }

    virtual void sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_mappedScenes.insert(sceneId);
        }
    }

    virtual void sceneUnmapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_FAIL != result)
        {
            m_mappedScenes.erase(sceneId);
        }
    }

    void waitForPublication(const ramses::sceneId_t sceneId)
    {
        waitForSceneInSet(sceneId, m_publishedScenes);
    }

    void waitForSubscription(const ramses::sceneId_t sceneId)
    {
        waitForSceneInSet(sceneId, m_subscribedScenes);
    }

    void waitForMapped(const ramses::sceneId_t sceneId)
    {
        waitForSceneInSet(sceneId, m_mappedScenes);
    }

    virtual void streamAvailabilityChanged(ramses::streamSource_t streamId, bool available) override
    {
        if(available)
        {
            printf("\n\nStream %u available !\n", streamId.getValue());
            m_sceneCreator.createMesh(streamId);
        }
        else
        {
            printf("\n\nStream %u unavailable !\n", streamId.getValue());
            m_sceneCreator.removeMesh(streamId);
        }
    }

private:
    typedef std::unordered_set<ramses::sceneId_t> SceneSet;

    void waitForSceneInSet(const ramses::sceneId_t sceneId, const SceneSet& sceneSet)
    {
        while (sceneSet.find(sceneId) == sceneSet.end())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            m_renderer.dispatchEvents(*this);
        }
    }

    SceneSet m_publishedScenes;
    SceneSet m_subscribedScenes;
    SceneSet m_mappedScenes;

    ramses::RamsesRenderer& m_renderer;
    StreamSourceViewer& m_sceneCreator;
};

int main(int argc, char* argv[])
{
    ramses_internal::CommandLineParser parser(argc, argv);
    ramses_internal::ArgumentBool      helpRequested(parser, "help", "help", false);
    ramses_internal::ArgumentFloat     maxFps(parser, "fps", "framesPerSecond", 60.0f);

    if (helpRequested)
    {
        ramses_internal::RendererConfigUtils::PrintCommandLineOptions();
        return 0;
    }

    ramses::RamsesFrameworkConfig config(argc, argv);
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);
    ramses::RamsesFramework framework(config);

    ramses::RamsesClient ramsesClient("stream viewer", framework);

    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer renderer(framework, rendererConfig);
    renderer.setMaximumFramerate(maxFps);
    renderer.startThread();

    ramses::DisplayConfig displayConfig(argc, argv);
    displayConfig.setClearColor(0.5f, 0.f, 0.f, 1.f);
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);

    framework.connect();

    const ramses::sceneId_t sceneId = 1u;
    StreamSourceViewer sceneCreator(ramsesClient, sceneId);

    RendererEventHandler eventHandler(renderer, sceneCreator);

    eventHandler.waitForPublication(sceneId);

    renderer.subscribeScene(sceneId);
    renderer.flush();
    eventHandler.waitForSubscription(sceneId);

    renderer.mapScene(display, sceneId);
    renderer.flush();
    eventHandler.waitForMapped(sceneId);

    renderer.showScene(sceneId);
    renderer.flush();

    for (;;)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        renderer.dispatchEvents(eventHandler);
    }
}
