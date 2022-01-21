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
#include "ramses-renderer-api/RendererSceneControl.h"
#include <unordered_set>
#include <thread>

/**
 * @example ramses-example-local-client/src/main.cpp
 * @brief Local Client Example
 */

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

int main(int argc, char* argv[])
{
    //Ramses client
    ramses::RamsesFrameworkConfig config(argc, argv);
    config.setRequestedRamsesShellType(ramses::ERamsesShellType_Console);  //needed for automated test of examples
    ramses::RamsesFramework framework(config);
    ramses::RamsesClient& client(*framework.createClient("ramses-local-client-test"));

    ramses::RendererConfig rendererConfig(argc, argv);
    ramses::RamsesRenderer& renderer(*framework.createRenderer(rendererConfig));
    ramses::RendererSceneControl& sceneControlAPI = *renderer.getSceneControlAPI();
    renderer.startThread();

    ramses::DisplayConfig displayConfig(argc, argv);
    const ramses::displayId_t display = renderer.createDisplay(displayConfig);
    renderer.flush();

    //client scene
    const ramses::sceneId_t sceneId(1u);
    ramses::Scene* clientScene = client.createScene(sceneId, ramses::SceneConfig(), "local client example scene");

    // every scene needs a render pass with camera
    auto* camera = clientScene->createPerspectiveCamera("my camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    ramses::RenderPass* renderPass = clientScene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = clientScene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsArray[] = { -1.f, 0.f, -6.f, 1.f, 0.f, -6.f, 0.f, 1.f, -6.f };
    ramses::ArrayResource* vertexPositions = clientScene->createArrayResource(ramses::EDataType::Vector3F, 3, vertexPositionsArray);
    uint16_t indicesArray[] = { 0, 1, 2 };
    ramses::ArrayResource* indices = clientScene->createArrayResource(ramses::EDataType::UInt16, 3, indicesArray);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-local-client-test.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-local-client-test.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    ramses::Effect* effect = clientScene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
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
    meshNode->setTranslation(0.0f, 0.0f, -5.0f);
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);

    ramses::AnimationSystemRealTime* animationSystem = clientScene->createRealTimeAnimationSystem(ramses::EAnimationSystemFlags_Default, "animation system");

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

    clientScene->publish(ramses::EScenePublicationMode_LocalOnly);
    clientScene->flush();

    // show the scene on the renderer
    sceneControlAPI.setSceneMapping(sceneId, display);
    sceneControlAPI.setSceneState(sceneId, ramses::RendererSceneState::Rendered);
    sceneControlAPI.flush();

    RendererEventHandler eventHandler;
    while (!eventHandler.isWindowClosed())
    {
        renderer.dispatchEvents(eventHandler);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}
