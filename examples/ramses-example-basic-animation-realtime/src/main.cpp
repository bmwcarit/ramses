//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-client.h"

#include <cstdio>
#include <thread>
#include <cstdlib>
#include <chrono>

/**
 * @example ramses-example-basic-animation-realtime/src/main.cpp
 * @brief Basic Realtime Animation Example
 */

int main(int argc, char* argv[])
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient& ramses(*framework.createClient("ramses-example-basic-animation-realtime"));
    framework.connect();

    // create a scene for distributing content
    ramses::Scene* scene = ramses.createScene(ramses::sceneId_t(123u), ramses::SceneConfig(), "red triangles scene");

    // every scene needs a render pass with camera
    auto* camera = scene->createPerspectiveCamera("my camera");
    camera->setViewport(0, 0, 1280u, 480u);
    camera->setFrustum(19.f, 1280.f / 480.f, 0.1f, 1500.f);
    camera->setTranslation(0.0f, 0.0f, 5.0f);
    ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsData[] = { -1.f, 0.f, -1.f, 1.f, 0.f, -1.f, 0.f, 1.f, -1.f };
    ramses::ArrayResource* vertexPositions = scene->createArrayResource(ramses::EDataType::Vector3F, 3, vertexPositionsData);
    uint16_t indexData[] = {0, 1, 2};
    ramses::ArrayResource* indices = scene->createArrayResource(ramses::EDataType::UInt16, 3, indexData);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-basic-animation-realtime.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-animation-realtime.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic::ModelViewProjectionMatrix);

    const ramses::Effect* effect = scene->createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = scene->createAppearance(*effect, "triangle appearance");
    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect, "triangle geometry");

    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    // create a mesh nodes to define the triangles with chosen appearance
    ramses::MeshNode* meshNode1 = scene->createMeshNode("red triangle mesh node1");
    meshNode1->setAppearance(*appearance);
    meshNode1->setGeometryBinding(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode1);

    ramses::MeshNode* meshNode2 = scene->createMeshNode("red triangle mesh node2");
    meshNode2->setAppearance(*appearance);
    meshNode2->setGeometryBinding(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode2);

    ramses::MeshNode* meshNode3 = scene->createMeshNode("red triangle mesh node3");
    meshNode3->setAppearance(*appearance);
    meshNode3->setGeometryBinding(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode3);

    /// [Basic Realtime Animation Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // create real time animation system, provide creation flag
    // EAnimationSystemFlags_ClientSideProcessing to get live updates of animated properties
    ramses::AnimationSystemRealTime* animationSystem = scene->createRealTimeAnimationSystem(ramses::EAnimationSystemFlags_ClientSideProcessing, "animation system");

    // create splines with animation keys
    ramses::SplineLinearFloat* spline1 = animationSystem->createSplineLinearFloat("spline1");
    spline1->setKey(0u, 0.f);
    spline1->setKey(5000u, -1.f);
    spline1->setKey(10000u, 0.f);
    ramses::SplineLinearFloat* spline2 = animationSystem->createSplineLinearFloat("spline2");
    spline2->setKey(0u, 0.f);
    spline2->setKey(5000u, 1.f);
    spline2->setKey(10000u, 0.f);

    // create animated property for each translation node with single component animation
    ramses::AnimatedProperty* animProperty1 = animationSystem->createAnimatedProperty(*meshNode1, ramses::EAnimatedProperty_Translation, ramses::EAnimatedPropertyComponent_X);
    ramses::AnimatedProperty* animProperty2 = animationSystem->createAnimatedProperty(*meshNode2, ramses::EAnimatedProperty_Translation, ramses::EAnimatedPropertyComponent_X);
    ramses::AnimatedProperty* animProperty3 = animationSystem->createAnimatedProperty(*meshNode3, ramses::EAnimatedProperty_Translation, ramses::EAnimatedPropertyComponent_Y);

    // create three animations
    ramses::Animation* animation1 = animationSystem->createAnimation(*animProperty1, *spline1, "animation1");
    ramses::Animation* animation2 = animationSystem->createAnimation(*animProperty2, *spline2, "animation2");
    ramses::Animation* animation3 = animationSystem->createAnimation(*animProperty3, *spline1, "animation3"); // we can reuse spline1 for animating Y component of the third translation node

    // create animation sequence
    ramses::AnimationSequence* animSequence = animationSystem->createAnimationSequence();

    // add animations to a sequence
    animSequence->addAnimation(*animation1);
    animSequence->addAnimation(*animation2);
    animSequence->addAnimation(*animation3);

    // set animation properties (optional)
    animSequence->setAnimationLooping(*animation1);
    animSequence->setAnimationLooping(*animation2);
    animSequence->setAnimationLooping(*animation3);

    // set sequence playback speed
    animSequence->setPlaybackSpeed(5.f);

    // start animation sequence
    // we use real time animation system, therefore we should provide system time
    // because renderer will be using system time to update its animation system every frame
    auto now = std::chrono::system_clock::now();
    animSequence->startAt(std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count());

    // signal the scene it is in a state that can be rendered
    scene->flush();

    // distribute the scene to RAMSES
    scene->publish();

    // no application logic is needed here
    // real time animation system is updated automatically on renderer every frame using system time

    float x;
    float y;
    float z;

    for (int ii = 0; ii < 500; ++ii)
    {
        // with RealTimeAnimationSystem, updateLocalTime() has to be called
        // to get up-to-date animated values
        animationSystem->updateLocalTime();

        // print current value of animated property
        meshNode1->getTranslation(x, y, z);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    // however local client animation system should be updated before any change is made
    animationSystem->updateLocalTime();
    animSequence->setPlaybackSpeed(10.f);

    for (int ii = 0; ii < 500; ++ii)
    {
        // with RealTimeAnimationSystem, updateLocalTime() has to be called
        // to get up-to-date animated values
        animationSystem->updateLocalTime();

        // print current value of animated property
        meshNode1->getTranslation(x, y, z);

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    /// [Basic Realtime Animation Example]

    // shutdown: stop distribution, free resources, unregister
    scene->destroy(*animationSystem);
    scene->unpublish();
    scene->destroy(*vertexPositions);
    scene->destroy(*indices);
    ramses.destroy(*scene);
    framework.disconnect();

    return 0;
}
