//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/RendererSceneControl.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-client-api/DataVector4f.h"
#include "ramses-utils.h"
#include <unordered_set>
#include <cmath>
#include <chrono>
#include <memory>
#include <thread>
#include <unordered_set>

/**
 * @example ramses-example-local-datalink/src/main.cpp
 * @brief Local Data Link Example
 */

class RendererAndSceneStateEventHandler : public ramses::RendererEventHandlerEmpty, public ramses::RendererSceneControlEventHandlerEmpty
{
public:
    RendererAndSceneStateEventHandler(ramses::RendererSceneControl& sceneControlApi, ramses::RamsesRenderer& renderer)
        : m_sceneControlApi(sceneControlApi)
        , m_renderer(renderer)
    {
    }

    void dataProviderCreated(ramses::sceneId_t sceneId, ramses::dataProviderId_t) override
    {
        scenesWithCreatedProviderOrConsumer.insert(sceneId);
    }

    void dataConsumerCreated(ramses::sceneId_t sceneId, ramses::dataConsumerId_t) override
    {
        scenesWithCreatedProviderOrConsumer.insert(sceneId);
    }

    virtual void sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state) override
    {
        if (state == ramses::RendererSceneState::Ready)
            mappedScenes.insert(sceneId);
    }

    void waitForSceneReady(ramses::sceneId_t sceneId)
    {
        waitUntilOrTimeout([&] { return mappedScenes.count(sceneId) != 0; });
    }

    void waitForDataProviderOrConsumerCreated(ramses::sceneId_t sceneId)
    {
        while (scenesWithCreatedProviderOrConsumer.find(sceneId) == scenesWithCreatedProviderOrConsumer.end())
        {
            m_renderer.doOneLoop();
            m_sceneControlApi.dispatchEvents(*this);
        }
    }

    void windowClosed(ramses::displayId_t) override
    {
        m_windowClosed = true;
    }

    bool isWindowClosed() const
    {
        return m_windowClosed;
    }

private:
    bool waitUntilOrTimeout(const std::function<bool()>& conditionFunction)
    {
        const std::chrono::steady_clock::time_point timeoutTS = std::chrono::steady_clock::now() + std::chrono::seconds{ 5 };
        while (!conditionFunction() && std::chrono::steady_clock::now() < timeoutTS)
        {
            m_renderer.doOneLoop();
            m_renderer.dispatchEvents(*this);
            m_sceneControlApi.dispatchEvents(*this);
        }

        return conditionFunction();
    }

    ramses::RendererSceneControl& m_sceneControlApi;
    ramses::RamsesRenderer& m_renderer;
    std::unordered_set<ramses::sceneId_t> mappedScenes;
    std::unordered_set<ramses::sceneId_t> scenesWithCreatedProviderOrConsumer;
    bool m_windowClosed = false;
};

// Helper struct for triangle scene
struct TriangleSceneInfo
{
public:
    // Node that will be used as transformation provider
    ramses::Node* translateNode;
    // Textures that will be used as texture provider
    ramses::Texture2D* textures[2];
    // Data object that will be used as data consumer
    ramses::DataVector4f* colorData;
    // Provider scene
    ramses::Scene* scene;
};

// Helper struct for quad scene
struct QuadSceneInfo
{
    // Consumer node that will use transformation from provider node
    ramses::Node* consumerNode;
    // Consumer texture sampler, that will use texture from provider
    ramses::TextureSampler* textureSampler;
    // Additional rotation chained after consumed transformation
    ramses::Node* rotateNode;
    // Data object that will be used as data provider
    ramses::DataVector4f* colorData;
    // Consumer scene
    ramses::Scene* scene;
};

std::unique_ptr<TriangleSceneInfo> createTriangleSceneContent(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    auto sceneInfo = std::make_unique<TriangleSceneInfo>();

    sceneInfo->scene = client.createScene(sceneId, ramses::SceneConfig(), "triangle scene");

    // every scene needs a render pass with camera
    auto* camera = sceneInfo->scene->createPerspectiveCamera("my camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    ramses::RenderPass* renderPass = sceneInfo->scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = sceneInfo->scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsArray[] = { -0.25f, -0.125f, 0.f, 0.25f, -0.125f, 0.f, 0.f, 0.125f, 0.f };
    ramses::ArrayResource* vertexPositions = sceneInfo->scene->createArrayResource(ramses::EDataType::Vector3F, 3, vertexPositionsArray);
    uint16_t indicesArray[] = { 0, 1, 2 };
    ramses::ArrayResource* indices = sceneInfo->scene->createArrayResource(ramses::EDataType::UInt16, 3, indicesArray);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-local-datalink.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-local-datalink.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effect = sceneInfo->scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = sceneInfo->scene->createAppearance(*effect, "triangle appearance");
    appearance->setCullingMode(ramses::ECullMode_Disabled);
    appearance->setDepthFunction(ramses::EDepthFunc_Always);

    ramses::GeometryBinding* geometry = sceneInfo->scene->createGeometryBinding(*effect, "triangle geometry");
    geometry->setIndices(*indices);

    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);
    appearance->setInputValueVector4f(colorInput, 1.0f, 0.0f, 0.3f, 1.0f);
    //bind input to data object
    sceneInfo->colorData = sceneInfo->scene->createDataVector4f("colorData");
    sceneInfo->colorData->setValue(1.0f, 0.0f, 0.3f, 1.0f);
    appearance->bindInput(colorInput, *sceneInfo->colorData);

    ramses::Node* rootTranslation = sceneInfo->scene->createNode("root scene translation node");
    rootTranslation->setTranslation(0.0f, 0.0f, -1.0f);

    sceneInfo->translateNode = sceneInfo->scene->createNode("triangle translation node");
    sceneInfo->translateNode->setParent(*rootTranslation);

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = sceneInfo->scene->createMeshNode("triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*geometry);
    meshNode->setParent(*sceneInfo->translateNode);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);

    // Create provided texture
    sceneInfo->textures[0] = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-local-datalink-texture.png", *sceneInfo->scene);
    sceneInfo->textures[1] = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-local-datalink-texture2.png", *sceneInfo->scene);

    return sceneInfo;
}

std::unique_ptr<QuadSceneInfo> createQuadSceneContent(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    auto sceneInfo = std::make_unique<QuadSceneInfo>();

    sceneInfo->scene = client.createScene(sceneId, ramses::SceneConfig(), "quad scene");

    // every scene needs a render pass with camera
    auto* camera = sceneInfo->scene->createPerspectiveCamera("my camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    ramses::RenderPass* renderPass = sceneInfo->scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = sceneInfo->scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsArray[] = { -0.1f, -0.1f, 0.0f,
                                    0.1f, -0.1f, 0.0f,
                                    0.1f, 0.1f, 0.0f,
                                    -0.1f, 0.1f, 0.0f };
    ramses::ArrayResource* vertexPositions = sceneInfo->scene->createArrayResource(ramses::EDataType::Vector3F, 4, vertexPositionsArray);
    float texCoordsArray[] = { 1.0f, 1.0f,
                                0.0f, 1.0f,
                                0.0f, 0.0f,
                                1.0f, 0.0f };
    ramses::ArrayResource* texCoords = sceneInfo->scene->createArrayResource(ramses::EDataType::Vector2F, 4, texCoordsArray);
    uint16_t indicesArray[] = { 0, 1, 2, 2, 3, 0 };
    ramses::ArrayResource* indices = sceneInfo->scene->createArrayResource(ramses::EDataType::UInt16, 6, indicesArray);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-local-datalink-texturing.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-local-datalink-texturing.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effect = sceneInfo->scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = sceneInfo->scene->createAppearance(*effect, "quad appearance");
    appearance->setDepthFunction(ramses::EDepthFunc_Always);
    appearance->setCullingMode(ramses::ECullMode_Disabled);

    ramses::GeometryBinding* geometry = sceneInfo->scene->createGeometryBinding(*effect, "quad geometry");
    geometry->setIndices(*indices);

    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    ramses::AttributeInput texCoordsInput;
    effect->findAttributeInput("a_texcoord", texCoordsInput);
    geometry->setInputBuffer(texCoordsInput, *texCoords);

    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);
    appearance->setInputValueVector4f(colorInput, 1.0f, 1.0f, 1.0f, 1.0f);

    ramses::UniformInput textureInput;
    effect->findUniformInput("textureSampler", textureInput);
    ramses::Texture2D* fallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-local-datalink-texture-fallback.png", *sceneInfo->scene);
    sceneInfo->textureSampler = sceneInfo->scene->createTextureSampler(ramses::ETextureAddressMode_Clamp, ramses::ETextureAddressMode_Clamp,
        ramses::ETextureSamplingMethod_Linear, ramses::ETextureSamplingMethod_Linear, *fallbackTexture);
    appearance->setInputTexture(textureInput, *sceneInfo->textureSampler);

    sceneInfo->consumerNode = sceneInfo->scene->createNode("quad root node");
    sceneInfo->rotateNode = sceneInfo->scene->createNode("");

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = sceneInfo->scene->createMeshNode("quad mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*geometry);
    meshNode->setParent(*sceneInfo->rotateNode);

    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);
    sceneInfo->rotateNode->setParent(*sceneInfo->consumerNode);

    //create data object for providing color info (not used by any appearance)
    sceneInfo->colorData = sceneInfo->scene->createDataVector4f("colorData");
    sceneInfo->colorData->setValue(1.0f, 1.0f, 1.0f, 1.0f);

    return sceneInfo;
}

int main(int argc, char* argv[])
{
    //Ramses client
    ramses::RamsesFrameworkConfig config(argc, argv);
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);  //needed for automated test of examples
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& client(*framework.createClient("ramses-local-datalink-example"));

    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));

    ramses::DisplayConfig displayConfig(argc, argv);
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);
    renderer.flush();
    auto& sceneControlAPI = *renderer.getSceneControlAPI();

    const ramses::sceneId_t triangleSceneId{1u};
    const ramses::sceneId_t quadSceneId{2u};
    const ramses::sceneId_t quadSceneId2{3u};
    std::unique_ptr<TriangleSceneInfo> triangleInfo = createTriangleSceneContent(client, triangleSceneId);
    std::unique_ptr<QuadSceneInfo> quadInfo = createQuadSceneContent(client, quadSceneId);
    std::unique_ptr<QuadSceneInfo> quadInfo2 = createQuadSceneContent(client, quadSceneId2);

    ramses::Scene* triangleScene = triangleInfo->scene;
    ramses::Scene* quadScene = quadInfo->scene;
    ramses::Scene* quadScene2 = quadInfo2->scene;
    ramses::Node* providerNode = triangleInfo->translateNode;
    ramses::Node* consumerNode = quadInfo->consumerNode;
    ramses::Node* consumerNode2 = quadInfo2->consumerNode;

    /// [Data Linking Example Client]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // Transformation links
    ramses::dataProviderId_t transformationProviderId(14u);
    ramses::dataConsumerId_t transformationConsumerId(12u);
    ramses::dataConsumerId_t transformationConsumerId2(87u);

    triangleScene->createTransformationDataProvider(*providerNode, transformationProviderId);
    quadScene->createTransformationDataConsumer(*consumerNode, transformationConsumerId);
    quadScene2->createTransformationDataConsumer(*consumerNode2, transformationConsumerId2);

    //Data links
    ramses::dataProviderId_t dataProviderId(100u);
    ramses::dataConsumerId_t dataConsumerId(101u);

    triangleScene->createDataConsumer(*triangleInfo->colorData, dataConsumerId);
    quadScene->createDataProvider(*quadInfo->colorData, dataProviderId);

    //Texture links
    ramses::dataProviderId_t textureProviderId(200u);
    ramses::dataConsumerId_t textureConsumerId(201u);

    triangleScene->createTextureProvider(*triangleInfo->textures[0], textureProviderId);
    quadScene2->createTextureConsumer(*quadInfo2->textureSampler, textureConsumerId);

    triangleScene->flush();
    quadScene->flush();
    quadScene2->flush();

    /// [Data Linking Example Client]
    framework.connect();
    triangleScene->publish();
    quadScene->publish();
    quadScene2->publish();

    const auto fbId = renderer.getDisplayFramebuffer(display);

    sceneControlAPI.setSceneMapping(triangleSceneId, display);
    sceneControlAPI.setSceneMapping(quadSceneId, display);
    sceneControlAPI.setSceneMapping(quadSceneId2, display);
    sceneControlAPI.flush();

    sceneControlAPI.setSceneDisplayBufferAssignment(triangleSceneId, fbId, 0);
    sceneControlAPI.setSceneDisplayBufferAssignment(quadSceneId, fbId, 1);
    sceneControlAPI.setSceneDisplayBufferAssignment(quadSceneId2, fbId, 2);

    sceneControlAPI.setSceneState(triangleSceneId, ramses::RendererSceneState::Rendered);
    sceneControlAPI.setSceneState(quadSceneId, ramses::RendererSceneState::Rendered);
    sceneControlAPI.setSceneState(quadSceneId2, ramses::RendererSceneState::Rendered);

    sceneControlAPI.flush();

    RendererAndSceneStateEventHandler eventHandler(sceneControlAPI, renderer);
    eventHandler.waitForSceneReady(triangleSceneId);
    eventHandler.waitForSceneReady(quadSceneId);
    eventHandler.waitForSceneReady(quadSceneId2);
    eventHandler.waitForDataProviderOrConsumerCreated(triangleSceneId);
    eventHandler.waitForDataProviderOrConsumerCreated(quadSceneId);
    eventHandler.waitForDataProviderOrConsumerCreated(quadSceneId2);

    /// [Data Linking Example Renderer]
    // link transformation
    sceneControlAPI.linkData(triangleSceneId, transformationProviderId, quadSceneId, transformationConsumerId);
    sceneControlAPI.linkData(triangleSceneId, transformationProviderId, quadSceneId2, transformationConsumerId2);
    // link data
    sceneControlAPI.linkData(quadSceneId, dataProviderId, triangleSceneId, dataConsumerId);
    // link texture
    sceneControlAPI.linkData(triangleSceneId, textureProviderId, quadSceneId2, textureConsumerId);
    /// [Data Linking Example Renderer]

    sceneControlAPI.flush();

    // run animation
    uint32_t textureId = 0;
    uint64_t timeStamp = 0u;
    while (!eventHandler.isWindowClosed())
    {
        renderer.dispatchEvents(eventHandler);
        if (timeStamp % 100 == 0)
        {
            textureId = (1 - textureId);
            triangleInfo->scene->updateTextureProvider(*triangleInfo->textures[textureId], textureProviderId);
        }

        triangleInfo->translateNode->setTranslation(std::sin(timeStamp * 0.05f) * 0.2f, 0.0f, 0.0f);
        triangleInfo->scene->flush();
        quadInfo->rotateNode->rotate(0.0f, 0.0f, 1.0f);
        quadInfo->scene->flush();
        quadInfo2->rotateNode->rotate(0.0f, 1.0f, 0.0f);
        quadInfo2->scene->flush();

        quadInfo->colorData->setValue(std::sin(timeStamp * 0.1f), 0.0f, 0.5f, 1.0f);

        renderer.doOneLoop();
        timeStamp++;
        std::this_thread::sleep_for(std::chrono::milliseconds{ 15u });
    }
}
