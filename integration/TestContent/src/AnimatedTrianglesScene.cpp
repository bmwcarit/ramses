//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TestScenes/AnimatedTrianglesScene.h"
#include "TestScenes/Triangle.h"
#include "ramses-client-api/Scene.h"
#include "ramses-client-api/AnimationSystem.h"
#include "ramses-client-api/AnimatedProperty.h"
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/SplineStepBool.h"
#include "ramses-client-api/SplineLinearVector3f.h"
#include "ramses-client-api/SplineLinearVector4f.h"
#include "ramses-client-api/SplineBezierFloat.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/Appearance.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/DataVector4f.h"
#include "ramses-client-api/Effect.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformAbstraction/PlatformMath.h"

namespace ramses_internal
{
    const UInt64 AnimatedTrianglesScene::StopTime[NUM_STATES] = { 2000u, 3000u, 4000u, 5000u, 6000u, 60000u };

    AnimatedTrianglesScene::AnimatedTrianglesScene(ramses::RamsesClient& ramsesClient, ramses::Scene& scene, UInt32 state, const Vector3& cameraPosition)
        : IntegrationScene(ramsesClient, scene, cameraPosition)
    {
        state = state % NUM_STATES;

        ramses::Effect* effect = getTestEffect("ramses-test-client-basic");
        ramses::Triangle redTriangle(m_client, m_scene, *effect, ramses::TriangleAppearance::EColor_Red);
        ramses::Appearance& appearance = redTriangle.GetAppearance();
        ramses::GeometryBinding& geometry = redTriangle.GetGeometry();

        // create a mesh node to define the triangle with chosen appearance
        ramses::MeshNode* meshNode = m_scene.createMeshNode("red triangle mesh node");
        addMeshNodeToDefaultRenderGroup(*meshNode);
        meshNode->setAppearance(appearance);
        meshNode->setGeometryBinding(geometry);
        ramses::MeshNode* meshNode2 = m_scene.createMeshNode("triangle mesh node");
        addMeshNodeToDefaultRenderGroup(*meshNode2);
        meshNode2->setAppearance(appearance);
        meshNode2->setGeometryBinding(geometry);
        ramses::MeshNode* meshNode3 = m_scene.createMeshNode("circle mesh node");
        addMeshNodeToDefaultRenderGroup(*meshNode3);
        meshNode3->setAppearance(appearance);
        meshNode3->setGeometryBinding(geometry);
        ramses::MeshNode* meshNode4 = m_scene.createMeshNode("target mesh node");
        addMeshNodeToDefaultRenderGroup(*meshNode4);
        meshNode4->setAppearance(appearance);
        meshNode4->setGeometryBinding(geometry);

        // create translation and scale node for mesh
        ramses::Node* rootNode = m_scene.createNode("group of triangles");
        ramses::Node* translateNode = m_scene.createNode();
        ramses::Node* scaleNode = m_scene.createNode();
        ramses::Node* circleNode = m_scene.createNode();
        ramses::Node* targetNode = m_scene.createNode();

        if (state == ANIMATION_POINT4)
        {
            rootNode->setTranslation(0.f, 0.f, -15.f);
        }
        else
        {
            rootNode->setTranslation(0.f, 0.f, 0.f);
        }

        translateNode->setTranslation(-1.5f, -0.5f, -5.f);
        circleNode->setTranslation(-3.f, -1.f, -5.f);
        targetNode->setTranslation(12.f, 0.f, -30.f);

        translateNode->setParent(*rootNode);
        scaleNode->setParent(*rootNode);
        circleNode->setParent(*rootNode);
        targetNode->setParent(*rootNode);

        meshNode->setParent(*translateNode);
        meshNode2->setParent(*scaleNode);
        meshNode3->setParent(*circleNode);
        meshNode4->setParent(*targetNode);

        // get handle to appearance input
        ramses::DataVector4f& colorData = *m_scene.createDataVector4f();
        ramses::UniformInput colorInput;
        effect->findUniformInput("color", colorInput);
        appearance.bindInput(colorInput, colorData);
        colorData.setValue(0.f, 0.f, 0.f, 1.f);

        // create animation system
        ramses::AnimationSystem* animationSystem = m_scene.createAnimationSystem(ramses::EAnimationSystemFlags_Default, "animation system");

        // create animation sequence
        ramses::AnimationSequence* animSequence = animationSystem->createAnimationSequence();

        // create animation spline for color animation
        ramses::SplineLinearVector4f* spline = animationSystem->createSplineLinearVector4f("spline");

        // define spline keys to describe color animation
        ramses::splineTimeStamp_t timeStamp = 0u;
        spline->setKey(timeStamp, 0.f, 0.f, 0.f, 1.f);
        timeStamp = 3000u;
        spline->setKey(timeStamp, 1.f, 0.f, 0.f, 1.f);
        timeStamp = 6000u;
        spline->setKey(timeStamp, 0.f, 1.f, 0.f, 1.f);
        timeStamp = 9000u;
        spline->setKey(timeStamp, 0.f, 0.f, 1.f, 1.f);
        timeStamp = 10000u;
        spline->setKey(timeStamp, 1.f, 1.f, 1.f, 1.f);

        // create animated property - what you want to animate
        const ramses::AnimatedProperty* animPropertyColor = animationSystem->createAnimatedProperty(colorData);

        // create animation - binds spline and animated property
        ramses::Animation* animationColor = animationSystem->createAnimation(*animPropertyColor, *spline, "test animation");

        animSequence->addAnimation(*animationColor);

        // translation animation
        ramses::SplineLinearVector3f* spline2 = animationSystem->createSplineLinearVector3f("spline2");
        timeStamp = 0u;
        spline2->setKey(timeStamp, -1.5f, -0.5f, -5.f);
        timeStamp = 3000u;
        spline2->setKey(timeStamp, -2.f, -0.5f, -10.f);
        timeStamp = 6000u;
        spline2->setKey(timeStamp, -1.f, 0.5f, -5.f);
        timeStamp = 9000u;
        spline2->setKey(timeStamp, -2.f, 0.5f, -10.f);
        timeStamp = 10000u;
        spline2->setKey(timeStamp, -1.5f, -0.5f, -5.f);
        const ramses::AnimatedProperty* animPropertyTranslation = animationSystem->createAnimatedProperty(*translateNode, ramses::EAnimatedProperty_Translation);
        ramses::Animation* animationTranslation = animationSystem->createAnimation(*animPropertyTranslation, *spline2, "test animation 2");
        ramses::AnimationSequence* animSequenceTranslation = animationSystem->createAnimationSequence();
        animSequenceTranslation->addAnimation(*animationTranslation);
        animSequenceTranslation->setAnimationLooping(*animationTranslation);
        animSequenceTranslation->setPlaybackSpeed(1.8f);

        // circle animation
        ramses::SplineBezierFloat* splineCircleX = animationSystem->createSplineBezierFloat("splineCircleX");
        ramses::SplineBezierFloat* splineCircleY = animationSystem->createSplineBezierFloat("splineCircleY");
        const float mag = 0.6f;
        splineCircleX->setKey(0u, -1.f, 0.f, 0.f, mag, 0.f);
        splineCircleX->setKey(2000u, 0.f, 0.f, -mag, 0.f, mag);
        splineCircleX->setKey(4000u, 1.f, -mag, 0.f, mag, 0.f);
        splineCircleX->setKey(6000u, 0.f, 0.f, mag, 0.f, -mag);
        splineCircleX->setKey(8000u, -1.f, -mag, 0.f, 0.f, 0.f);

        splineCircleY->setKey(0u, 0.f, 0.f, 0.f, 0.f, mag);
        splineCircleY->setKey(2000u, 1.f, -mag, 0.f, mag, 0.f);
        splineCircleY->setKey(4000u, 0.f, 0.f, mag, 0.f, -mag);
        splineCircleY->setKey(6000u, -1.f, -mag, 0.f, mag, 0.f);
        splineCircleY->setKey(8000u, 0.f, 0.f, -mag, 0.f, 0.f);
        const ramses::AnimatedProperty* animPropertyCircleX = animationSystem->createAnimatedProperty(*circleNode, ramses::EAnimatedProperty_Translation, ramses::EAnimatedPropertyComponent_X);
        const ramses::AnimatedProperty* animPropertyCircleY = animationSystem->createAnimatedProperty(*circleNode, ramses::EAnimatedProperty_Translation, ramses::EAnimatedPropertyComponent_Y);
        ramses::Animation* animationCircleX = animationSystem-> createAnimation(*animPropertyCircleX, *splineCircleX, "test animation circleX");
        ramses::Animation* animationCircleY = animationSystem-> createAnimation(*animPropertyCircleY, *splineCircleY, "test animation circleY");
        animSequence->addAnimation(*animationCircleX);
        animSequence->addAnimation(*animationCircleY);
        animSequence->setAnimationRelative(*animationCircleX);
        animSequence->setAnimationRelative(*animationCircleY);
        animSequence->setAnimationLooping(*animationCircleX);
        animSequence->setAnimationLooping(*animationCircleY);

        // animation sequence
        ramses::AnimationSequence& sequence = createSequence(m_scene, *animationSystem, appearance, geometry);
        sequence.startAt(StartTime);

        animSequence->startAt(StartTime);
        animSequenceTranslation->startAt(StartTime);

        if (state == ANIMATION_RUNNING)
        {
            m_scene.publish();
        }

        // run animations

        for (UInt64 currTime = 0u; currTime < StopTime[state]; currTime += 50u)
        {
            animationSystem->setTime(currTime);

            if (state == ANIMATION_RUNNING)
            {
                m_scene.flush();
                PlatformThread::Sleep(100u);
            }
        }
    }

    ramses::AnimationSequence& AnimatedTrianglesScene::createSequence(ramses::Scene& scene, ramses::AnimationSystem& animationSystem, ramses::Appearance& appearance, ramses::GeometryBinding& geometry)
    {
        ramses::AnimationSequence* sequence = animationSystem.createAnimationSequence();
        ramses::Animation* prevAnimation = 0;
        for (int i = 0; i < 6; ++i)
        {
            ramses::MeshNode* meshNode = scene.createMeshNode("seq mesh");
            addMeshNodeToDefaultRenderGroup(*meshNode);
            meshNode->setAppearance(appearance);
            meshNode->setGeometryBinding(geometry);
            ramses::Node* translateNode = scene.createNode();
            translateNode->setTranslation(6.f + i, -3.f, -30.f);
            meshNode->setParent(*translateNode);

            ramses::SplineLinearVector3f* spline = animationSystem.createSplineLinearVector3f("seq spline");
            ramses::splineTimeStamp_t timeStamp = 0u;
            spline->setKey(timeStamp, 6.f + i, -3.f, -30.f);
            timeStamp = 1000u;
            spline->setKey(timeStamp, 6.f + i, -5.f, -30.f);
            timeStamp = 2000u;
            spline->setKey(timeStamp, 12.f + i, -3.f - i, -30.f);
            timeStamp = 3000u;
            spline->setKey(timeStamp, 6.f + i, -3.f, -30.f);

            const ramses::AnimatedProperty* animProperty = animationSystem.createAnimatedProperty(*translateNode, ramses::EAnimatedProperty_Translation);
            ramses::Animation* animation = animationSystem.createAnimation(*animProperty, *spline, "seq animation");
#if 0
            animation.setLooping();
            sequence.addAnimation(animation);
#else
            if (prevAnimation)
            {
                sequence->addAnimation(*animation, sequence->getAnimationStopTimeInSequence(*prevAnimation));
            }
            else
            {
                sequence->addAnimation(*animation);
            }
            prevAnimation = animation;
#endif
        }

        return *sequence;
    }
}
