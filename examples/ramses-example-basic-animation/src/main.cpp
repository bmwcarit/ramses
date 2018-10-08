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
#include <chrono>

/**
 * @example ramses-example-basic-animation/src/main.cpp
 * @brief Basic Animation Example
 */

int main(int argc, char* argv[])
{
    // register at RAMSES daemon
    ramses::RamsesFramework framework(argc, argv);
    ramses::RamsesClient ramses("ramses-example-basic-animation", framework);
    framework.connect();

    // create a scene for distributing content
    ramses::Scene* scene = ramses.createScene(123u, ramses::SceneConfig(), "red triangle scene");

    // every scene needs a render pass with camera
    ramses::TranslateNode* cameraTranslate = scene->createTranslateNode();
    cameraTranslate->setTranslation(0.0f, 0.0f, 5.0f);
    ramses::Camera* camera = scene->createRemoteCamera("my camera");
    camera->setParent(*cameraTranslate);
    ramses::RenderPass* renderPass = scene->createRenderPass("my render pass");
    renderPass->setClearFlags(ramses::EClearFlags_None);
    renderPass->setCamera(*camera);
    ramses::RenderGroup* renderGroup = scene->createRenderGroup();
    renderPass->addRenderGroup(*renderGroup);

    // prepare triangle geometry: vertex position array and index array
    float vertexPositionsData[] = { -1.f, 0.f, -1.f, 1.f, 0.f, -1.f, 0.f, 1.f, -1.f };
    const ramses::Vector3fArray* vertexPositions = ramses.createConstVector3fArray(3, vertexPositionsData);
    uint16_t indexData[] = {0, 1, 2};
    const ramses::UInt16Array* indices = ramses.createConstUInt16Array(3, indexData);

    // create an appearance for red triangle
    ramses::EffectDescription effectDesc;
    effectDesc.setVertexShaderFromFile("res/ramses-example-basic-animation.vert");
    effectDesc.setFragmentShaderFromFile("res/ramses-example-basic-animation.frag");
    effectDesc.setUniformSemantic("mvpMatrix", ramses::EEffectUniformSemantic_ModelViewProjectionMatrix);

    ramses::Effect* effect = ramses.createEffect(effectDesc, ramses::ResourceCacheFlag_DoNotCache, "glsl shader");
    ramses::Appearance* appearance = scene->createAppearance(*effect, "triangle appearance");
    ramses::GeometryBinding* geometry = scene->createGeometryBinding(*effect, "triangle geometry");

    geometry->setIndices(*indices);
    ramses::AttributeInput positionsInput;
    effect->findAttributeInput("a_position", positionsInput);
    geometry->setInputBuffer(positionsInput, *vertexPositions);

    // create a mesh node to define the triangle with chosen appearance
    ramses::MeshNode* meshNode = scene->createMeshNode("red triangle mesh node");
    meshNode->setAppearance(*appearance);
    meshNode->setGeometryBinding(*geometry);
    // mesh needs to be added to a render group that belongs to a render pass with camera in order to be rendered
    renderGroup->addMeshNode(*meshNode);

    /// [Basic Animation Example]
    // IMPORTANT NOTE: For simplicity and readability the example code does not check return values from API calls.
    //                 This should not be the case for real applications.

    // create animation system, provide creation flag
    // EAnimationSystemFlags_LiveUpdates to get live updates of animated properties
    ramses::AnimationSystem* animationSystem = scene->createAnimationSystem(ramses::EAnimationSystemFlags_ClientSideProcessing, "animation system");

    // create spline with animation keys
    ramses::SplineLinearFloat* colorSpline = animationSystem->createSplineLinearFloat("color spline");
    colorSpline->setKey(0u, 0.f);
    colorSpline->setKey(5000u, 1.f);
    colorSpline->setKey(10000u, 0.f);

    // get handle to appearance input
    ramses::UniformInput colorInput;
    effect->findUniformInput("color", colorInput);
    appearance->setInputValueVector4f(colorInput, 1.f, 0.f, 0.f, 1.f);

    // create animated property for color input with single component animation (green color channel is referred to via Z component)
    ramses::AnimatedProperty* colorAnimProperty = animationSystem->createAnimatedProperty(colorInput, *appearance, ramses::EAnimatedPropertyComponent_Y);

    // create animation
    ramses::Animation* animation = animationSystem->createAnimation(*colorAnimProperty, *colorSpline, "color animation");

    // create animation sequence and add animation
    ramses::AnimationSequence* sequence = animationSystem->createAnimationSequence();
    sequence->addAnimation(*animation);

    // set animation properties (optional)
    sequence->setAnimationLooping(*animation);

    // start sequence
    sequence->setPlaybackSpeed(5.f);
    sequence->startAt(0u);

    // signal the scene it is in a state that can be rendered
    scene->flush();

    // distribute the scene to RAMSES
    scene->publish();

    // application logic

    float x;
    float y;
    float z;
    float w;

    for (uint64_t timeStamp = 0u; timeStamp < 10000u; timeStamp += 20u)
    {
        animationSystem->setTime(timeStamp);

        // print current value of animated property
        appearance->getInputValueVector4f(colorInput, x, y, z, w);
        printf("%f %f %f %f\n", x, y, z, w);

        // signal the scene it is in a state that can be rendered
        scene->flush();

        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    /// [Basic Animation Example]

    // shutdown: stop distribution, free resources, unregister
    scene->destroy(*animationSystem);
    scene->unpublish();
    ramses.destroy(*scene);
    ramses.destroy(*vertexPositions);
    ramses.destroy(*indices);
    framework.disconnect();

    return 0;
}
