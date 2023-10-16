//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/ramses-client.h"

#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/renderer/IRendererEventHandler.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "ramses/renderer/RendererSceneControl.h"
#include "ramses/client/ramses-utils.h"
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <cstring>

/**
 * @example ramses-example-local-offscreenbuffer/src/main.cpp
 * @brief Offscreen Buffer Example
 */

/** \cond HIDDEN_SYMBOLS */
class SceneStateEventHandler : public ramses::RendererEventHandlerEmpty, public ramses::RendererSceneControlEventHandlerEmpty
{
public:
    explicit SceneStateEventHandler(ramses::RamsesRenderer& renderer)
        : m_renderer(renderer)
    {
    }

    void sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state) override
    {
        m_scenes[sceneId].state = state;
    }

    void sceneFlushed(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersion) override
    {
        m_scenes[sceneId].version = sceneVersion;
    }

    void offscreenBufferCreated(ramses::displayId_t /*displayId*/, ramses::displayBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult::Failed != result)
        {
            m_createdOffscreenBuffers.insert(offscreenBufferId);
        }
    }

    void offscreenBufferLinked(ramses::displayBufferId_t /*offscreenBufferId*/, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t /*consumerId*/, bool success) override
    {
        if (success)
        {
            m_scenesConsumingOffscreenBuffer[consumerScene].state = ramses::RendererSceneState::Unavailable;
        }
    }

    void waitForOffscreenBufferCreated(const ramses::displayBufferId_t offscreenBufferId)
    {
        waitForElementInSet(offscreenBufferId, m_createdOffscreenBuffers);
    }

    void waitForOffscreenBufferLinkedToConsumingScene(const ramses::sceneId_t sceneId)
    {
        waitUntilOrTimeout([&] {return m_scenesConsumingOffscreenBuffer.count(sceneId) > 0; });
    }

    void waitForSceneState(ramses::Scene& scene, ramses::RendererSceneState state)
    {
        waitUntilOrTimeout([&] { return m_scenes[scene.getSceneId()].state == state; }, &scene);
    }

    bool waitForFlush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersion)
    {
        return waitUntilOrTimeout([&] { return m_scenes[sceneId].version == sceneVersion; });
    }

    bool waitUntilOrTimeout(const std::function<bool()>& conditionFunction, ramses::Scene* scene = nullptr)
    {
        const std::chrono::steady_clock::time_point timeoutTS = std::chrono::steady_clock::now() + std::chrono::seconds{ 5 };
        while (!conditionFunction() && std::chrono::steady_clock::now() < timeoutTS)
        {
            if (scene)
                scene->flush(); // local only scenes have to be flushed periodically when getting scene to READY state

            m_renderer.dispatchEvents(*this);
            m_renderer.getSceneControlAPI()->dispatchEvents(*this);

            std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });  // will give the renderer time to process changes
        }

        return conditionFunction();
    }

    void windowClosed(ramses::displayId_t /*displayId*/) override
    {
        m_windowClosed = true;
    }

    bool isWindowClosed() const
    {
        return m_windowClosed;
    }

private:
    struct SceneInfo
    {
        ramses::RendererSceneState state = ramses::RendererSceneState::Unavailable;
        ramses::sceneVersionTag_t version = ramses::InvalidSceneVersionTag;
    };

    using SceneSet = std::unordered_map<ramses::sceneId_t, SceneInfo>;
    using DataConsumerSet = std::unordered_set<ramses::dataConsumerId_t>;
    using OffscreenBufferSet = std::unordered_set<ramses::displayBufferId_t>;

    template <typename T>
    void waitForElementInSet(const T element, const std::unordered_set<T>& set)
    {
        while (set.find(element) == set.end())
        {
            m_renderer.dispatchEvents(*this);
            m_renderer.getSceneControlAPI()->dispatchEvents(*this);
            std::this_thread::sleep_for(std::chrono::milliseconds(10u));
        }
    }

    SceneSet m_scenes;
    SceneSet m_scenesAssignedToOffscreenBuffer;
    SceneSet m_scenesConsumingOffscreenBuffer;
    OffscreenBufferSet m_createdOffscreenBuffers;
    DataConsumerSet m_dataConsumers;
    ramses::RamsesRenderer& m_renderer;
    bool m_windowClosed = false;
};
/** \endcond */

uint64_t nowMs()
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
}

static constexpr uint32_t SampleCount = 4u;
static constexpr uint32_t ObWidth = 200u;
static constexpr uint32_t ObHeight = 200u;
static constexpr uint32_t DisplayWidth = 800u;
static constexpr uint32_t DisplayHeight = 800u;

ramses::MeshNode* createTexturedQuad(ramses::Scene* clientScene, const ramses::Effect* effect, const ramses::ArrayResource* indices, const ramses::ArrayResource* vertexPositions, const ramses::ArrayResource* textureCoords)
{
    ramses::Appearance* appearance = clientScene->createAppearance(*effect, "quad appearance");
    ramses::Geometry* geometry = clientScene->createGeometry(*effect, "quad geometry");

    geometry->setIndices(*indices);
    std::optional<ramses::AttributeInput> positionsInput = effect->findAttributeInput("a_position");
    std::optional<ramses::AttributeInput> texCoordsInput = effect->findAttributeInput("a_texcoord");
    assert(positionsInput.has_value() && texCoordsInput.has_value());
    geometry->setInputBuffer(*positionsInput, *vertexPositions);
    geometry->setInputBuffer(*texCoordsInput, *textureCoords);

    ramses::MeshNode* meshNode = clientScene->createMeshNode();
    meshNode->setAppearance(*appearance);
    meshNode->setGeometry(*geometry);

    std::optional<ramses::UniformInput> sampleCountUnif;
    for (uint32_t i = 0; i < effect->getUniformInputCount(); ++i)
    {
        sampleCountUnif = effect->getUniformInput(i);
        if (sampleCountUnif.has_value() && strcmp(sampleCountUnif->getName(), "sampleCount") == 0)
        {
            appearance->setInputValue(*sampleCountUnif, static_cast<int32_t>(SampleCount));
            break;
        }
    }

    return meshNode;
}

void createClearPass(ramses::Scene* scene, ramses::RenderBuffer* renderBuffer)
{
    ramses::RenderPass* clearPass = scene->createRenderPass();
    clearPass->setRenderOrder(-100);
    //Create camera with dummy values as it is only clear pass
    ramses::PerspectiveCamera& dummyCamera = *scene->createPerspectiveCamera();
    dummyCamera.setFrustum(1.f, 1.f, 1.f, 100.f);
    dummyCamera.setViewport(0u, 0u, 16u, 16u);
    clearPass->setCamera(dummyCamera);

    ramses::RenderTargetDescription rtDesc;
    rtDesc.addRenderBuffer(*renderBuffer);
    ramses::RenderTarget& renderTarget = *scene->createRenderTarget(rtDesc);

    //Clear RenderBuffer to visible color
    clearPass->setRenderTarget(&renderTarget);
    clearPass->setClearColor({1.f, 0.f, 0.f, 1.f});
    clearPass->setClearFlags(ramses::EClearFlag::All);
}

ramses::Effect* createEffect(ramses::Scene* scene, const std::string& shaderName, const std::string& effectName)
{
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile(("res/" + shaderName + ".vert").c_str());
    effectDesc.setFragmentShaderFromFile(("res/" + shaderName + ".frag").c_str());
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    return scene->createEffect(effectDesc, effectName.c_str());
}

ramses::Scene* createConsumerScene(ramses::RamsesClient& client, ramses::sceneId_t sceneId, const char* samplerConsumerName, const char* samplerConsumerNameMS)
{
    ramses::Scene* clientScene = client.createScene(sceneId, "local displays example");

    ramses::OrthographicCamera* camera = clientScene->createOrthographicCamera("MyCamera");
    camera->setTranslation({0.0f, 0.0f, 5.0f});
    camera->setFrustum(-2.f, 2.f, -2.f, 2.f, 0.1f, 100.f);
    camera->setViewport(0, 0u, DisplayWidth, DisplayHeight);
    ramses::RenderPass* renderPass = clientScene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlag::None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = clientScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare quad geometry: vertex position array and index array
    const std::array<uint16_t, 6u> indicesArray{ 0, 1, 2, 2, 1, 3 };
    ramses::ArrayResource* sharedIndices = clientScene->createArrayResource(6u, indicesArray.data());

    const std::array<ramses::vec3f, 4u> vertexPositionsQuadArray{ ramses::vec3f{-1.f, -1.f, 0.f}, ramses::vec3f{1.f, -1.f, 0.f}, ramses::vec3f{-1.f, 1.f, 0.f}, ramses::vec3f{1.f, 1.f, 0.f} };
    ramses::ArrayResource* sharedVertexPositions = clientScene->createArrayResource(4u, vertexPositionsQuadArray.data());

    const std::array<ramses::vec2f, 4u> textureCoordsArray{ ramses::vec2f{0.f, 1.f}, ramses::vec2f{1.f, 1.f}, ramses::vec2f{0.f, 0.f}, ramses::vec2f{1.f, 0.f} };
    ramses::ArrayResource* sharedTextureCoords = clientScene->createArrayResource(4u, textureCoordsArray.data());

    // create an appearance for grey quad
    ramses::Effect* effect = createEffect(clientScene, "ramses-example-offscreenbuffer_texture","quad effect");
    ramses::MeshNode* meshNode = createTexturedQuad(clientScene, effect, sharedIndices, sharedVertexPositions, sharedTextureCoords);

    ramses::Node* transNode = clientScene->createNode();
    transNode->setTranslation({1.1f, 0.f, 0.f});
    meshNode->setParent(*transNode);
    renderGroup->addMeshNode(*meshNode);

    // Create fallback texture to show when sampler not linked to an offscreen buffer
    ramses::Texture2D* fallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng("res/ramses-example-offscreenbuffer-fallback.png", *clientScene);
    ramses::TextureSampler* textureSampler = clientScene->createTextureSampler(ramses::ETextureAddressMode::Repeat, ramses::ETextureAddressMode::Repeat, ramses::ETextureSamplingMethod::Linear, ramses::ETextureSamplingMethod::Linear, *fallbackTexture, 1u, samplerConsumerName);
    std::optional<ramses::UniformInput> textureUnif = effect->findUniformInput("textureSampler");
    assert(textureUnif.has_value());
    meshNode->getAppearance()->setInputTexture(*textureUnif, *textureSampler);

    // create an appearance for grey quad multi sampled
    ramses::Effect* effectMS = createEffect(clientScene, "ramses-example-offscreenbuffer_textureMS","quad effect MS");
    ramses::MeshNode* meshNodeMS = createTexturedQuad(clientScene, effectMS, sharedIndices, sharedVertexPositions, sharedTextureCoords);

    ramses::Node* transNodeMS = clientScene->createNode();
    transNodeMS->setTranslation({-1.1f, 0.f, 0.f});
    meshNodeMS->setParent(*transNodeMS);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNodeMS);

    // Multisampled texture cannot use a texture as fallback like the quad above but it has to use a multisampled render buffer as fallback instead in case the offscreen buffer is not linked.
    ramses::RenderBuffer* fallbackBuffer = clientScene->createRenderBuffer(ObWidth, ObHeight, ramses::ERenderBufferFormat::RGBA8, ramses::ERenderBufferAccessMode::ReadWrite, SampleCount);
    // Buffer gets cleared to solid color so it is visible when sampler is not linked to the offscreen buffer
    createClearPass(clientScene, fallbackBuffer);
    ramses::TextureSamplerMS* textureSamplerMS = clientScene->createTextureSamplerMS(*fallbackBuffer, samplerConsumerNameMS);
    std::optional<ramses::UniformInput> textureUnifMS = effectMS->findUniformInput("textureSampler");
    assert(textureUnifMS.has_value());
    meshNodeMS->getAppearance()->setInputTexture(*textureUnifMS, *textureSamplerMS);

    return clientScene;
}

ramses::Scene* createProviderScene(ramses::RamsesClient& client, ramses::sceneId_t sceneId, const char* rotationNodeName)
{
    ramses::Scene* clientScene = client.createScene(sceneId, "local displays example scene");

    ramses::OrthographicCamera* camera = clientScene->createOrthographicCamera ();
    camera->setTranslation({0.0f, 0.0f, 5.0f});
    camera->setFrustum ( -2.f, 2.f, -2.f, 2.f, 0.1f, 100.f );
    camera->setViewport ( 0, 0u, ObWidth, ObHeight );

    ramses::RenderPass* renderPass = clientScene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlag::None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = clientScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    ramses::Effect* effect = createEffect(clientScene, "ramses-example-offscreenbuffer","quad effect");

    const std::array<uint16_t, 6u> indicesArray{ 0, 1, 2, 2, 1, 3 };
    ramses::ArrayResource* indices = clientScene->createArrayResource(6u, indicesArray.data());

    const std::array<ramses::vec3f, 4u> vertexPositionsQuadArray{ ramses::vec3f{-1.f, -1.f, 0.f}, ramses::vec3f{1.f, -1.f, 0.f}, ramses::vec3f{-1.f, 1.f, 0.f}, ramses::vec3f{1.f, 1.f, 0.f} };
    ramses::ArrayResource* vertexPositions = clientScene->createArrayResource(4u, vertexPositionsQuadArray.data());

    ramses::Appearance* appearance = clientScene->createAppearance(*effect, "Quad appearance");
    ramses::Geometry* geometry = clientScene->createGeometry(*effect, "Quad geometry");

    geometry->setIndices(*indices);
    std::optional<ramses::AttributeInput> positionsInput = effect->findAttributeInput("a_position");
    assert(positionsInput.has_value());
    geometry->setInputBuffer(*positionsInput, *vertexPositions);

    std::optional<ramses::UniformInput> colorInput = effect->findUniformInput("color");
    assert(colorInput.has_value());
    appearance->setInputValue(*colorInput, ramses::vec4f{ 1.0f, 0.3f, 0.5f, 1.0f });

    ramses::MeshNode* meshNode = clientScene->createMeshNode("quad mesh node");
    ramses::Node* rotationNode = clientScene->createNode(rotationNodeName);
    meshNode->setParent(*rotationNode);
    meshNode->setAppearance(*appearance);
    meshNode->setGeometry(*geometry);
    renderGroup->addMeshNode(*meshNode);

    return clientScene;
}

int main()
{
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    config.setRequestedRamsesShellType(ramses::ERamsesShellType::Console);  //needed for automated test of examples
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& client(*framework.createClient("ramses-local-client-test"));

    ramses::RendererConfig rendererConfig;
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    auto& sceneControlAPI = *renderer.getSceneControlAPI();
    framework.connect();

    renderer.startThread();

    ramses::sceneId_t consumerSceneId{1u};
    ramses::Scene* consumerScene = createConsumerScene(client, consumerSceneId, "textureSamplerConsumer", "textureSamplerConsumerMS");
    consumerScene->flush();
    consumerScene->publish();

    ramses::sceneId_t providerSceneId{ 2u };
    ramses::Scene* providerScene = createProviderScene(client, providerSceneId, "rotationNode");
    providerScene->flush();
    providerScene->publish();

    ramses::sceneId_t providerSceneId2{ 3u };
    ramses::Scene* providerScene2 = createProviderScene(client, providerSceneId2, "rotationNodeMS");
    providerScene2->flush();
    providerScene2->publish();

    SceneStateEventHandler eventHandler(renderer);

    ramses::DisplayConfig displayConfig;
    displayConfig.setWindowRectangle(100, 100, DisplayWidth, DisplayHeight);
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);
    renderer.flush();

    // Both consumer and provider scene have to be mapped to same display in order to link them using offscreen buffer
    sceneControlAPI.setSceneMapping(consumerSceneId, display);
    sceneControlAPI.setSceneState(consumerSceneId, ramses::RendererSceneState::Ready);
    sceneControlAPI.flush();

    // Ramses renders all scenes mapped to offscreen buffers (except for interruptible offscreen buffers)
    // always before rendering scenes mapped to framebuffer.
    // If consumer scene is rendered into framebuffer, then there does not have to be any explicit render order set.
    // However should the consumer scene be rendered into offscreen buffer as well, it is recommended to set
    // explicit render order - provided content before consumer content.
    sceneControlAPI.setSceneMapping(providerSceneId, display);
    sceneControlAPI.setSceneState(providerSceneId, ramses::RendererSceneState::Ready);
    sceneControlAPI.setSceneMapping(providerSceneId2, display);
    sceneControlAPI.setSceneState(providerSceneId2, ramses::RendererSceneState::Ready);
    sceneControlAPI.flush();

    eventHandler.waitForSceneState(*consumerScene, ramses::RendererSceneState::Ready);
    eventHandler.waitForSceneState(*providerScene, ramses::RendererSceneState::Ready);
    eventHandler.waitForSceneState(*providerScene2, ramses::RendererSceneState::Ready);

    const auto* textureSamplerConsumerMS = consumerScene->findObject<ramses::TextureSamplerMS>("textureSamplerConsumerMS");
    const auto* textureSamplerConsumer   = consumerScene->findObject<ramses::TextureSampler>("textureSamplerConsumer");

    /// [Offscreen Buffer Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // Create texture consumer that will use multi sampled offscreen buffer to sample from
    const ramses::dataConsumerId_t samplerConsumerIdMS(456u);
    consumerScene->createTextureConsumer(*textureSamplerConsumerMS, samplerConsumerIdMS);
    // flush with version tag and wait for flush applied to make sure consumer is created
    ramses::sceneVersionTag_t versionTagMS{ 42 };
    consumerScene->flush(versionTagMS);
    eventHandler.waitForFlush(consumerSceneId, versionTagMS);

    // Create second texture consumer that will use non-MS offscreen buffer to sample from to visually compare MS vs non-MS
    const ramses::dataConsumerId_t samplerConsumerId(457u);
    consumerScene->createTextureConsumer(*textureSamplerConsumer, samplerConsumerId);
    // flush with version tag and wait for flush applied to make sure consumer is created
    ramses::sceneVersionTag_t versionTag{ 43 };
    consumerScene->flush(versionTag);
    eventHandler.waitForFlush(consumerSceneId, versionTag);

    // Create the two offscreen buffers - have to be on the same display where both the scene to be rendered into it is mapped and the scene to consume it is mapped
    const ramses::displayBufferId_t offscreenBufferMS = renderer.createOffscreenBuffer(display, ObWidth, ObHeight, SampleCount);
    const ramses::displayBufferId_t offscreenBuffer = renderer.createOffscreenBuffer(display, ObWidth, ObHeight);
    renderer.flush();
    eventHandler.waitForOffscreenBufferCreated(offscreenBufferMS);
    eventHandler.waitForOffscreenBufferCreated(offscreenBuffer);

    // Assign the provider scenes to the offscreen buffers
    // This means the providerscenes are not going to be rendered on the display but to the offscreen buffer they are assigned to instead
    sceneControlAPI.setSceneDisplayBufferAssignment(providerSceneId, offscreenBufferMS);
    sceneControlAPI.setSceneDisplayBufferAssignment(providerSceneId2, offscreenBuffer);
    sceneControlAPI.flush();

    // Link the offscreen buffers scene texture samplers so that the provided content will be used as texture
    sceneControlAPI.linkOffscreenBuffer(offscreenBufferMS, consumerSceneId, samplerConsumerIdMS);
    sceneControlAPI.linkOffscreenBuffer(offscreenBuffer, consumerSceneId, samplerConsumerId);
    sceneControlAPI.flush();
    eventHandler.waitForOffscreenBufferLinkedToConsumingScene(consumerSceneId);

    // Now both the consumer scene and the provider scenes can be rendered at the same time
    sceneControlAPI.setSceneState(consumerSceneId, ramses::RendererSceneState::Rendered);
    sceneControlAPI.setSceneState(providerSceneId, ramses::RendererSceneState::Rendered);
    sceneControlAPI.setSceneState(providerSceneId2, ramses::RendererSceneState::Rendered);
    sceneControlAPI.flush();

    // Unlink the offscreen buffer scenes every other second and show the fallback texture (sampler2D) and fallback buffer (sampler2DMS)
    bool link = false;
    // Start at 1 so texture samplers are not unlinked immediately
    uint64_t timeStamp = 1u;
    float rotationZ = 0.f;

    auto* rotationNode   = providerScene->findObject<ramses::Node>("rotationNode");
    auto* rotationNodeMS = providerScene2->findObject<ramses::Node>("rotationNodeMS");

    while (!eventHandler.isWindowClosed())
    {
        renderer.dispatchEvents(eventHandler);
        //Rotate the quads of the provider scenes
        rotationNode->setRotation({0.f, 0.f, rotationZ}, ramses::ERotationType::Euler_XYZ);
        providerScene->flush();
        rotationNodeMS->setRotation({0.f, 0.f, rotationZ}, ramses::ERotationType::Euler_XYZ);
        providerScene2->flush();

        if (timeStamp % 200 == 0)
        {
            if (link)
            {
                sceneControlAPI.linkOffscreenBuffer(offscreenBuffer, consumerSceneId, samplerConsumerId);
                sceneControlAPI.linkOffscreenBuffer(offscreenBufferMS, consumerSceneId, samplerConsumerIdMS);
            }
            else
            {
                sceneControlAPI.unlinkData(consumerSceneId, samplerConsumerId);
                sceneControlAPI.unlinkData(consumerSceneId, samplerConsumerIdMS);
            }
            sceneControlAPI.flush();

            link = !link;
        }

        ++rotationZ;
        ++timeStamp;
        std::this_thread::sleep_for(std::chrono::milliseconds{ 15u });
    }
    /// [Offscreen Buffer Example]

    renderer.stopThread();
}
