//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"
#include "ramses-utils.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include <unordered_set>
#include <thread>

/**
 * @example ramses-example-local-displays/src/main.cpp
 * @brief Example of a local client plus renderer and two displays
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
    auto now = std::chrono::system_clock::now();
    return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
}

ramses::Scene* createScene1(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    //client scene
    ramses::Scene* clientScene = client.createScene(sceneId, ramses::SceneConfig(), "local displays example scene");

    //fill scene with content, i.e. use high level api
    // every scene needs a render pass with camera
    ramses::Camera* camera = clientScene->createRemoteCamera("my camera");
    camera->setTranslation(0.0f, 0.0f, 5.0f);
    ramses::RenderPass* renderPass = clientScene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = clientScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsArray[] = { -1.f, 0.f, -6.f, 1.f, 0.f, -6.f, 0.f, 1.f, -6.f };
    const ramses::Vector3fArray* vertexPositions = client.createConstVector3fArray(3, vertexPositionsArray);
    uint16_t indicesArray[] = { 0, 1, 2 };
    const ramses::UInt16Array* indices = client.createConstUInt16Array(3, indicesArray);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-local-displays-test.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-local-displays-test.frag");
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

    appearance->setInputValueVector4f(colorInput, 1.0f, 1.0f, 0.3f, 1.0f);

    return clientScene;
}

ramses::Scene* createScene2(ramses::RamsesClient& client, ramses::sceneId_t sceneId)
{
    //client scene
    ramses::Scene* clientScene = client.createScene(sceneId, ramses::SceneConfig(), "local displays example scene");

    //fill scene with content, i.e. use high level api
    // every scene needs a render pass with camera
    ramses::Camera* camera = clientScene->createRemoteCamera("my camera");
    camera->setTranslation(0.0f, 0.0f, 5.0f);
    ramses::RenderPass* renderPass = clientScene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = clientScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsArray[] = { -1.1f, 0.f, -6.1f, 1.1f, 0.f, -6.1f, 0.f, 1.1f, -6.1f };
    const ramses::Vector3fArray* vertexPositions = client.createConstVector3fArray(3, vertexPositionsArray);
    uint16_t indicesArray[] = { 2, 0, 1 };
    const ramses::UInt16Array* indices = client.createConstUInt16Array(3, indicesArray);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-local-displays-test.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-local-displays-test.frag");
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

    appearance->setInputValueVector4f(colorInput, 1.0f, 0.0f, 0.5f, 1.0f);

    return clientScene;
}

int main(int argc, char* argv[])
{
    //Ramses client
    ramses::RamsesFrameworkConfig config(argc, argv);
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);  //needed for automated test of examples
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& client(*framework.createClient("ramses-local-client-test"));

    // Ramses renderer
    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    framework.connect();

    ramses::sceneId_t sceneId1(1u);
    ramses::Scene* scene1 = createScene1(client, sceneId1);
    scene1->flush();
    scene1->publish();

    ramses::sceneId_t sceneId2(2u);
    ramses::Scene* scene2 = createScene2(client, sceneId2);
    scene2->flush();
    scene2->publish();

    SceneStateEventHandler eventHandler(renderer);

    eventHandler.waitForPublication(sceneId1);
    eventHandler.waitForPublication(sceneId2);

    //subscribe to scenes
    renderer.subscribeScene(sceneId1);
    renderer.subscribeScene(sceneId2);
    //apply subscriptions immediately, otherwise scene cannot be mapped
    renderer.flush();
    eventHandler.waitForSubscription(sceneId1);
    eventHandler.waitForSubscription(sceneId2);

    /// [Displays Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.
    // Create displays and map scenes to them
    ramses::DisplayConfig displayConfig1(argc, argv);
    displayConfig1.setIntegrityRGLDeviceUnit(0);
    displayConfig1.setWaylandIviSurfaceID(0);
    displayConfig1.setWaylandIviLayerID(3);
    displayConfig1.setWindowIviVisible();
    const ramses::displayId_t display1 = renderer.createDisplay(displayConfig1);
    renderer.mapScene(display1, sceneId1);
    renderer.flush();
    eventHandler.waitForMapped(sceneId1);
    renderer.showScene(sceneId1);

    ramses::DisplayConfig displayConfig2(argc, argv);
    displayConfig2.setIntegrityRGLDeviceUnit(1);
    displayConfig2.setWaylandIviSurfaceID(1);
    displayConfig2.setWaylandIviLayerID(3);
    displayConfig2.setWindowIviVisible();
    const ramses::displayId_t display2 = renderer.createDisplay(displayConfig2);
    renderer.mapScene(display2, sceneId2);
    renderer.flush();
    eventHandler.waitForMapped(sceneId2);
    renderer.showScene(sceneId2);
    /// [Displays Example]

    renderer.flush();

    ramses::MeshNode* meshScene1 = ramses::RamsesUtils::TryConvert<ramses::MeshNode>(*scene1->findObjectByName("triangle mesh node"));
    ramses::MeshNode* meshScene2 = ramses::RamsesUtils::TryConvert<ramses::MeshNode>(*scene2->findObjectByName("triangle mesh node"));
    for (;;)
    {
        renderer.doOneLoop();
        meshScene1->rotate(0.f, 0.f, 15.f);
        scene1->flush();
        meshScene2->rotate(0.f, 0.f, -15.f);
        scene2->flush();
    }
}
