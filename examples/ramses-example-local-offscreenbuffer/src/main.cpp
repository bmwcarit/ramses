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
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "ramses-renderer-api/RendererSceneControl.h"
#include "ramses-utils.h"
#include <unordered_set>
#include <unordered_map>
#include <thread>

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

    virtual void sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state) override
    {
        m_scenes[sceneId].state = state;
    }

    virtual void sceneFlushed(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersion) override
    {
        m_scenes[sceneId].version = sceneVersion;
    }

    virtual void offscreenBufferCreated(ramses::displayId_t, ramses::displayBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override
    {
        if (ramses::ERendererEventResult_FAIL != result)
        {
            m_createdOffscreenBuffers.insert(offscreenBufferId);
        }
    }

    virtual void offscreenBufferLinked(ramses::displayBufferId_t, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t, bool success) override
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

    void waitForSceneState(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        waitUntilOrTimeout([&] { return m_scenes[sceneId].state == state; });
    }

    bool waitForFlush(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersion)
    {
        return waitUntilOrTimeout([&] { return m_scenes[sceneId].version == sceneVersion; });
    }

    bool waitUntilOrTimeout(const std::function<bool()>& conditionFunction)
    {
        const std::chrono::steady_clock::time_point timeoutTS = std::chrono::steady_clock::now() + std::chrono::seconds{ 5 };
        while (!conditionFunction() && std::chrono::steady_clock::now() < timeoutTS)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds{ 5 });  // will give the renderer time to process changes

            m_renderer.dispatchEvents(*this);
            m_renderer.getSceneControlAPI()->dispatchEvents(*this);
        }

        return conditionFunction();
    }

private:
    struct SceneInfo
    {
        ramses::RendererSceneState state = ramses::RendererSceneState::Unavailable;
        ramses::sceneVersionTag_t version = ramses::InvalidSceneVersionTag;
    };

    typedef std::unordered_map<ramses::sceneId_t, SceneInfo> SceneSet;
    typedef std::unordered_set<ramses::dataConsumerId_t> DataConsumerSet;
    typedef std::unordered_set<ramses::displayBufferId_t> OffscreenBufferSet;



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
};
/** \endcond */

uint64_t nowMs()
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
}

ramses::Scene* createScene1(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    //client scene
    ramses::Scene* clientScene = client.createScene(sceneId, ramses::SceneConfig(), "local displays example scene");

    // every scene needs a render pass with camera
    ramses::OrthographicCamera* camera = clientScene->createOrthographicCamera ( "MyCamera" );
    camera->setTranslation(0.0f, 0.0f, 5.0f);
    camera->setFrustum ( -2.f, 2.f, -2.f, 2.f, 0.1f, 100.f );
    // use right side of the viewport
    camera->setViewport(0, 0u, 800u, 800u);
    ramses::RenderPass* renderPass = clientScene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = clientScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    const float vertexPositionsArray[] = { -1.f, 0.f, -6.f, 1.f, 0.f, -6.f, 0.f, 1.f, -6.f };
    const ramses::Vector3fArray* vertexPositions = client.createConstVector3fArray(3, vertexPositionsArray);
    const uint16_t indicesArray[] = { 0, 1, 2 };
    const ramses::UInt16Array* indices = client.createConstUInt16Array(3, indicesArray);
    const float texCoordsArray[] = { 2.0f, 2.0f, 0.0f, 2.0f, 0.0f, 0.0f, 2.0f, 0.0f };
    const ramses::Vector2fArray* texCoords = client.createConstVector2fArray(4, texCoordsArray);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-offscreenbuffer_texture.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-offscreenbuffer_texture.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    const ramses::Effect* effect = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = clientScene->createAppearance(*effect, "triangle appearance");
    ramses::GeometryBinding* geometry = clientScene->createGeometryBinding(*effect, "triangle geometry");

    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);
    ramses::AttributeInput texCoordsInput;
    effect->findAttributeInput("a_texcoord", texCoordsInput);
    geometry->setInputBuffer(texCoordsInput, *texCoords);

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = clientScene->createMeshNode("triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);

    ramses::AnimationSystemRealTime* animationSystem = clientScene->createRealTimeAnimationSystem(ramses::EAnimationSystemFlags_Default, "animation system");

    // create splines with animation keys
    ramses::SplineLinearFloat* spline1 = animationSystem->createSplineLinearFloat("spline1");
    spline1->setKey(0u, 0.f);
    spline1->setKey(5000u, 360.f);

    // create animated property for each translation node with single component animation
    ramses::AnimatedProperty* animProperty1 = animationSystem->createAnimatedProperty(*meshNode, ramses::EAnimatedProperty_Rotation, ramses::EAnimatedPropertyComponent_Z);

    // create three animations
    ramses::Animation* animation1 = animationSystem->createAnimation(*animProperty1, *spline1, "animation1");

    // create animation sequence and add animation
    ramses::AnimationSequence* sequence = animationSystem->createAnimationSequence();
    sequence->addAnimation(*animation1);

    // set animation properties (optional)
    sequence->setAnimationLooping(*animation1);

    // start animation sequence
    animationSystem->updateLocalTime(nowMs());
    sequence->start();

    ramses::UniformInput textureUnif;
    effect->findUniformInput ( "textureSampler", textureUnif );

    ramses::Texture2D* fallbackTexture = ramses::RamsesUtils::CreateTextureResourceFromPng ( "res/ramses-example-offscreenbuffer.png", client );
    ramses::TextureSampler* textureSampler = clientScene->createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat, ramses::ETextureSamplingMethod_Linear, ramses::ETextureSamplingMethod_Linear, *fallbackTexture, 1u, "textureSamplerConsumer");
    appearance->setInputTexture(textureUnif, *textureSampler);

    return clientScene;
}

ramses::Scene* createScene2(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    //client scene
    ramses::Scene* clientScene = client.createScene(sceneId, ramses::SceneConfig(), "local displays example scene");

    // every scene needs a render pass with camera
    ramses::OrthographicCamera* camera = clientScene->createOrthographicCamera ("MyCamera");
    camera->setTranslation(0.0f, 0.0f, 5.0f);
    camera->setFrustum ( -2.f, 2.f, -2.f, 2.f, 0.1f, 100.f );
    // use right side of the viewport
    camera->setViewport ( 0, 0u, 200u, 200u );

    ramses::RenderPass* renderPass = clientScene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = clientScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    const float vertexPositionsArray[] = { -1.1f, 0.f, -6.1f, 1.1f, 0.f, -6.1f, 0.f, 1.1f, -6.1f };
    const ramses::Vector3fArray* vertexPositions = client.createConstVector3fArray(3, vertexPositionsArray);
    const uint16_t indicesArray[] = { 2, 0, 1 };
    const ramses::UInt16Array* indices = client.createConstUInt16Array(3, indicesArray);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-offscreenbuffer.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-offscreenbuffer.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    const ramses::Effect* effect = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = clientScene->createAppearance(*effect, "triangle appearance");
    ramses::GeometryBinding* geometry = clientScene->createGeometryBinding(*effect, "triangle geometry");

    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = clientScene->createMeshNode("triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);

    ramses::AnimationSystemRealTime* animationSystem = clientScene->createRealTimeAnimationSystem(ramses::EAnimationSystemFlags_Default, "animation system");

    // create splines with animation keys
    ramses::SplineLinearFloat* spline1 = animationSystem->createSplineLinearFloat("spline1");
    spline1->setKey(0u, 0.f);
    spline1->setKey(2000u, -360.f);

    // create animated property for each translation node with single component animation
    ramses::AnimatedProperty* animProperty1 = animationSystem->createAnimatedProperty(*meshNode, ramses::EAnimatedProperty_Rotation, ramses::EAnimatedPropertyComponent_Z);

    // create three animations
    ramses::Animation* animation1 = animationSystem->createAnimation(*animProperty1, *spline1, "animation1");

    // create animation sequence and add animation
    ramses::AnimationSequence* sequence = animationSystem->createAnimationSequence();
    sequence->addAnimation(*animation1);

    // set animation properties (optional)
    sequence->setAnimationLooping(*animation1);

    // start animation sequence
    animationSystem->updateLocalTime(nowMs());
    sequence->start();

    appearance->setInputValueVector4f(colorInput, 1.0f, 0.3f, 0.5f, 1.0f);

    return clientScene;
}

int main(int argc, char* argv[])
{
    // Ramses client
    ramses::RamsesFrameworkConfig config(argc, argv);
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);  //needed for automated test of examples
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& client(*framework.createClient("ramses-local-client-test"));

    // Ramses renderer
    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    auto& sceneControlAPI = *renderer.getSceneControlAPI();
    framework.connect();

    renderer.setMaximumFramerate(60.0f);
    renderer.startThread();

    ramses::sceneId_t sceneId1{1u};
    ramses::Scene* scene1 = createScene1(client, sceneId1);
    scene1->flush();
    scene1->publish();

    ramses::sceneId_t sceneId2{2u};
    ramses::Scene* scene2 = createScene2(client, sceneId2);
    scene2->flush();
    scene2->publish();

    SceneStateEventHandler eventHandler(renderer);

    ramses::DisplayConfig displayConfig(argc, argv);
    displayConfig.setWindowRectangle(100, 100, 800u, 800u);
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);
    renderer.flush();

    // Both scenes have to be mapped to same display in order to link them using offscreen buffer
    sceneControlAPI.setSceneMapping(sceneId1, display);
    sceneControlAPI.setSceneState(sceneId1, ramses::RendererSceneState::Ready);
    sceneControlAPI.flush();

    // Ramses renders all scenes mapped to offscreen buffers (except for interruptible offscreen buffers)
    // always before rendering scenes mapped to framebuffer.
    // If consumer scene is rendered into framebuffer, then there does not have to be any explicit render order set.
    // However should the consumer scene be rendered into offscreen buffer as well, it is recommended to set
    // explicit render order - provided content before consumer content.
    sceneControlAPI.setSceneMapping(sceneId2, display);
    sceneControlAPI.setSceneState(sceneId2, ramses::RendererSceneState::Ready);
    sceneControlAPI.flush();

    eventHandler.waitForSceneState(sceneId1, ramses::RendererSceneState::Ready);
    eventHandler.waitForSceneState(sceneId2, ramses::RendererSceneState::Ready);

    const ramses::TextureSampler* textureSamplerConsumer = ramses::RamsesUtils::TryConvert<ramses::TextureSampler>(*scene1->findObjectByName("textureSamplerConsumer"));

    /// [Offscreen Buffer Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // Create texture consumer that will use offscreen buffer to sample from
    const ramses::dataConsumerId_t samplerConsumerId(456u);
    scene1->createTextureConsumer(*textureSamplerConsumer, samplerConsumerId);
    // flush with version tag and wait for flush applied to make sure consumer is created
    ramses::sceneVersionTag_t versionTag{ 42 };
    scene1->flush(versionTag);
    eventHandler.waitForFlush(sceneId1, versionTag);

    // Create an offscreen buffer - has to be on the same display where both the scene to be rendered into it is mapped and the scene to consume it is mapped
    const ramses::displayBufferId_t offscreenBuffer = renderer.createOffscreenBuffer(display, 200u, 200u);
    renderer.flush();
    eventHandler.waitForOffscreenBufferCreated(offscreenBuffer);

    // Assign the second scene to the offscreen buffer
    // This means scene is not going to be rendered on the display but to the offscreen buffer instead
    sceneControlAPI.setSceneDisplayBufferAssignment(sceneId2, offscreenBuffer);
    sceneControlAPI.flush();


    // Link the offscreen buffer to first scene texture sampler so that the content will be used as texture
    sceneControlAPI.linkOffscreenBuffer(offscreenBuffer, sceneId1, samplerConsumerId);
    sceneControlAPI.flush();
    eventHandler.waitForOffscreenBufferLinkedToConsumingScene(sceneId1);

    // Now both scenes can be rendered at the same time
    sceneControlAPI.setSceneState(sceneId1, ramses::RendererSceneState::Rendered);
    sceneControlAPI.setSceneState(sceneId2, ramses::RendererSceneState::Rendered);
    sceneControlAPI.flush();
    /// [Offscreen Buffer Example]
    std::this_thread::sleep_for(std::chrono::seconds(100u));
    renderer.stopThread();
}
