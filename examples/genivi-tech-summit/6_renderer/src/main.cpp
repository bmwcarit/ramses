//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <thread>
#include <array>
#include <vector>

#include "ramses-client.h"
#include "ramses-utils.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererEventHandler.h"

using Vertex = std::array<float, 3>;

void addTexturedQuad(const std::array<Vertex, 4>& vertices,
    ramses::RamsesClient& ramsesClient, ramses::Scene& scene, const char* textureFile, ramses::RenderGroup& renderGroup,
    ramses::GroupNode& groupNode, ramses::Effect& effect, const ramses::Vector2fArray& textureCoordsArray)
{
    std::vector<float> vertexData;
    vertexData.insert(vertexData.end(), vertices[0].begin(), vertices[0].end());
    vertexData.insert(vertexData.end(), vertices[1].begin(), vertices[1].end());
    vertexData.insert(vertexData.end(), vertices[2].begin(), vertices[2].end());
    vertexData.insert(vertexData.end(), vertices[3].begin(), vertices[3].end());

    const ramses::Vector3fArray* vertexArray = ramsesClient.createConstVector3fArray(4, vertexData.data());

    ramses::Texture2D* fallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng(textureFile, ramsesClient);

    ramses::TextureSampler* sampler = scene.createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat,
        ramses::ETextureSamplingMethod_Bilinear, *fallbackTexture);

    ramses::AttributeInput positionAttributeInput;
    effect.findAttributeInput("a_position", positionAttributeInput);

    ramses::AttributeInput textCoordInput;
    effect.findAttributeInput("a_texcoord", textCoordInput);

    ramses::UniformInput textureSamplerInput;
    effect.findUniformInput("textureSampler", textureSamplerInput);

    ramses::Appearance* appearance = scene.createAppearance(effect);
    appearance->setDrawMode(ramses::EDrawMode_TriangleStrip);
    appearance->setInputTexture(textureSamplerInput, *sampler);

    ramses::GeometryBinding* geometrybinding = scene.createGeometryBinding(effect);
    geometrybinding->setInputBuffer(positionAttributeInput, *vertexArray);
    geometrybinding->setInputBuffer(textCoordInput, textureCoordsArray);

    ramses::MeshNode* mesh = scene.createMeshNode();
    mesh->setGeometryBinding(*geometrybinding);
    mesh->setAppearance(*appearance);
    mesh->setIndexCount(4);

    mesh->setParent(groupNode);
    renderGroup.addMeshNode(*mesh);
}

class SimpleSceneStateEventHandler : public ramses::RendererEventHandlerEmpty
{
public:
    SimpleSceneStateEventHandler(ramses::RamsesRenderer& renderer, ramses::sceneId_t scene)
        : m_renderer(renderer)
        , m_scene(scene)
    {
    }

    virtual void scenePublished(ramses::sceneId_t sceneId) override
    {
        if (sceneId == m_scene)
        {
            m_published = true;
        }
    }

    virtual void sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_OK == result && sceneId == m_scene)
        {
            m_subscribed = true;
        }
    }

    virtual void sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_OK == result && sceneId == m_scene)
        {
            m_mapped = true;
        }
    }

    void waitForPublication()
    {
        while (!m_published)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            m_renderer.dispatchEvents(*this);
        }
    }

    void waitForSubscription()
    {
        while (!m_subscribed)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            m_renderer.dispatchEvents(*this);
        }
    }

    void waitForMapped()
    {
        while (!m_mapped)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            m_renderer.dispatchEvents(*this);
        }
    }

private:
    ramses::RamsesRenderer& m_renderer;
    const ramses::sceneId_t m_scene;

    bool m_published = false;
    bool m_subscribed = false;
    bool m_mapped = false;
};

int main(int argc, char* argv[])
{
    int instance = 0;
    if (argc > 1)
    {
        instance = atoi(argv[1]);
    }

    ramses::RamsesFramework framework(argc, argv);

    ramses::RamsesClient client("workshop example client and renderer", framework);

    framework.connect();

    ramses::sceneId_t sceneId(123);
    ramses::Scene* scene = client.createScene(sceneId);

    ramses::RenderPass* renderPass = scene->createRenderPass();

    ramses::Camera* camera = scene->createRemoteCamera();
    renderPass->setCamera(*camera);
    camera->translate(0, 0, 5);

    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    ramses::EffectDescription effectDescription;
    effectDescription.setVertexShaderFromFile("res/renderer-texturing.vert");
    effectDescription.setFragmentShaderFromFile("res/renderer-texturing.frag");
    effectDescription.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);
    ramses::Effect* effect = client.createEffect(effectDescription);

    const float textureCoordsData[] = {
        0.0, 1.0,
        1.0, 1.0,
        0.0, 0.0,
        1.0, 0.0
    };
    const ramses::Vector2fArray* textureCoordsArray = client.createConstVector2fArray(4, textureCoordsData);

    ramses::GroupNode* groupNode = scene->createGroupNode();

    /* prepare triangle geometry for the cube faces
    *
    *    H-------G
    *   /|      /|
    *  C-------D |     2-top-3                          +y|   /-z
    *  | |     | |     | \   | per side                   |  /
    *  | F-----|-E     |   \ | vertex strip order         | /
    *  |/      |/      0-bot-1                            |/_____ +x
    *  A-------B
    */

    const Vertex A = { -0.5f, -0.5f,  0.5f };
    const Vertex B = {  0.5f, -0.5f,  0.5f };
    const Vertex C = { -0.5f,  0.5f,  0.5f };
    const Vertex D = {  0.5f,  0.5f,  0.5f };
    const Vertex E = {  0.5f, -0.5f, -0.5f };
    const Vertex F = { -0.5f, -0.5f, -0.5f };
    const Vertex G = {  0.5f,  0.5f, -0.5f };
    const Vertex H = { -0.5f,  0.5f, -0.5f };

    addTexturedQuad({ A, B, C, D }, client, *scene, "res/renderer-face-1.png", *renderGroup, *groupNode, *effect, *textureCoordsArray);
    addTexturedQuad({ B, E, D, G }, client, *scene, "res/renderer-face-2.png", *renderGroup, *groupNode, *effect, *textureCoordsArray);
    addTexturedQuad({ C, D, H, G }, client, *scene, "res/renderer-face-3.png", *renderGroup, *groupNode, *effect, *textureCoordsArray);
    addTexturedQuad({ E, F, G, H }, client, *scene, "res/renderer-face-1.png", *renderGroup, *groupNode, *effect, *textureCoordsArray);
    addTexturedQuad({ F, A, H, C }, client, *scene, "res/renderer-face-2.png", *renderGroup, *groupNode, *effect, *textureCoordsArray);
    addTexturedQuad({ F, E, A, B }, client, *scene, "res/renderer-face-3.png", *renderGroup, *groupNode, *effect, *textureCoordsArray);

    groupNode->rotate(-20.0f, 30.0f, 0.0f);

    if (instance == 1)
    {
        groupNode->translate(-1, 0, 0);
    }
    else if (instance == 2)
    {
        groupNode->translate(1, 0, 0);
    }

    scene->flush(ramses::ESceneFlushMode_SynchronizedWithResources);
    scene->publish(ramses::EScenePublicationMode_LocalAndRemote);

    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer renderer(framework, rendererConfig);
    renderer.startThread();

    const ramses::displayId_t display = renderer.createDisplay(ramses::DisplayConfig());

    SimpleSceneStateEventHandler eventHandler(renderer, sceneId);
    eventHandler.waitForPublication();

    renderer.subscribeScene(sceneId);
    renderer.flush();
    eventHandler.waitForSubscription();

    renderer.mapScene(display, sceneId);
    renderer.flush();
    eventHandler.waitForMapped();

    renderer.showScene(sceneId);
    renderer.flush();

    while (1)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    scene->unpublish();
    client.destroy(*scene);
    framework.disconnect();

    return 0;
}
