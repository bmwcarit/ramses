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
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-client-api/DataVector4f.h"
#include "ramses-utils.h"
#include <unordered_set>
#include <cmath>
#include <chrono>
#include <thread>

/** \cond HIDDEN_SYMBOLS */
class SceneStateEventHandler : public ramses::RendererEventHandlerEmpty
{
public:
    SceneStateEventHandler(ramses::RamsesRenderer& renderer)
        : m_renderer(renderer)
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

private:
    typedef std::unordered_set<ramses::sceneId_t> SceneSet;

    void waitForSceneInSet(const ramses::sceneId_t sceneId, const SceneSet& sceneSet)
    {
        while (sceneSet.find(sceneId) == sceneSet.end())
        {
            m_renderer.doOneLoop();
            m_renderer.dispatchEvents(*this);
        }
    }

    SceneSet m_publishedScenes;
    SceneSet m_subscribedScenes;
    SceneSet m_mappedScenes;

    ramses::RamsesRenderer& m_renderer;
};
/** \endcond */

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

TriangleSceneInfo* createTriangleSceneContent(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    TriangleSceneInfo* sceneInfo = new TriangleSceneInfo();

    sceneInfo->scene = client.createScene(sceneId, ramses::SceneConfig(), "triangle scene");

    // every scene needs a render pass with camera
    ramses::Camera* camera = sceneInfo->scene->createRemoteCamera("my camera");
    ramses::RenderPass* renderPass = sceneInfo->scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = sceneInfo->scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsArray[] = { -0.25f, -0.125f, 0.f, 0.25f, -0.125f, 0.f, 0.f, 0.125f, 0.f };
    const ramses::Vector3fArray* vertexPositions = client.createConstVector3fArray(3, vertexPositionsArray);
    uint16_t indicesArray[] = { 0, 1, 2 };
    const ramses::UInt16Array* indices = client.createConstUInt16Array(3, indicesArray);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-local-datalink.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-local-datalink.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    const ramses::Effect* effect = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
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
    sceneInfo->textures[0] = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-local-datalink-texture.png", client);
    sceneInfo->textures[1] = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-local-datalink-texture2.png", client);

    return sceneInfo;
}

QuadSceneInfo* createQuadSceneContent(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    QuadSceneInfo* sceneInfo = new QuadSceneInfo();

    sceneInfo->scene = client.createScene(sceneId, ramses::SceneConfig(), "quad scene");

    // every scene needs a render pass with camera
    ramses::Camera* camera = sceneInfo->scene->createRemoteCamera("my camera");
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
    const ramses::Vector3fArray* vertexPositions = client.createConstVector3fArray(4, vertexPositionsArray);
    float texCoordsArray[] = { 1.0f, 1.0f,
                                0.0f, 1.0f,
                                0.0f, 0.0f,
                                1.0f, 0.0f };
    const ramses::Vector2fArray* texCoords = client.createConstVector2fArray(4, texCoordsArray);
    uint16_t indicesArray[] = { 0, 1, 2, 2, 3, 0 };
    const ramses::UInt16Array* indices = client.createConstUInt16Array(6, indicesArray);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-local-datalink-texturing.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-local-datalink-texturing.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    const ramses::Effect* effect = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
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
    ramses::Texture2D* fallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-local-datalink-texture-fallback.png", client);
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
    ramses::RamsesClient client("ramses-local-datalink-example", framework);

    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer renderer(framework, rendererConfig);

    ramses::DisplayConfig displayConfig;
    displayConfig.setIntegrityRGLDeviceUnit(0);
    displayConfig.setWaylandIviSurfaceID(0);
    displayConfig.setWaylandIviLayerID(3);
    displayConfig.setWindowIviVisible();
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);

    const ramses::sceneId_t triangleSceneId = 1u;
    const ramses::sceneId_t quadSceneId = 2u;
    const ramses::sceneId_t quadSceneId2 = 3u;
    TriangleSceneInfo* triangleInfo = createTriangleSceneContent(client, triangleSceneId);
    QuadSceneInfo* quadInfo = createQuadSceneContent(client, quadSceneId);
    QuadSceneInfo* quadInfo2 = createQuadSceneContent(client, quadSceneId2);

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

    SceneStateEventHandler eventHandler(renderer);

    eventHandler.waitForPublication(triangleSceneId);
    eventHandler.waitForPublication(quadSceneId);
    eventHandler.waitForPublication(quadSceneId2);

    renderer.subscribeScene(triangleSceneId);
    renderer.subscribeScene(quadSceneId);
    renderer.subscribeScene(quadSceneId2);
    renderer.flush();
    eventHandler.waitForSubscription(triangleSceneId);
    eventHandler.waitForSubscription(quadSceneId);
    eventHandler.waitForSubscription(quadSceneId2);

    renderer.mapScene(display, triangleSceneId, 0);
    renderer.mapScene(display, quadSceneId, 1);
    renderer.mapScene(display, quadSceneId2, 2);
    renderer.flush();
    eventHandler.waitForMapped(triangleSceneId);
    eventHandler.waitForMapped(quadSceneId);
    eventHandler.waitForMapped(quadSceneId2);

    renderer.showScene(triangleSceneId);
    renderer.showScene(quadSceneId);
    renderer.showScene(quadSceneId2);
    renderer.flush();

    /// [Data Linking Example Renderer]
    // link transformation
    renderer.linkData(triangleSceneId, transformationProviderId, quadSceneId, transformationConsumerId);
    renderer.linkData(triangleSceneId, transformationProviderId, quadSceneId2, transformationConsumerId2);
    // link data
    renderer.linkData(quadSceneId, dataProviderId, triangleSceneId, dataConsumerId);
    // link texture
    renderer.linkData(triangleSceneId, textureProviderId, quadSceneId2, textureConsumerId);
    /// [Data Linking Example Renderer]

    renderer.flush();

    // run animation
    uint32_t textureId = 0;
    uint64_t timeStamp = 0u;
    for (;;)
    {
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
