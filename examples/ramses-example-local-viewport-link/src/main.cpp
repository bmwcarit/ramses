//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
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

    void sceneStateChanged(ramses::sceneId_t sceneId, ramses::RendererSceneState state) override
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
    using SceneSet = std::unordered_map<ramses::sceneId_t, ramses::RendererSceneState>;

    ramses::RamsesRenderer& m_renderer;
    SceneSet m_scenes;
};
/** \endcond */
static constexpr int32_t VPWidth = 600;
static constexpr int32_t VPHeight = 400;
static constexpr int32_t DispWidth  = 1280;
static constexpr int32_t DispHeight = 480;

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
    camera->setTranslation({0.0f, 0.0f, 5.0f});

    /// [Viewport Link Example Content]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.
    // Bind data objects to scene's camera viewport offset/size and mark as data consumers
    const auto vpOffsetData = clientScene->createDataObject(ramses::EDataType::Vector2I, "vpOffset");
    const auto vpSizeData = clientScene->createDataObject(ramses::EDataType::Vector2I, "vpSize");
    vpOffsetData->setValue(ramses::vec2i{ 0, 0 });
    vpSizeData->setValue(ramses::vec2i{ VPWidth, VPHeight });
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

    const std::array<ramses::vec3f, 3u> vertexPositionsData{ ramses::vec3f{-1.f, 0.f, -6.f}, ramses::vec3f{1.f, 0.f, -6.f}, ramses::vec3f{0.f, 1.f, -6.f} };
    ramses::ArrayResource* vertexPositions = clientScene->createArrayResource(3u, vertexPositionsData.data());
    const std::array<uint16_t, 3u> indexData{ 0, 1, 2 };
    ramses::ArrayResource* indices = clientScene->createArrayResource(3u, indexData.data());

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

    ramses::MeshNode*   meshNode2   = clientScene->createMeshNode("triangle mesh node 2");
    ramses::Appearance* appearance2 = clientScene->createAppearance(*effect, "triangle appearance");
    meshNode2->setAppearance(*appearance2);
    meshNode2->setGeometryBinding(*geometry);
    renderGroup->addMeshNode(*meshNode2);
    meshNode2->setTranslation({0, -10, -5});
    meshNode2->setScaling({100, 100, 1});

    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);

    // Create data objects to hold color and bind them to appearance inputs
    auto color1 = clientScene->createDataObject(ramses::EDataType::Vector4F);
    auto color2 = clientScene->createDataObject(ramses::EDataType::Vector4F);
    color1->setValue(ramses::vec4f{ 1.f, 1.f, 1.f, 1.f });
    color1->setValue(ramses::vec4f{ 0.5f, 0.5f, 0.5f, 1.f });
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
    const auto vp1offset = clientScene->createDataObject(ramses::EDataType::Vector2I, "vp1offset");
    const auto vp1size = clientScene->createDataObject(ramses::EDataType::Vector2I, "vp1size");
    const auto vp2offset = clientScene->createDataObject(ramses::EDataType::Vector2I, "vp2offset");
    const auto vp2size = clientScene->createDataObject(ramses::EDataType::Vector2I, "vp2size");

    vp1offset->setValue(ramses::vec2i{ 0, 0 });
    vp1size->setValue(ramses::vec2i{ DispWidth, DispHeight });
    vp2offset->setValue(ramses::vec2i{ 0, 0 });
    vp2size->setValue(ramses::vec2i{ DispWidth, DispHeight });

    clientScene->createDataProvider(*vp1offset, VP1OffsetProviderId);
    clientScene->createDataProvider(*vp1size, VP1SizeProviderId);
    clientScene->createDataProvider(*vp2offset, VP2OffsetProviderId);
    clientScene->createDataProvider(*vp2size, VP2SizeProviderId);
    /// [Viewport Link Example Master]

    // Create data objects and mark them as providers to control content scenes colors
    const auto color1 = clientScene->createDataObject(ramses::EDataType::Vector4F, "color1");
    const auto color2 = clientScene->createDataObject(ramses::EDataType::Vector4F, "color2");
    const auto color3 = clientScene->createDataObject(ramses::EDataType::Vector4F, "color3");
    const auto color4 = clientScene->createDataObject(ramses::EDataType::Vector4F, "color4");

    color1->setValue(ramses::vec4f{ 1.f, 1.f, 0.3f, 1.f });
    color2->setValue(ramses::vec4f{ 0.f, 0.3f, 0.5f, 1.f });
    color3->setValue(ramses::vec4f{ 1.f, 0.f, 0.5f, 1.f });
    color4->setValue(ramses::vec4f{ 0.5f, 0.3f, 0.f, 1.f });

    clientScene->createDataProvider(*color1, Color1ProviderId);
    clientScene->createDataProvider(*color2, Color2ProviderId);
    clientScene->createDataProvider(*color3, Color3ProviderId);
    clientScene->createDataProvider(*color4, Color4ProviderId);

    return clientScene;
}

int main()
{
    // Ramses client
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    config.setRequestedRamsesShellType(ramses::ERamsesShellType::Console);  //needed for automated test of examples
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& client(*framework.createClient("ramses-local-client-test"));

    // Ramses renderer
    ramses::RendererConfig rendererConfig;
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

    ramses::DisplayConfig displayConfig;
    displayConfig.setWindowRectangle(50, 50, DispWidth, DispHeight);
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);
    renderer.flush();

    sceneControlAPI.setSceneMapping(sceneId1, display);
    sceneControlAPI.setSceneMapping(sceneId2, display);
    sceneControlAPI.setSceneMapping(sceneIdMaster, display);
    sceneControlAPI.setSceneState(sceneId1, ramses::RendererSceneState::Rendered);
    sceneControlAPI.setSceneState(sceneId2, ramses::RendererSceneState::Rendered);
    // Master scene does not need to be rendered, it only provides data to the other 2 scenes,
    // it must be at least in Ready state meaning its data is on renderer
    sceneControlAPI.setSceneState(sceneIdMaster, ramses::RendererSceneState::Ready);
    sceneControlAPI.flush();

    eventHandler.waitForSceneState(sceneId1, ramses::RendererSceneState::Rendered);
    eventHandler.waitForSceneState(sceneId2, ramses::RendererSceneState::Rendered);
    eventHandler.waitForSceneState(sceneIdMaster, ramses::RendererSceneState::Ready);

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
    auto scene1vpOffset = ramses::RamsesUtils::TryConvert<ramses::DataObject>(*sceneMaster->findObjectByName("vp1offset"));
    auto scene1vpSize = ramses::RamsesUtils::TryConvert<ramses::DataObject>(*sceneMaster->findObjectByName("vp1size"));
    auto scene2vpOffset = ramses::RamsesUtils::TryConvert<ramses::DataObject>(*sceneMaster->findObjectByName("vp2offset"));
    auto scene2vpSize = ramses::RamsesUtils::TryConvert<ramses::DataObject>(*sceneMaster->findObjectByName("vp2size"));

    int animParam = 0;
    bool animInc = true;

    ramses::MeshNode* meshScene1 = ramses::RamsesUtils::TryConvert<ramses::MeshNode>(*scene1->findObjectByName("triangle mesh node"));
    ramses::MeshNode* meshScene2 = ramses::RamsesUtils::TryConvert<ramses::MeshNode>(*scene2->findObjectByName("triangle mesh node"));

    RendererEventHandler rendererEventHandler;
    float rotationFactor = 0.f;
    while (!rendererEventHandler.isWindowClosed())
    {
        renderer.dispatchEvents(rendererEventHandler);

        // animate master scene
        scene1vpOffset->setValue(ramses::vec2i{ VPWidth / 8 + VPWidth / 2 * animParam / 100, VPHeight / 8 + VPHeight / 4 * animParam / 100 });
        scene1vpSize->setValue(ramses::vec2i{ VPWidth / 4 + VPWidth / 2 * animParam / 100, VPHeight / 4 + VPHeight / 4 * animParam / 100 });
        const auto invAnimParam = 100 - animParam;
        scene2vpOffset->setValue(ramses::vec2i{ DispWidth / 2 - VPWidth / 2 * invAnimParam / 100, DispHeight - DispHeight / 4 - VPHeight / 4 * invAnimParam / 100 });
        scene2vpSize->setValue(ramses::vec2i{ VPWidth / 2 + VPWidth / 2 * invAnimParam / 100, VPHeight / 4 + VPHeight / 4 * invAnimParam / 100 });
        sceneMaster->flush();

        animParam = (animInc ? animParam + 1 : animParam - 1);
        if (animParam == 0 || animParam == 100)
            animInc = !animInc;

        // animate content inside provider scenes
        rotationFactor += 1.f;
        meshScene1->setRotation({0.f, 0.f, 2 * rotationFactor}, ramses::ERotationType::Euler_XYZ);
        scene1->flush();
        meshScene2->setRotation({0.f, 0.f, 4 * rotationFactor}, ramses::ERotationType::Euler_XYZ);
        scene2->flush();

        renderer.doOneLoop();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
