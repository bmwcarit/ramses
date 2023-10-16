//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/client/ramses-client.h"

#include "ramses/renderer/RamsesRenderer.h"
#include "ramses/renderer/DisplayConfig.h"
#include "ramses/renderer/IRendererEventHandler.h"
#include "ramses/renderer/RendererSceneControl.h"
#include "ramses/framework/RamsesFrameworkTypes.h"
#include "ramses/client/PickableObject.h"
#include "ramses/client/PerspectiveCamera.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include <unordered_set>
#include <thread>
#include <iostream>
#include <map>
#include <random>

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
class SceneStateEventHandler : public ramses::RendererEventHandlerEmpty, public ramses::RendererSceneControlEventHandlerEmpty
{
public:
    SceneStateEventHandler(ramses::RendererSceneControl& sceneControlApi, ramses::Appearance& appearanceA, ramses::Appearance& appearanceB, ramses::UniformInput& colorInput)
        : m_sceneControlApi(sceneControlApi)
        , m_appearanceA(appearanceA)
        , m_appearanceB(appearanceB)
        , m_colorInput(colorInput)
    {
    }

    void mouseEvent(ramses::displayId_t /*displayId*/, ramses::EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY) override
    {
        if (eventType == ramses::EMouseEvent::LeftButtonDown)
        {
            //Flip PosY
            mousePosY = static_cast<int32_t>(DisplayHeight) - mousePosY;

            //normalized coordinate calculation
            const float mouseXNormalized = ((2.0f * static_cast<float>(mousePosX) / DisplayWidth)) - 1.f;
            const float mouseYNormalized = ((2.0f * static_cast<float>(mousePosY) / DisplayHeight)) - 1.f;

            m_sceneControlApi.handlePickEvent(ramses::sceneId_t(1u), mouseXNormalized, mouseYNormalized);
            std::cout << "Window was clicked at Coordinates: X = " << mousePosX << " Y = " << mousePosY << "\n";
        }
    }

    void objectsPicked(ramses::sceneId_t /*sceneId*/, const ramses::pickableObjectId_t* pickedObjects, size_t pickedObjectsCount) override
    {
        std::uniform_real_distribution<float> dist;
        const float r = dist(m_randomGenerator);
        const float g = dist(m_randomGenerator);
        const float b = dist(m_randomGenerator);

        // log all picked objects to the console
        for (size_t po = 0; po < pickedObjectsCount; ++po)
        {
            std::cout << "PickableObject with Id: " << pickedObjects[po].getValue() << " has been picked\n";
            // change color of clicked triangles to random value
            if (pickedObjects[po].getValue() == 1)
            {
                std::optional<ramses::UniformInput> colorInput = m_appearanceA.getEffect().findUniformInput("color");
                assert(colorInput.has_value());
                m_colorInput = std::move(*colorInput);
                m_appearanceA.setInputValue(m_colorInput, ramses::vec4f{ r, g, b, 1.0f });
            }
            else if (pickedObjects[po].getValue() == 2)
            {
                std::optional<ramses::UniformInput> colorInput = m_appearanceB.getEffect().findUniformInput("color");
                assert(colorInput.has_value());
                m_colorInput = std::move(*colorInput);
                m_appearanceB.setInputValue(m_colorInput, ramses::vec4f{ r, g, b, 1.0f });
            }
        }
    }

    void windowClosed(ramses::displayId_t /*displayId*/) override
    {
        m_windowClosed = true;
    }

    [[nodiscard]] bool isWindowClosed() const
    {
        return m_windowClosed;
    }

private:
    ramses::RendererSceneControl& m_sceneControlApi;
    ramses::Appearance& m_appearanceA;
    ramses::Appearance& m_appearanceB;
    ramses::UniformInput& m_colorInput;
    std::mt19937 m_randomGenerator{std::random_device{}()};
    bool m_windowClosed = false;
};
/** \endcond */

int main()
{
    //Ramses client
    ramses::RamsesFrameworkConfig config{ramses::EFeatureLevel_Latest};
    config.setRequestedRamsesShellType(ramses::ERamsesShellType::Console);  //needed for automated test of examples
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& client(*framework.createClient("ramses-local-client-test"));

    ramses::RendererConfig rendererConfig;
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    auto& sceneControlAPI = *renderer.getSceneControlAPI();
    renderer.startThread();

    ramses::DisplayConfig displayConfig;
    displayConfig.setWindowRectangle(150, 150, DisplayWidth, DisplayHeight);
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);
    renderer.flush();

    framework.connect();

    //client scene
    const ramses::sceneId_t sceneId(1u);
    ramses::Scene* clientScene = client.createScene(sceneId, "local client example scene");

    // every scene needs a render pass with camera
    ramses::OrthographicCamera* orthographicCamera = clientScene->createOrthographicCamera("my orthographicCamera");
    orthographicCamera->setFrustum(-1.f, 1.f, -1.f, 1.f, 0.1f, 100.f);
    orthographicCamera->setViewport(0, 0, 1280, 480);
    orthographicCamera->translate({.2f, 0.f, 5.f});
    orthographicCamera->setRotation({0.f, -5.f, 0.f}, ramses::ERotationType::Euler_XYZ);
    orthographicCamera->scale({1.f, 2.f, 1.f});

    ramses::PerspectiveCamera* perspectiveCamera = clientScene->createPerspectiveCamera("my perspectiveCamera");
    perspectiveCamera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 100.f);
    perspectiveCamera->setViewport(perspViewportOffsetX, perspViewportOffsetY, perspViewportWidth, perspViewportHeight);
    perspectiveCamera->translate({0.f, 0.f, 11.f});
    perspectiveCamera->setRotation({0.f, -12.f, -4.f}, ramses::ERotationType::Euler_XYZ);
    perspectiveCamera->scale({1.f, 2.f, 1.f});
    ramses::RenderPass* renderPassA = clientScene->createRenderPass("my render pass A");
    ramses::RenderPass* renderPassB = clientScene->createRenderPass("my render pass B");
    renderPassA->setClearFlags(ramses::EClearFlag::None);
    renderPassB->setClearFlags(ramses::EClearFlag::None);
    renderPassA->setCamera(*perspectiveCamera);
    renderPassB->setCamera(*orthographicCamera);
    ramses::RenderGroup* renderGroupA = clientScene->createRenderGroup();
    ramses::RenderGroup* renderGroupB = clientScene->createRenderGroup();
    renderPassA->addRenderGroup(*renderGroupA);
    renderPassB->addRenderGroup(*renderGroupB);

    // prepare triangle geometry: vertex position array and index array
    const std::vector<ramses::vec3f> vertexPositionsArray{
        ramses::vec3f{ -1.f, 0.f, 0.f },
        ramses::vec3f{ 1.f, 0.f, 0.f },
        ramses::vec3f{ 0.f, 1.f, 0.f } };

    ramses::ArrayResource* vertexPositions = clientScene->createArrayResource(3u, vertexPositionsArray.data());
    const std::array<uint16_t, 3u> indexData{ 0, 1, 2 };
    ramses::ArrayResource* indices = clientScene->createArrayResource(3u, indexData.data());

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-local-pick-handling.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-local-pick-handling.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effect = clientScene->createEffect(effectDesc, "glsl shader");
    ramses::Appearance* appearanceA = clientScene->createAppearance(*effect, "triangle appearance A");
    ramses::Appearance* appearanceB = clientScene->createAppearance(*effect, "triangle appearance B");
    ramses::Geometry* geometry = clientScene->createGeometry(*effect, "triangle geometry");

    geometry->setIndices(*indices);
    std::optional<ramses::AttributeInput> positionsInput = effect->findAttributeInput("a_position");
    assert(positionsInput.has_value());
    geometry->setInputBuffer(*positionsInput, *vertexPositions);

    std::optional<ramses::UniformInput> colorInput = effect->findUniformInput("color");
    assert(colorInput.has_value());

    // create two mesh nodes to define the triangles with chosen appearance
    ramses::MeshNode* meshNode = clientScene->createMeshNode("triangle mesh node");
    ramses::MeshNode* meshNode2 = clientScene->createMeshNode("triangle mesh node 2");

    meshNode->setAppearance(*appearanceA);
    meshNode->setTranslation({1.5f, -0.3f, 0.4f});
    meshNode->setRotation({-12.f, -30.f, -10.f}, ramses::ERotationType::Euler_XYZ);
    meshNode->setScaling({1.2f, 1.5f, 0.7f});
    meshNode->setGeometry(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroupA->addMeshNode(*meshNode);

    meshNode2->setTranslation({1.f, 0.0f, 0.0f});
    meshNode2->setRotation({-10.f, -34.f, -19.f}, ramses::ERotationType::Euler_XYZ);
    meshNode2->setScaling({0.5f, 0.5f, 0.5f});
    meshNode2->setAppearance(*appearanceB);
    meshNode2->setGeometry(*geometry);
    renderGroupB->addMeshNode(*meshNode2);

    appearanceA->setInputValue(*colorInput, ramses::vec4f{ 1.0f, 0.0f, 0.3f, 1.0f });
    appearanceB->setInputValue(*colorInput, ramses::vec4f{ 0.0f, 1.0f, 0.3f, 1.0f });

    /// [Pick Handling Example]
    // use triangle's vertex position array as PickableObject geometry
    // the two PickableObjects are exactly covering the two triangles
    ramses::ArrayBuffer* pickableGeometryBuffer = clientScene->createArrayBuffer(ramses::EDataType::Vector3F, 3u, "geometryBuffer");
    pickableGeometryBuffer->updateData(0u, 3u, vertexPositionsArray.data());

    ramses::PickableObject* pickableObject1 =  clientScene->createPickableObject(*pickableGeometryBuffer, ramses::pickableObjectId_t(1), "pickableObject");
    pickableObject1->setParent(*meshNode);
    // use the scenes camera for calculating the right intersection point with the PickableObject
    pickableObject1->setCamera(*perspectiveCamera);

    ramses::PickableObject* pickableObject2 = clientScene->createPickableObject(*pickableGeometryBuffer, ramses::pickableObjectId_t(2), "pickableObject2");
    pickableObject2->setParent(*meshNode2);
    pickableObject2->setCamera(*orthographicCamera);

    clientScene->publish();
    clientScene->flush();

    SceneStateEventHandler eventHandler(sceneControlAPI, *appearanceA, *appearanceB, *colorInput);

    // show the scene on the renderer
    sceneControlAPI.setSceneMapping(sceneId, display);
    sceneControlAPI.setSceneState(sceneId, ramses::RendererSceneState::Rendered);
    sceneControlAPI.flush();

    while (!eventHandler.isWindowClosed())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(15));
        renderer.dispatchEvents(eventHandler);
        sceneControlAPI.dispatchEvents(eventHandler);
        // signal the scene it is in a state that can be rendered
        clientScene->flush();
        sceneControlAPI.flush();
        /// [Pick Handling Example]
    }
}
