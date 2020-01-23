//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include "ramses-renderer-api/RamsesRenderer.h"
#include "ramses-renderer-api/DisplayConfig.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include <unordered_set>
#include <thread>
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include <iostream>
#include "ramses-client-api/PickableObject.h"
#include "ramses-client-api/PerspectiveCamera.h"
#include "RamsesObjectTypeUtils.h"
#include <map>

/**
 * @example ramses-example-local-pick-handling/src/main.cpp
 * @brief Example of adding touch capability to triangles by creating PickableObjects that cover up the triangles.
 */

constexpr uint32_t DisplayWidth = 1280u;
constexpr uint32_t DisplayHeight = 480u;
constexpr int32_t perspViewportOffsetX = 150;
constexpr int32_t perspViewportOffsetY = 130;
constexpr int32_t perspViewportHeight = 240;
constexpr int32_t perspViewportWidth = 640;

/** \cond HIDDEN_SYMBOLS */
class SceneStateEventHandler : public ramses::RendererEventHandlerEmpty
{
public:
    SceneStateEventHandler(ramses::RamsesRenderer& renderer, ramses::Appearance& appearanceA, ramses::Appearance& appearanceB, ramses::UniformInput& colorInput)
        : m_renderer(renderer)
        , m_appearanceA(appearanceA)
        , m_appearanceB(appearanceB)
        , m_colorInput(colorInput)
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

    void mouseEvent(ramses::displayId_t, ramses::EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY) override
    {
        if (eventType == ramses::EMouseEvent_LeftButtonDown)
        {
            //Flip PosY
            mousePosY = DisplayHeight - mousePosY;

            //normalized coordinate calculation
            const float mouseXNormalized = ((2.0f * static_cast<float>(mousePosX) / DisplayWidth)) - 1.f;
            const float mouseYNormalized = ((2.0f * static_cast<float>(mousePosY) / DisplayHeight)) - 1.f;

            m_renderer.handlePickEvent(ramses::sceneId_t(1u), mouseXNormalized, mouseYNormalized);
            std::cout << "Window was clicked at Coordinates: X = " << mousePosX << " Y = " << mousePosY << "\n";
        }
    }

    virtual void objectsPicked(ramses::sceneId_t sceneId, const ramses::pickableObjectId_t* pickedObjects, uint32_t pickedObjectsCount) override
    {
        (void)sceneId;

        const float r = (static_cast<float>(rand()) / (RAND_MAX));
        const float g = (static_cast<float>(rand()) / (RAND_MAX));
        const float b = (static_cast<float>(rand()) / (RAND_MAX));

        // log all picked objects to the console
        for (uint32_t po = 0; po < pickedObjectsCount; ++po)
        {
            std::cout << "PickableObject with Id: " << pickedObjects[po].getValue() << " has been picked\n";
            // change color of clicked triangles to random value
            if (pickedObjects[po].getValue() == 1)
            {
                m_appearanceA.getEffect().findUniformInput("color", m_colorInput);
                m_appearanceA.setInputValueVector4f(m_colorInput, r, g, b, 1.0f);
            }
            else if (pickedObjects[po].getValue() == 2)
            {
                m_appearanceB.getEffect().findUniformInput("color", m_colorInput);
                m_appearanceB.setInputValueVector4f(m_colorInput, r, g, b, 1.0f);
            }
        }
    }

private:
    typedef std::unordered_set<ramses::sceneId_t> SceneSet;

    void waitForSceneInSet(const ramses::sceneId_t sceneId, const SceneSet& sceneSet)
    {
        while (sceneSet.find(sceneId) == sceneSet.end())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            m_renderer.dispatchEvents(*this);
        }
    }

    SceneSet m_publishedScenes;
    SceneSet m_subscribedScenes;
    SceneSet m_mappedScenes;

    ramses::RamsesRenderer& m_renderer;

    ramses::Appearance& m_appearanceA;
    ramses::Appearance& m_appearanceB;
    ramses::UniformInput& m_colorInput;
};
/** \endcond */

int main(int argc, char* argv[])
{
    //Ramses client
    ramses::RamsesFrameworkConfig config(argc, argv);
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);  //needed for automated test of examples
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& client(*framework.createClient("ramses-local-client-test"));

    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    renderer.startThread();

    ramses::DisplayConfig displayConfig;
    displayConfig.setWindowRectangle(150, 150, DisplayWidth, DisplayHeight);
    displayConfig.setIntegrityRGLDeviceUnit(0);
    displayConfig.setWaylandIviSurfaceID(0);
    displayConfig.setWindowIviVisible();
    displayConfig.setWaylandIviLayerID(3);
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);

    framework.connect();

    //client scene
    const ramses::sceneId_t sceneId(1u);
    ramses::Scene* clientScene = client.createScene(sceneId, ramses::SceneConfig(), "local client example scene");

    // every scene needs a render pass with camera
    ramses::OrthographicCamera* orthographicCamera = clientScene->createOrthographicCamera("my orthographicCamera");
    orthographicCamera->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 100.f);
    orthographicCamera->setViewport(0, 0, 1280, 480);
    orthographicCamera->translate(.2f, 0.f, 5.f);
    orthographicCamera->rotate(0.f, 5.f, 0.f);
    orthographicCamera->scale(1.f, 2.f, 1.f);

    ramses::PerspectiveCamera* perspectiveCamera = clientScene->createPerspectiveCamera("my perspectiveCamera");
    perspectiveCamera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 100.f);
    perspectiveCamera->setViewport(perspViewportOffsetX, perspViewportOffsetY, perspViewportWidth, perspViewportHeight);
    perspectiveCamera->translate(0.f, 0.f, 11.f);
    perspectiveCamera->rotate(0.f, 12.f, 4.f);
    perspectiveCamera->scale(1.f, 2.f, 1.f);
    ramses::RenderPass* renderPassA = clientScene->createRenderPass("my render pass A");
    ramses::RenderPass* renderPassB = clientScene->createRenderPass("my render pass B");
    renderPassA->setClearFlags(ramses::EClearFlags_None);
    renderPassB->setClearFlags(ramses::EClearFlags_None);
    renderPassA->setCamera(*perspectiveCamera);
    renderPassB->setCamera(*orthographicCamera);
    ramses::RenderGroup* renderGroupA = clientScene->createRenderGroup();
    ramses::RenderGroup* renderGroupB = clientScene->createRenderGroup();
    renderPassA->addRenderGroup(*renderGroupA);
    renderPassB->addRenderGroup(*renderGroupB);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsArray[] = { -1.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 1.f, 0.f };
    const ramses::Vector3fArray* vertexPositions = client.createConstVector3fArray(3, vertexPositionsArray);
    uint16_t indicesArray[] = { 0, 1, 2 };
    const ramses::UInt16Array* indices = client.createConstUInt16Array(3, indicesArray);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-local-pick-handling.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-local-pick-handling.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    const ramses::Effect* effect = client.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearanceA = clientScene->createAppearance(*effect, "triangle appearance A");
    ramses::Appearance* appearanceB = clientScene->createAppearance(*effect, "triangle appearance B");
    ramses::GeometryBinding* geometry = clientScene->createGeometryBinding(*effect, "triangle geometry");

    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);

    // create two mesh nodes to define the triangles with chosen appearance
    ramses::MeshNode* meshNode = clientScene->createMeshNode("triangle mesh node");
    ramses::MeshNode* meshNode2 = clientScene->createMeshNode("triangle mesh node 2");

    meshNode->setAppearance(*appearanceA);
    meshNode->setTranslation(1.5f, -0.3f, 0.4f);
    meshNode->setRotation(12.f, 30.f, 10.f);
    meshNode->setScaling(1.2f, 1.5f, 0.7f);
    meshNode->setGeometryBinding(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroupA->addMeshNode(*meshNode);

    meshNode2->setTranslation(1.f, 0.0f, 0.0f);
    meshNode2->setRotation(10.f, 34.f, 19.f);
    meshNode2->setScaling(0.5f, 0.5f, 0.5f);
    meshNode2->setAppearance(*appearanceB);
    meshNode2->setGeometryBinding(*geometry);
    renderGroupB->addMeshNode(*meshNode2);

    appearanceA->setInputValueVector4f(colorInput, 1.0f, 0.0f, 0.3f, 1.0f);
    appearanceB->setInputValueVector4f(colorInput, 0.0f, 1.0f, 0.3f, 1.0f);

    /// [Pick Handling Example]
    // use triangle's vertex position array as PickableObject geometry
    // the two PickableObjects are exactly covering the two triangles
    ramses::VertexDataBuffer* pickableGeometryBuffer = clientScene->createVertexDataBuffer(sizeof(vertexPositionsArray), ramses::EDataType_Vector3F, "geometryBuffer");
    pickableGeometryBuffer->setData(reinterpret_cast<const char*>(vertexPositionsArray), sizeof(vertexPositionsArray));

    ramses::PickableObject* pickableObject1 =  clientScene->createPickableObject(*pickableGeometryBuffer, ramses::pickableObjectId_t(1), "pickableObject");
    pickableObject1->setParent(*meshNode);
    // use the scenes camera for calculating the right intersection point with the PickableObject
    pickableObject1->setCamera(*perspectiveCamera);

    ramses::PickableObject* pickableObject2 = clientScene->createPickableObject(*pickableGeometryBuffer, ramses::pickableObjectId_t(2), "pickableObject2");
    pickableObject2->setParent(*meshNode2);
    pickableObject2->setCamera(*orthographicCamera);

    clientScene->publish();
    clientScene->flush();

    SceneStateEventHandler eventHandler(renderer, *appearanceA, *appearanceB, colorInput);

    // show the scene on the renderer
    eventHandler.waitForPublication(sceneId);

    renderer.subscribeScene(sceneId);
    renderer.flush();
    eventHandler.waitForSubscription(sceneId);

    renderer.mapScene(display, sceneId);
    renderer.flush();
    eventHandler.waitForMapped(sceneId);

    renderer.showScene(sceneId);
    renderer.flush();

    for (;;)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        renderer.dispatchEvents(eventHandler);
        // signal the scene it is in a state that can be rendered
        clientScene->flush();
        renderer.flush();
        /// [Pick Handling Example]
    }
}
