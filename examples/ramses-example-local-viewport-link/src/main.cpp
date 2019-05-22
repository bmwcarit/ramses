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
#include "ramses-client-api/PerspectiveCamera.h"
#include <unordered_set>
#include <thread>
#include "ramses-utils.h"

/**
 * @example ramses-example-local-viewport-link/src/main.cpp
 * @brief Example of linking viewport parameters to control scene position on screen
 */

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

uint64_t nowMs()
{
    const auto now = std::chrono::system_clock::now();
    return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
}

static constexpr unsigned VPWidth = 600;
static constexpr unsigned VPHeight = 400;
static constexpr unsigned DispWidth  = 1280;
static constexpr unsigned DispHeight = 480;

// Master scene's data provider IDs
static constexpr ramses::dataProviderId_t VP1OffsetProviderId{ 333 };
static constexpr ramses::dataProviderId_t VP1SizeProviderId{ 334 };
static constexpr ramses::dataProviderId_t VP2OffsetProviderId{ 335 };
static constexpr ramses::dataProviderId_t VP2SizeProviderId{ 336 };
static constexpr ramses::dataProviderId_t Color1ProviderId{ 337 };
static constexpr ramses::dataProviderId_t Color2ProviderId{ 338 };
static constexpr ramses::dataProviderId_t Color3ProviderId{ 339 };
static constexpr ramses::dataProviderId_t Color4ProviderId{ 340 };

// Content scene's data consumer IDs
static constexpr ramses::dataConsumerId_t VPOffsetConsumerId{ 350 };
static constexpr ramses::dataConsumerId_t VPSizeConsumerId{ 351 };
static constexpr ramses::dataConsumerId_t Color1ConsumerId{ 352 };
static constexpr ramses::dataConsumerId_t Color2ConsumerId{ 353 };

ramses::Scene* createContentProviderScene(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    ramses::Scene* clientScene = client.createScene(sceneId);

    ramses::PerspectiveCamera* camera = clientScene->createPerspectiveCamera("camera");
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    camera->setTranslation(0.0f, 0.0f, 5.0f);

    /// [Viewport Link Example Content]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.
    // Bind data objects to scene's camera viewport offset/size and mark as data consumers
    const auto vpOffsetData = clientScene->createDataVector2i("vpOffset");
    const auto vpSizeData = clientScene->createDataVector2i("vpSize");
    vpOffsetData->setValue(0, 0);
    vpSizeData->setValue(VPWidth, VPHeight);
    camera->bindViewportOffset(*vpOffsetData);
    camera->bindViewportSize(*vpSizeData);
    clientScene->createDataConsumer(*vpOffsetData, VPOffsetConsumerId);
    clientScene->createDataConsumer(*vpSizeData, VPSizeConsumerId);
    /// [Viewport Link Example Content]

    ramses::RenderPass* renderPass = clientScene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = clientScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    float vertexPositionsArray[] = { -1.f, 0.f, -6.f, 1.f, 0.f, -6.f, 0.f, 1.f, -6.f };
    const ramses::Vector3fArray* vertexPositions = client.createConstVector3fArray(3, vertexPositionsArray);
    uint16_t indicesArray[] = { 0, 1, 2 };
    const ramses::UInt16Array* indices = client.createConstUInt16Array(3, indicesArray);

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-local-viewport-link.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-local-viewport-link.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    const ramses::Effect* effect = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = clientScene->createAppearance(*effect, "triangle appearance");
    ramses::GeometryBinding* geometry = clientScene->createGeometryBinding(*effect, "triangle geometry");

    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    ramses::MeshNode* meshNode = clientScene->createMeshNode("triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*geometry);
    renderGroup->addMeshNode(*meshNode);

    ramses::MeshNode*   meshNode2   = clientScene->createMeshNode("triangle mesh node");
    ramses::Appearance* appearance2 = clientScene->createAppearance(*effect, "triangle appearance");
    meshNode2->setAppearance(*appearance2);
    meshNode2->setGeometryBinding(*geometry);
    renderGroup->addMeshNode(*meshNode2);
    meshNode2->setTranslation(0, -10, -5);
    meshNode2->setScaling(100, 100, 1);

    ramses::AnimationSystemRealTime* animationSystem = clientScene->createRealTimeAnimationSystem(ramses::EAnimationSystemFlags_Default, "animation system");
    ramses::SplineLinearFloat* spline1 = animationSystem->createSplineLinearFloat("spline1");
    spline1->setKey(0u, 0.f);
    spline1->setKey(5000u, 500.f);
    spline1->setKey(10000u, 1000.f);
    ramses::AnimatedProperty* animProperty1 = animationSystem->createAnimatedProperty(*meshNode, ramses::EAnimatedProperty_Rotation, ramses::EAnimatedPropertyComponent_Z);
    ramses::Animation* animation1 = animationSystem->createAnimation(*animProperty1, *spline1, "animation1");
    ramses::AnimationSequence* sequence = animationSystem->createAnimationSequence();
    sequence->addAnimation(*animation1);
    sequence->setAnimationLooping(*animation1);
    sequence->setPlaybackSpeed(1.f);
    animationSystem->updateLocalTime(nowMs());
    sequence->start();

    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);

    // Create data objects to hold color and bind them to appearance inputs
    auto color1 = clientScene->createDataVector4f();
    auto color2 = clientScene->createDataVector4f();
    color1->setValue(1.f, 1.f, 1.f, 1.f);
    color1->setValue(0.5f, 0.5f, 0.5f, 1.f);
    appearance->bindInput(colorInput, *color1);
    appearance2->bindInput(colorInput, *color2);
    clientScene->createDataConsumer(*color1, Color1ConsumerId);
    clientScene->createDataConsumer(*color2, Color2ConsumerId);

    return clientScene;
}

ramses::Scene* createSceneMaster(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    ramses::Scene* clientScene = client.createScene(sceneId);

    /// [Viewport Link Example Master]
    // In master scene create data objects and mark them as providers to control other scenes' viewports
    const auto vp1offset = clientScene->createDataVector2i("vp1offset");
    const auto vp1size = clientScene->createDataVector2i("vp1size");
    const auto vp2offset = clientScene->createDataVector2i("vp2offset");
    const auto vp2size = clientScene->createDataVector2i("vp2size");

    vp1offset->setValue(0, 0);
    vp1size->setValue(DispWidth, DispHeight);
    vp2offset->setValue(0, 0);
    vp2size->setValue(DispWidth, DispHeight);

    clientScene->createDataProvider(*vp1offset, VP1OffsetProviderId);
    clientScene->createDataProvider(*vp1size, VP1SizeProviderId);
    clientScene->createDataProvider(*vp2offset, VP2OffsetProviderId);
    clientScene->createDataProvider(*vp2size, VP2SizeProviderId);
    /// [Viewport Link Example Master]

    // Create data objects and mark them as providers to control content scenes colors
    const auto color1 = clientScene->createDataVector4f("color1");
    const auto color2 = clientScene->createDataVector4f("color2");
    const auto color3 = clientScene->createDataVector4f("color3");
    const auto color4 = clientScene->createDataVector4f("color4");

    color1->setValue(1.f, 1.f, 0.3f, 1.f);
    color2->setValue(0.f, 0.3f, 0.5f, 1.f);
    color3->setValue(1.f, 0.f, 0.5f, 1.f);
    color4->setValue(0.5f, 0.3f, 0.f, 1.f);

    clientScene->createDataProvider(*color1, Color1ProviderId);
    clientScene->createDataProvider(*color2, Color2ProviderId);
    clientScene->createDataProvider(*color3, Color3ProviderId);
    clientScene->createDataProvider(*color4, Color4ProviderId);

    return clientScene;
}

int main(int argc, char* argv[])
{
    // Ramses client
    ramses::RamsesFrameworkConfig config(argc, argv);
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);  //needed for automated test of examples
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient client("ramses-local-client-test", framework);

    // Ramses renderer
    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer renderer(framework, rendererConfig);
    framework.connect();

    const ramses::sceneId_t sceneId1 = 1u;
    const ramses::sceneId_t sceneId2 = 2u;
    const ramses::sceneId_t sceneIdMaster = 3u;

    ramses::Scene* scene1 = createContentProviderScene(client, sceneId1);
    ramses::Scene* scene2 = createContentProviderScene(client, sceneId2);
    ramses::Scene* sceneMaster = createSceneMaster(client, sceneIdMaster);

    scene1->flush();
    scene2->flush();
    sceneMaster->flush();

    scene1->publish();
    scene2->publish();
    sceneMaster->publish();

    SceneStateEventHandler eventHandler(renderer);
    eventHandler.waitForPublication(sceneId1);
    eventHandler.waitForPublication(sceneId2);
    eventHandler.waitForPublication(sceneIdMaster);

    renderer.subscribeScene(sceneId1);
    renderer.subscribeScene(sceneId2);
    renderer.subscribeScene(sceneIdMaster);
    renderer.flush();

    eventHandler.waitForSubscription(sceneId1);
    eventHandler.waitForSubscription(sceneId2);
    eventHandler.waitForSubscription(sceneIdMaster);

    ramses::DisplayConfig displayConfig1(argc, argv);
    displayConfig1.setIntegrityRGLDeviceUnit(0);
    displayConfig1.setWaylandIviSurfaceID(0);
    displayConfig1.setWaylandIviLayerID(3);
    displayConfig1.setWindowIviVisible();
    displayConfig1.setWindowRectangle(50, 50, DispWidth, DispHeight);
    const ramses::displayId_t display1 = renderer.createDisplay(displayConfig1);

    renderer.mapScene(display1, sceneId1);
    renderer.flush();
    eventHandler.waitForMapped(sceneId1);
    renderer.showScene(sceneId1);

    renderer.mapScene(display1, sceneId2);
    renderer.flush();
    eventHandler.waitForMapped(sceneId2);
    renderer.showScene(sceneId2);

    renderer.flush();

#if 1
    /// [Viewport Link Example]
    // Link master scene's data objects to other scenes cameras' viewports
    renderer.linkData(sceneIdMaster, VP1OffsetProviderId, sceneId1, VPOffsetConsumerId);
    renderer.linkData(sceneIdMaster, VP1SizeProviderId, sceneId1, VPSizeConsumerId);
    renderer.linkData(sceneIdMaster, VP2OffsetProviderId, sceneId2, VPOffsetConsumerId);
    renderer.linkData(sceneIdMaster, VP2SizeProviderId, sceneId2, VPSizeConsumerId);
    renderer.flush();
    /// [Viewport Link Example]

    // Link master scene's data objects to scene1 and scene2 colors
    renderer.linkData(sceneIdMaster, Color1ProviderId, sceneId1, Color1ConsumerId);
    renderer.linkData(sceneIdMaster, Color2ProviderId, sceneId1, Color2ConsumerId);
    renderer.linkData(sceneIdMaster, Color3ProviderId, sceneId2, Color1ConsumerId);
    renderer.linkData(sceneIdMaster, Color4ProviderId, sceneId2, Color2ConsumerId);
    renderer.flush();
#endif

    // These are master scene's data objects linked to scene1 and scene2 cameras' viewports
    auto scene1vpOffset = ramses::RamsesUtils::TryConvert<ramses::DataVector2i>(*sceneMaster->findObjectByName("vp1offset"));
    auto scene1vpSize = ramses::RamsesUtils::TryConvert<ramses::DataVector2i>(*sceneMaster->findObjectByName("vp1size"));
    auto scene2vpOffset = ramses::RamsesUtils::TryConvert<ramses::DataVector2i>(*sceneMaster->findObjectByName("vp2offset"));
    auto scene2vpSize = ramses::RamsesUtils::TryConvert<ramses::DataVector2i>(*sceneMaster->findObjectByName("vp2size"));

    int animParam = 0;
    bool animInc = true;

    for (;;)
    {
        scene1vpOffset->setValue(VPWidth/8 + VPWidth/2 * animParam/100, VPHeight/8 + VPHeight/4 * animParam/100);
        scene1vpSize->setValue(VPWidth/4 + VPWidth/2 * animParam/100, VPHeight/4 + VPHeight/4 * animParam/100);
        const auto invAnimParam = 100 - animParam;
        scene2vpOffset->setValue(DispWidth/2 - VPWidth/2 * invAnimParam/100, DispHeight - DispHeight/4 - VPHeight/4 * invAnimParam/100);
        scene2vpSize->setValue(VPWidth/2 + VPWidth/2 * invAnimParam/100, VPHeight/4 + VPHeight/4 * invAnimParam/100);
        sceneMaster->flush();

        renderer.doOneLoop();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        animParam = (animInc ? animParam + 1 : animParam - 1);
        if (animParam == 0 || animParam == 100)
            animInc = !animInc;
    }
}
