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
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "ramses-renderer-api/RendererSceneControl.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include <unordered_set>
#include <thread>
#include <unordered_map>
#include "ramses-utils.h"

/**
 * @example ramses-example-local-viewport-link/src/main.cpp
 * @brief Example of linking viewport parameters to control scene position on screen
 */

uint64_t nowMs()
{
    auto now = std::chrono::system_clock::now();
    return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
}

/** \cond HIDDEN_SYMBOLS */
class SceneStateEventHandler : public ramses::RendererSceneControlEventHandlerEmpty
{
public:
    explicit SceneStateEventHandler(ramses::RamsesRenderer& renderer)
        : m_renderer(renderer)
    {
    }

    virtual void sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state) override
    {
        m_scenes[sceneId] = state;
    }

    void waitForSceneState(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        while (m_scenes.count(sceneId) == 0 || m_scenes.find(sceneId)->second != state)
        {
            m_renderer.doOneLoop();
            m_renderer.getSceneControlAPI()->dispatchEvents(*this);
        }
    }

private:
    typedef std::unordered_map<ramses::sceneId_t, ramses::RendererSceneState> SceneSet;

    ramses::RamsesRenderer& m_renderer;
    SceneSet m_scenes;
};
/** \endcond */
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
    ramses::ArrayResource* vertexPositions = clientScene->createArrayResource(ramses::EDataType::Vector3F, 3, vertexPositionsArray);
    uint16_t indicesArray[] = { 0, 1, 2 };
    ramses::ArrayResource* indices = clientScene->createArrayResource(ramses::EDataType::UInt16, 3, indicesArray);

    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-local-viewport-link.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-local-viewport-link.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effect = clientScene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
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
    ramses::RamsesClient& client(*framework.createClient("ramses-local-client-test"));

    // Ramses renderer
    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    auto& sceneControlAPI = *renderer.getSceneControlAPI();
    framework.connect();

    const ramses::sceneId_t sceneId1(1u);
    const ramses::sceneId_t sceneId2(2u);
    const ramses::sceneId_t sceneIdMaster(3u);

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

    ramses::DisplayConfig displayConfig(argc, argv);
    displayConfig.setWindowRectangle(50, 50, DispWidth, DispHeight);
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);
    renderer.flush();

    sceneControlAPI.setSceneMapping(sceneId1, display);
    sceneControlAPI.setSceneMapping(sceneId2, display);
    sceneControlAPI.setSceneState(sceneId1, ramses::RendererSceneState::Rendered);
    sceneControlAPI.setSceneState(sceneId2, ramses::RendererSceneState::Rendered);
    // Master scene does not need to be rendered, it only provides data to the other 2 scenes,
    // it must be at least in Available state meaning its data is on renderer
    sceneControlAPI.setSceneState(sceneIdMaster, ramses::RendererSceneState::Available);
    sceneControlAPI.flush();

    eventHandler.waitForSceneState(sceneId1, ramses::RendererSceneState::Rendered);
    eventHandler.waitForSceneState(sceneId2, ramses::RendererSceneState::Rendered);
    eventHandler.waitForSceneState(sceneIdMaster, ramses::RendererSceneState::Available);

#if 1
    /// [Viewport Link Example]
    // Link master scene's data objects to other scenes cameras' viewports
    sceneControlAPI.linkData(sceneIdMaster, VP1OffsetProviderId, sceneId1, VPOffsetConsumerId);
    sceneControlAPI.linkData(sceneIdMaster, VP1SizeProviderId, sceneId1, VPSizeConsumerId);
    sceneControlAPI.linkData(sceneIdMaster, VP2OffsetProviderId, sceneId2, VPOffsetConsumerId);
    sceneControlAPI.linkData(sceneIdMaster, VP2SizeProviderId, sceneId2, VPSizeConsumerId);
    sceneControlAPI.flush();
    /// [Viewport Link Example]

    // Link master scene's data objects to scene1 and scene2 colors
    sceneControlAPI.linkData(sceneIdMaster, Color1ProviderId, sceneId1, Color1ConsumerId);
    sceneControlAPI.linkData(sceneIdMaster, Color2ProviderId, sceneId1, Color2ConsumerId);
    sceneControlAPI.linkData(sceneIdMaster, Color3ProviderId, sceneId2, Color1ConsumerId);
    sceneControlAPI.linkData(sceneIdMaster, Color4ProviderId, sceneId2, Color2ConsumerId);
    sceneControlAPI.flush();
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
