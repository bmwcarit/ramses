//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

static const char* const providerFragmentShaderCode = R"GLSL(
#version 100
uniform highp vec4 color;
void main(void)
{
    gl_FragColor = color + vec4(0.1);
}
)GLSL";

static const char* const providerVertexShaderCode = R"GLSL(
#version 100
uniform highp mat4 mvpMatrix;
attribute vec3 a_position;
void main()
{
    gl_Position = mvpMatrix * vec4(a_position, 1.0);
}
)GLSL";

static const char* const consumerVertShader = R"GLSL(
#version 100
uniform highp mat4 mvpMatrix;
attribute vec3 a_position;
attribute vec2 a_texcoord;
varying vec2 v_texcoord;
void main()
{
    gl_Position = mvpMatrix * vec4(a_position, 1.0);
    v_texcoord = a_texcoord;
}
)GLSL";

static const char* const consumerFragshader = R"GLSL(
#version 100
precision mediump float;
uniform sampler2D textureSampler;
varying lowp vec2 v_texcoord;
void main(void)
{
    gl_FragColor = vec4(0.1) + texture2D(textureSampler, v_texcoord);
}
)GLSL";

#include "ramses-client.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/RendererSceneControl.h"
#include "ramses-framework-api/DcsmApiTypes.h"
#include <unordered_set>
#include <thread>
#include "ramses-framework-api/DcsmProvider.h"
#include "ramses-renderer-api/DcsmContentControl.h"
#include "ramses-renderer-api/IDcsmContentControlEventHandler.h"
#include "ramses-framework-api/IDcsmProviderEventHandler.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include <array>

/**
 * @example ramses-example-local-client-dcsm/src/main.cpp
 * @brief Local Client with DCSM Example
 */

using namespace ramses;

constexpr ramses::sceneId_t providerSceneId(1u);
constexpr ramses::sceneId_t consumerSceneId(2u);
constexpr ramses::ContentID providerContentID{ 1 };
constexpr ramses::ContentID consumerContentID{ 2 };
constexpr ramses::Category localCategory{ 1 };

class LocalDCSMExample : public ramses::DcsmContentControlEventHandlerEmpty, public ramses::IDcsmProviderEventHandler, public ramses::RendererEventHandlerEmpty
{
public:
    LocalDCSMExample(int32_t argc, char const* const* argv)
    {
        // framework
        ramses::RamsesFrameworkConfig config(argc, argv);
        config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);  //needed for automated test of examples
        m_framework = new RamsesFramework(config);

        // provider side
        client = m_framework->createClient("ramses-local-client-test");
        dcsmProvider = m_framework->createDcsmProvider();

        // consumer / renderer side
        ramses::RendererConfig rendererConfig(argc, argv);
        renderer = m_framework->createRenderer(rendererConfig);
        dcsmControl = renderer->createDcsmContentControl();
        renderer->startThread();
        ramses::DisplayConfig displayConfig(argc, argv);
        display = renderer->createDisplay(displayConfig);
        ob = renderer->createOffscreenBuffer(display, 640, 480);
        dcsmControl->addContentCategory(localCategory, display, {{ 1280, 480 }, { 0, 0, 1280, 480 }, {0,0,0,0}} );
        dcsmControl->update(0, *this);
        renderer->flush();
    }

    ~LocalDCSMExample() override
    {
        renderer->stopThread();
        m_framework->destroyRenderer(*renderer);
        m_framework->destroyClient(*client);
        delete m_framework;
    }

    // Provider side callbacks
    void contentHide(ContentID /*contentID*/, AnimationInformation /*animInfo*/) override
    {
    }

    void contentShow(ContentID /*contentID*/, AnimationInformation /*animInfo*/) override
    {
    }

    void stopOfferAccepted(ContentID /*contentID*/, AnimationInformation /*animInfo*/) override
    {
    }

    void contentSizeChange(ContentID /*contentID*/, const CategoryInfoUpdate& /*categoryInfo*/, AnimationInformation /*animInfo*/) override
    {
    }

    void contentReadyRequested(ContentID /*contentID*/) override
    {
    }

    void contentRelease(ContentID /*contentID*/, AnimationInformation /*animInfo*/) override
    {
    }

    // consumer side callbacks
    void contentAvailable(ContentID contentID, Category ) override
    {
        // ramp up content
        dcsmControl->requestContentReady(contentID, 0);
    }

    void contentReady(ContentID contentID, DcsmContentControlEventResult ) override
    {
        if (contentID == providerContentID)
        {
            dcsmControl->assignContentToDisplayBuffer(contentID, ob);
            m_providerSceneReady = true;
            linkOffscreenBufferIfReady();
        }
        if (contentID == consumerContentID)
        {
            m_consumerSceneReady = true;
            linkOffscreenBufferIfReady();
        }
        dcsmControl->showContent(contentID, {});
    }

    void offscreenBufferCreated(displayId_t /*displayId*/, displayBufferId_t /*offscreenBufferId*/, ERendererEventResult result) override
    {
        if (result == ERendererEventResult_OK)
        {
            m_offscreenBufferCreated = true;
            linkOffscreenBufferIfReady();
        }
    }

    void windowClosed(displayId_t /*displayId*/) override
    {
        m_windowClosed = true;
    }

    void run()
    {
        createProviderScene();
        createLocalConsumerScene();
        offerScenesViaDcsm();
        while (!m_windowClosed)
        {
            dcsmControl->update(0, *this);
            renderer->dispatchEvents(*this);
            dcsmProvider->dispatchEvents(*this);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

private:

    void createProviderScene()
    {
        ramses::Scene* scene = client->createScene(providerSceneId, ramses::SceneConfig(), "local client example scene");
        auto* camera = scene->createPerspectiveCamera("my camera");
        camera->setViewport(0, 0, 640u, 480u);
        camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
        ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
        renderPass->setClearFlags(ramses::EClearFlags_None);
        renderPass->setCamera(*camera);
        ramses::RenderGroup* renderGroup = scene->createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);

        // prepare triangle geometry: vertex position array and index array
        float vertexPositionsArray[] = { -1.f, 0.f, -6.f, 1.f, 0.f, -6.f, 0.f, 1.f, -6.f };
        ramses::ArrayResource* vertexPositions = scene->createArrayResource(ramses::EDataType::Vector3F, 3, vertexPositionsArray);
        uint16_t indicesArray[] = { 0, 1, 2 };
        ramses::ArrayResource* indices = scene->createArrayResource(ramses::EDataType::UInt16, 3, indicesArray);

        // create an appearance for red triangle
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShader(providerVertexShaderCode);
        effectDesc.setFragmentShader(providerFragmentShaderCode);
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

        ramses::Effect* effect = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
        ramses::Appearance* appearance = scene->createAppearance(*effect, "triangle appearance");
        ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect, "triangle geometry");

        geometry->setIndices(*indices);
        ramses::AttributeInput positionsInput;
        effect->findAttributeInput("a_position", positionsInput);
        geometry->setInputBuffer(positionsInput, *vertexPositions);

        ramses::UniformInput colorInput;
        effect->findUniformInput("color", colorInput);

        // create a mesh node to define the triangle with chosen appearance
        ramses::MeshNode* meshNode = scene->createMeshNode("triangle mesh node");
        meshNode->setTranslation(0.0f, 0.0f, -5.0f);
        meshNode->setAppearance(*appearance);
        meshNode->setGeometryBinding(*geometry);
        // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
        renderGroup->addMeshNode(*meshNode);

        ramses::AnimationSystemRealTime* animationSystem = scene->createRealTimeAnimationSystem(ramses::EAnimationSystemFlags_Default, "animation system");

        // create splines with animation keys
        ramses::SplineLinearFloat* spline1 = animationSystem->createSplineLinearFloat("spline1");
        spline1->setKey(0u, 0.f);
        spline1->setKey(4000u, 360.f);

        // create animated property for each translation node with single component animation
        ramses::AnimatedProperty* animProperty1 = animationSystem->createAnimatedProperty(*meshNode, ramses::EAnimatedProperty_Rotation, ramses::EAnimatedPropertyComponent_Z);

        // create three animations
        ramses::Animation* animation1 = animationSystem->createAnimation(*animProperty1, *spline1, "animation1");

        // create animation sequence and add animation
        ramses::AnimationSequence* sequence = animationSystem->createAnimationSequence();
        sequence->addAnimation(*animation1);

        // set animation properties (optional)
        sequence->setAnimationLooping(*animation1);
        sequence->setPlaybackSpeed(0.5f);

        // start animation sequence
        animationSystem->updateLocalTime();
        sequence->start();

        appearance->setInputValueVector4f(colorInput, 1.0f, 0.0f, 0.3f, 1.0f);

        scene->publish(ramses::EScenePublicationMode_LocalOnly);
        scene->flush();
    }

    void createLocalConsumerScene()
    {
        ramses::Scene* scene = client->createScene(consumerSceneId, ramses::SceneConfig(), "local client example scene");

        ramses::OrthographicCamera* camera = scene->createOrthographicCamera("MyCamera");
        camera->setTranslation(0.0f, 0.0f, 5.0f);
        camera->setFrustum(-2.f, 2.f, -2.f, 2.f, 0.1f, 100.f);
        camera->setViewport(0, 0u, 1280, 480);
        ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
        renderPass->setClearFlags(ramses::EClearFlags_None);
        renderPass->setCamera(*camera);
        ramses::RenderGroup* renderGroup = scene->createRenderGroup();
        renderPass->addRenderGroup(*renderGroup);

        // prepare quad geometry: vertex position array and index array
        const uint16_t indicesArray[] = { 0, 1, 2, 2, 1, 3 };
        const ramses::ArrayResource* sharedIndices = scene->createArrayResource(ramses::EDataType::UInt16, 6, indicesArray);

        const float vertexPositionsArray[] =
        {
            -1.f, -1.f, 0.f,
            1.f, -1.f, 0.f,
            -1.f, 1.f, 0.f,
            1.f, 1.f, 0.f
        };
        const ramses::ArrayResource* sharedVertexPositions = scene->createArrayResource(ramses::EDataType::Vector3F, 4, vertexPositionsArray);

        const float textureCoordsArray[] = { 0.f, 1.f, 1.f, 1.f, 0.f, 0.f, 1.f, 0.f };
        const ramses::ArrayResource* sharedTextureCoords = scene->createArrayResource(ramses::EDataType::Vector2F, 4, textureCoordsArray);
        ramses::EffectDescription effectDesc;
        effectDesc.setVertexShader(consumerVertShader);
        effectDesc.setFragmentShader(consumerFragshader);
        effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

        auto effect = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "");
        ramses::Appearance* appearance = scene->createAppearance(*effect, "quad appearance");
        ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect, "quad geometry");

        geometry->setIndices(*sharedIndices);
        ramses::AttributeInput positionsInput;
        effect->findAttributeInput("a_position", positionsInput);
        geometry->setInputBuffer(positionsInput, *sharedVertexPositions);
        ramses::AttributeInput texCoordsInput;
        effect->findAttributeInput("a_texcoord", texCoordsInput);
        geometry->setInputBuffer(texCoordsInput, *sharedTextureCoords);

        ramses::MeshNode* meshNode = scene->createMeshNode();
        meshNode->setAppearance(*appearance);
        meshNode->setGeometryBinding(*geometry);
        const std::array<uint8_t, 4> pxData{ {0xff, 0x0, 0x0, 0xff} };
        const ramses::MipLevelData mipLevelData(4, pxData.data());
        const ramses::Texture2D* texture = scene->createTexture2D(ramses::ETextureFormat::RGBA8, 1u, 1u, 1, &mipLevelData, false, {}, ramses::ResourceCacheFlag_DoNotCache);
        ramses::TextureSampler* textureSampler = scene->createTextureSampler(ramses::ETextureAddressMode_Repeat, ramses::ETextureAddressMode_Repeat, ramses::ETextureSamplingMethod_Linear, ramses::ETextureSamplingMethod_Linear, *texture, 1u, "");
        ramses::UniformInput textureUnif;
        effect->findUniformInput("textureSampler", textureUnif);
        meshNode->getAppearance()->setInputTexture(textureUnif, *textureSampler);
        renderGroup->addMeshNode(*meshNode);
        scene->createTextureConsumer(*textureSampler, ramses::dataConsumerId_t{1});
        scene->publish(EScenePublicationMode_LocalOnly);
        scene->flush();
    }

    void offerScenesViaDcsm()
    {
        dcsmProvider->offerContent(providerContentID, localCategory, providerSceneId, ramses::EDcsmOfferingMode::LocalOnly);
        dcsmProvider->markContentReady(providerContentID);
        dcsmProvider->offerContent(consumerContentID, localCategory, consumerSceneId, ramses::EDcsmOfferingMode::LocalOnly);
        dcsmProvider->markContentReady(consumerContentID);
        dcsmControl->update(0, *this);
        dcsmProvider->dispatchEvents(*this);
    }

    void linkOffscreenBufferIfReady()
    {
        if (m_providerSceneReady && m_consumerSceneReady && m_offscreenBufferCreated)
        {
            dcsmControl->linkOffscreenBuffer(ob, consumerContentID, ramses::dataConsumerId_t{ 1 });
        }
    }

    ramses::RamsesFramework* m_framework;
    ramses::RamsesClient* client;
    ramses::DcsmProvider* dcsmProvider;
    ramses::RamsesRenderer* renderer;
    ramses::DcsmContentControl* dcsmControl;
    ramses::displayId_t display;
    ramses::displayBufferId_t ob;
    bool m_providerSceneReady = false;
    bool m_consumerSceneReady = false;
    bool m_offscreenBufferCreated = false;
    bool m_windowClosed = false;
};

int main(int argc, char* argv[])
{
    LocalDCSMExample example (argc, argv);
    example.run();
}
