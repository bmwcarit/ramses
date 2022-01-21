//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include "ramses-client-api/SceneReference.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/RendererSceneControl.h"

#include "ramses-utils.h"

#include <thread>
#include <unordered_map>

class RendererEventHandler : public ramses::RendererEventHandlerEmpty
{
public:
    void windowClosed(ramses::displayId_t /*displayId*/) override
    {
        m_windowClosed = true;
    }

    bool isWindowClosed() const
    {
        return m_windowClosed;
    }

private:
    bool m_windowClosed = false;
};


/** \cond HIDDEN_SYMBOLS */
class SceneReferenceEventHandler final : public ramses::IClientEventHandler
{
public:
    explicit SceneReferenceEventHandler(ramses::RamsesClient& client)
        : m_client(client)
    {}

    virtual void sceneFileLoadFailed(const char*) override {}
    virtual void sceneFileLoadSucceeded(const char*, ramses::Scene*) override {}
    virtual void sceneReferenceFlushed(ramses::SceneReference&, ramses::sceneVersionTag_t) override {}
    virtual void dataLinked(ramses::sceneId_t, ramses::dataProviderId_t, ramses::sceneId_t, ramses::dataConsumerId_t, bool) override {}
    virtual void dataUnlinked(ramses::sceneId_t, ramses::dataConsumerId_t, bool) override {}

    virtual void sceneReferenceStateChanged(ramses::SceneReference& sceneRef, ramses::RendererSceneState state) override
    {
        m_sceneRefState[sceneRef.getReferencedSceneId()] = state;
    }

    void waitForSceneRefState(ramses::sceneId_t sceneId, ramses::RendererSceneState state)
    {
        while (m_sceneRefState.count(sceneId) == 0 || m_sceneRefState.find(sceneId)->second != state)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            m_client.dispatchEvents(*this);
        }
    }

private:
    ramses::RamsesClient& m_client;
    std::unordered_map<ramses::sceneId_t, ramses::RendererSceneState> m_sceneRefState;
};
/** \endcond */

static constexpr unsigned VPWidth = 600;
static constexpr unsigned VPHeight = 400;
static constexpr unsigned DispWidth = 1280;
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

void createContentProviderScene(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    ramses::Scene* clientScene = client.createScene(sceneId);

    ramses::PerspectiveCamera* camera = clientScene->createPerspectiveCamera("camera");
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    camera->setTranslation(0.0f, 0.0f, 5.0f);

    // Bind data objects to scene's camera viewport offset/size and mark as data consumers
    const auto vpOffsetData = clientScene->createDataVector2i("vpOffset");
    const auto vpSizeData = clientScene->createDataVector2i("vpSize");
    vpOffsetData->setValue(0, 0);
    vpSizeData->setValue(VPWidth, VPHeight);
    camera->bindViewportOffset(*vpOffsetData);
    camera->bindViewportSize(*vpSizeData);
    clientScene->createDataConsumer(*vpOffsetData, VPOffsetConsumerId);
    clientScene->createDataConsumer(*vpSizeData, VPSizeConsumerId);

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
    effectDesc.setVertexShader(R"glsl(#version 100
uniform highp mat4 mvpMatrix;
attribute vec3 a_position;
void main() {
    gl_Position = mvpMatrix * vec4(a_position, 1.0);
})glsl");
    effectDesc.setFragmentShader(R"glsl(#version 100
uniform highp vec4 color;
void main(void) {
    gl_FragColor = color + vec4(0.1);
})glsl");
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

    ramses::MeshNode* meshNode2 = clientScene->createMeshNode("triangle mesh node 2");
    ramses::Appearance* appearance2 = clientScene->createAppearance(*effect, "triangle appearance");
    meshNode2->setAppearance(*appearance2);
    meshNode2->setGeometryBinding(*geometry);
    renderGroup->addMeshNode(*meshNode2);
    meshNode2->setTranslation(0, -10, -5);
    meshNode2->setScaling(100, 100, 1);

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

    clientScene->flush();
    clientScene->publish();
}

ramses::Scene* createSceneMaster(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    ramses::Scene* clientScene = client.createScene(sceneId);

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

    clientScene->flush();
    clientScene->publish();

    return clientScene;
}

/**
 * @example ramses-example-local-scene-referencing/src/main.cpp
 * @brief Example for controlling scene state and data linking of scenes via scene referencing instead of renderer API.
 */
int main(int argc, char* argv[])
{
    // create a renderer and a client locally, open a display
    ramses::RamsesFrameworkConfig config(argc, argv);
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);  //needed for automated test of examples
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& client(*framework.createClient("ExampleSceneReferencing"));

    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    renderer.startThread();

    ramses::DisplayConfig displayConfig(argc, argv);
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);
    renderer.flush();

    framework.connect();

    // prepare a master scene and two scenes, which are to be referenced
    const ramses::sceneId_t refSceneId1(1u);
    const ramses::sceneId_t refSceneId2(2u);
    const ramses::sceneId_t sceneIdMaster(3u);

    createContentProviderScene(client, refSceneId1);
    createContentProviderScene(client, refSceneId2);

    ramses::Scene* masterScene = createSceneMaster(client, sceneIdMaster);
    ramses::RendererSceneControl& sceneControlAPI = *renderer.getSceneControlAPI();
    sceneControlAPI.setSceneMapping(sceneIdMaster, display);
    sceneControlAPI.flush();

    /// [Scene referencing setup]
    // create a scene reference for both scenes
    ramses::SceneReference* sceneRef1 = masterScene->createSceneReference(refSceneId1);
    ramses::SceneReference* sceneRef2 = masterScene->createSceneReference(refSceneId2);

    // request scene references state to rendered, they will be brought to state rendered alongside with master scene
    sceneRef1->requestState(ramses::RendererSceneState::Rendered);
    sceneRef2->requestState(ramses::RendererSceneState::Rendered);
    masterScene->flush();
    /// [Scene referencing setup]

    // request master scene to be ready, so scene references can become ready as well
    sceneControlAPI.setSceneState(sceneIdMaster, ramses::RendererSceneState::Ready);
    sceneControlAPI.flush();

    /// [Scene referencing data linking]
    // wait for referenced scene to be available, before trying to data link
    SceneReferenceEventHandler eventHandler(client);
    eventHandler.waitForSceneRefState(refSceneId1, ramses::RendererSceneState::Ready);
    eventHandler.waitForSceneRefState(refSceneId2, ramses::RendererSceneState::Ready);

    // Link master scene's data objects to other scenes cameras' viewports
    masterScene->linkData(nullptr, VP1OffsetProviderId, sceneRef1, VPOffsetConsumerId);
    masterScene->linkData(nullptr, VP1SizeProviderId, sceneRef1, VPSizeConsumerId);
    masterScene->linkData(nullptr, VP2OffsetProviderId, sceneRef2, VPOffsetConsumerId);
    masterScene->linkData(nullptr, VP2SizeProviderId, sceneRef2, VPSizeConsumerId);
    masterScene->flush();
    /// [Scene referencing data linking]

    // Link master scene's data objects to scene1 and scene2 colors
    masterScene->linkData(nullptr, Color1ProviderId, sceneRef1, Color1ConsumerId);
    masterScene->linkData(nullptr, Color2ProviderId, sceneRef1, Color2ConsumerId);
    masterScene->linkData(nullptr, Color3ProviderId, sceneRef2, Color1ConsumerId);
    masterScene->linkData(nullptr, Color4ProviderId, sceneRef2, Color2ConsumerId);
    masterScene->flush();

    // wait and check for data link result events for all links, skipped here for example simplicity

    // Now that everything is set up properly, bring master scene to a rendered state
    sceneControlAPI.setSceneState(sceneIdMaster, ramses::RendererSceneState::Rendered);
    sceneControlAPI.flush();

    // These are master scene's data objects linked to scene1 and scene2 cameras' viewports
    auto scene1vpOffset = ramses::RamsesUtils::TryConvert<ramses::DataVector2i>(*masterScene->findObjectByName("vp1offset"));
    auto scene1vpSize = ramses::RamsesUtils::TryConvert<ramses::DataVector2i>(*masterScene->findObjectByName("vp1size"));
    auto scene2vpOffset = ramses::RamsesUtils::TryConvert<ramses::DataVector2i>(*masterScene->findObjectByName("vp2offset"));
    auto scene2vpSize = ramses::RamsesUtils::TryConvert<ramses::DataVector2i>(*masterScene->findObjectByName("vp2size"));

    // start animating data provider values after scenes are being rendered
    eventHandler.waitForSceneRefState(refSceneId1, ramses::RendererSceneState::Rendered);
    eventHandler.waitForSceneRefState(refSceneId2, ramses::RendererSceneState::Rendered);

    int animParam = 0;
    bool animInc = true;
    RendererEventHandler rendererEventHandler;
    while (!rendererEventHandler.isWindowClosed())
    {
        renderer.dispatchEvents(rendererEventHandler);
        // animate master scene
        scene1vpOffset->setValue(VPWidth / 8 + VPWidth / 2 * animParam / 100, VPHeight / 8 + VPHeight / 4 * animParam / 100);
        scene1vpSize->setValue(VPWidth / 4 + VPWidth / 2 * animParam / 100, VPHeight / 4 + VPHeight / 4 * animParam / 100);
        const auto invAnimParam = 100 - animParam;
        scene2vpOffset->setValue(DispWidth / 2 - VPWidth / 2 * invAnimParam / 100, DispHeight - DispHeight / 4 - VPHeight / 4 * invAnimParam / 100);
        scene2vpSize->setValue(VPWidth / 2 + VPWidth / 2 * invAnimParam / 100, VPHeight / 4 + VPHeight / 4 * invAnimParam / 100);
        masterScene->flush();

        animParam = (animInc ? animParam + 1 : animParam - 1);
        if (animParam == 0 || animParam == 100)
            animInc = !animInc;

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
