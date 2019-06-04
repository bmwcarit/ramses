//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "TestEffectCreator.h"
#include "AnimationSystemImpl.h"
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/AnimationSystemRealTime.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/SplineLinearFloat.h"
#include "ramses-client-api/SplineLinearVector3f.h"
#include "ramses-client-api/UniformInput.h"
#include "ramses-client-api/DataVector3f.h"
#include "ramses-utils.h"

using namespace testing;

namespace ramses
{
    class AnimatedProperty;
    class Animation;

    class AnimationSystemTest : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    public:
        explicit AnimationSystemTest(uint32_t flags = EAnimationSystemFlags_Default)
            : LocalTestClientWithSceneAndAnimationSystem(flags)
        {
        }

    protected:
        AnimationSequence* createAnimationSequence(Animation** animation = NULL, Node** animatedNode = NULL)
        {
            SplineLinearVector3f* spline = animationSystem.createSplineLinearVector3f("spline");
            EXPECT_FALSE(spline == NULL);

            spline->setKey(0u, 0.0f, 0.0f, 0.0f);
            spline->setKey(5000u, 1.0f, 1.0f, 1.0f);

            Node* node = m_scene.createNode("node");
            EXPECT_FALSE(node == NULL);
            node->setTranslation(1.0f, 1.0f, 1.0f);

            AnimatedProperty* prop = animationSystem.createAnimatedProperty(*node, EAnimatedProperty_Translation, EAnimatedPropertyComponent_All);
            EXPECT_FALSE(prop == NULL);

            Animation* anim = animationSystem.createAnimation(*prop, *spline, "anim");
            EXPECT_FALSE(anim == NULL);

            AnimationSequence* sequence = animationSystem.createAnimationSequence("sequence");
            EXPECT_FALSE(sequence == NULL);

            sequence->addAnimation(*anim);

            if (animation != NULL)
            {
                *animation = anim;
            }
            if (animatedNode != NULL)
            {
                *animatedNode = node;
            }

            return sequence;
        }
    };

    class AnimationSystemTestClientSideProcessing : public AnimationSystemTest
    {
    public:
        AnimationSystemTestClientSideProcessing()
            : AnimationSystemTest(EAnimationSystemFlags_ClientSideProcessing)
        {
        }
    };

    // Valid creation cases are tested in ownership test and other global RamsesObject tests.

    TEST_F(AnimationSystemTest, initialTime)
    {
        EXPECT_EQ(0u, this->animationSystem.getTime());
    }

    TEST_F(AnimationSystemTest, setGetTime)
    {
        EXPECT_EQ(StatusOK, this->animationSystem.setTime(333u));
        EXPECT_EQ(333u, this->animationSystem.getTime());
    }

    TEST_F(AnimationSystemTest, realTimeAnimationSystemSetsLocalTime)
    {
        AnimationSystemRealTime& rtAnimSystem = this->createObject<AnimationSystemRealTime>("rt anim system");
        EXPECT_EQ(StatusOK, rtAnimSystem.updateLocalTime(333u));
        EXPECT_EQ(333u, rtAnimSystem.getTime());
    }

    TEST_F(AnimationSystemTest, createAnimationWithSplineFromDifferentAnimationSystemFails)
    {
        AnimationSystem* otherAnimationSystem = this->getScene().createAnimationSystem();
        SplineLinearVector3f* spline = otherAnimationSystem->createSplineLinearVector3f("spline");
        EXPECT_FALSE(spline == NULL);
        Node* node = this->m_scene.createNode("node");
        EXPECT_FALSE(node == NULL);
        AnimatedProperty* prop = this->animationSystem.createAnimatedProperty(*node, EAnimatedProperty_Translation);
        EXPECT_FALSE(prop == NULL);

        Animation* anim = this->animationSystem.createAnimation(*prop, *spline, "anim");
        EXPECT_TRUE(anim == NULL);
    }

    TEST_F(AnimationSystemTest, createAnimationWithAnimatedPropertyFromDifferentAnimationSystemFails)
    {
        AnimationSystem* otherAnimationSystem = this->getScene().createAnimationSystem();
        SplineLinearVector3f* spline = this->animationSystem.createSplineLinearVector3f("spline");
        EXPECT_FALSE(spline == NULL);
        Node* node = this->m_scene.createNode("node");
        EXPECT_FALSE(node == NULL);
        AnimatedProperty* prop = otherAnimationSystem->createAnimatedProperty(*node, EAnimatedProperty_Translation);
        EXPECT_FALSE(prop == NULL);

        Animation* anim = this->animationSystem.createAnimation(*prop, *spline, "anim");
        EXPECT_TRUE(anim == NULL);
    }

    TEST_F(AnimationSystemTest, createAnimatedSetterWithAnimatedPropertyFromDifferentAnimationSystemFails)
    {
        AnimationSystem* otherAnimationSystem = this->getScene().createAnimationSystem();
        Node* node = this->m_scene.createNode("node");
        EXPECT_FALSE(node == NULL);
        AnimatedProperty* prop = otherAnimationSystem->createAnimatedProperty(*node, EAnimatedProperty_Translation);
        EXPECT_FALSE(prop == NULL);

        AnimatedSetter* anim = this->animationSystem.createAnimatedSetter(*prop, "anim");
        EXPECT_TRUE(anim == NULL);
    }

    TEST_F(AnimationSystemTest, createAnimationWithSplineAndPropertyDataMismatchFails)
    {
        SplineLinearFloat* spline = this->animationSystem.createSplineLinearFloat("spline");
        EXPECT_FALSE(spline == NULL);
        Node* node = this->m_scene.createNode("node");
        EXPECT_FALSE(node == NULL);
        AnimatedProperty* prop = this->animationSystem.createAnimatedProperty(*node, EAnimatedProperty_Translation);
        EXPECT_FALSE(prop == NULL);

        Animation* anim = this->animationSystem.createAnimation(*prop, *spline, "anim");
        EXPECT_TRUE(anim == NULL);
    }

    TEST_F(AnimationSystemTest, createAnimationWithSplineAndPropertyComponentMismatchFails)
    {
        SplineLinearVector3f* spline = this->animationSystem.createSplineLinearVector3f("spline");
        EXPECT_FALSE(spline == NULL);
        Node* node = this->m_scene.createNode("node");
        EXPECT_FALSE(node == NULL);
        AnimatedProperty* prop = this->animationSystem.createAnimatedProperty(*node, EAnimatedProperty_Translation, EAnimatedPropertyComponent_Y);
        EXPECT_FALSE(prop == NULL);

        Animation* anim = this->animationSystem.createAnimation(*prop, *spline, "anim");
        EXPECT_TRUE(anim == NULL);
    }

    TEST_F(AnimationSystemTest, realTimeAnimationSystemCanBeConvertedToAnimationSystem)
    {
        RamsesObject& obj = this->createObject<AnimationSystemRealTime>("anim system");
        EXPECT_TRUE(RamsesUtils::TryConvert<AnimationSystem>(obj) != NULL);
        const RamsesObject& constObj = obj;
        EXPECT_TRUE(RamsesUtils::TryConvert<AnimationSystem>(constObj) != NULL);
    }

    TEST_F(AnimationSystemTest, retrieveAnimationFromFinishedAnimationsWhenAnimationFinished)
    {
        Animation* animation = NULL;
        AnimationSequence* sequence = createAnimationSequence(&animation);
        sequence->startAt(0u);

        EXPECT_EQ(0u, animationSystem.getNumberOfFinishedAnimationsSincePreviousUpdate());

        animationSystem.setTime(5000u);
        EXPECT_EQ(1u, animationSystem.getNumberOfFinishedAnimationsSincePreviousUpdate());
        EXPECT_EQ(animation, animationSystem.getFinishedAnimationSincePreviousUpdate(0u));
    }

    TEST_F(AnimationSystemTest, retrieveAnimationFromFinishedAnimationsWhenAnimationStopped)
    {
        Animation* animation = NULL;
        AnimationSequence* sequence = createAnimationSequence(&animation);
        sequence->startAt(0u);

        EXPECT_EQ(0u, animationSystem.getNumberOfFinishedAnimationsSincePreviousUpdate());

        animationSystem.setTime(1000u);
        sequence->stop();

        EXPECT_EQ(1u, animationSystem.getNumberOfFinishedAnimationsSincePreviousUpdate());
        EXPECT_EQ(animation, animationSystem.getFinishedAnimationSincePreviousUpdate(0u));
    }

    TEST_F(AnimationSystemTest, canValidate)
    {
        Animation* animation = NULL;
        createAnimationSequence(&animation);
        EXPECT_EQ(StatusOK, animationSystem.validate());
    }

    TEST_F(AnimationSystemTest, validationReportsContainsObjectCounts)
    {
        // 1 sequence
        createAnimationSequence();

        // 4 splines in total
        SplineLinearVector3f* spline2 = animationSystem.createSplineLinearVector3f("spline2");
        SplineLinearVector3f* spline3 = animationSystem.createSplineLinearVector3f("spline3");
        SplineLinearVector3f* spline4 = animationSystem.createSplineLinearVector3f("spline4");
        spline2->setKey(0u, 0.0f, 0.0f, 0.0f);
        spline3->setKey(0u, 0.0f, 0.0f, 0.0f);
        spline4->setKey(0u, 0.0f, 0.0f, 0.0f);

        // 3 animated properties in total
        Node* node2 = m_scene.createNode("node2");
        Node* node3 = m_scene.createNode("node3");
        AnimatedProperty* prop2 = animationSystem.createAnimatedProperty(*node2, EAnimatedProperty_Translation, EAnimatedPropertyComponent_All);
        animationSystem.createAnimatedProperty(*node3, EAnimatedProperty_Translation, EAnimatedPropertyComponent_All);

        // 2 animations in total
        animationSystem.createAnimation(*prop2, *spline2, "anim");

        EXPECT_EQ(StatusOK, animationSystem.validate());
        const ramses_internal::String validationReport = animationSystem.getValidationReport();
        EXPECT_TRUE(validationReport.find("Number of ERamsesObjectType_Animation instances: 2") >= 0);
        EXPECT_TRUE(validationReport.find("Number of ERamsesObjectType_AnimationSequence instances: 1") >= 0);
        EXPECT_TRUE(validationReport.find("Number of ERamsesObjectType_AnimatedProperty instances: 3") >= 0);
        EXPECT_TRUE(validationReport.find("Number of ERamsesObjectType_AnimatedSetter instances: 0") >= 0);
        EXPECT_TRUE(validationReport.find("Number of ERamsesObjectType_SplineLinearVector3f instances: 4") >= 0);
    }

    TEST_F(AnimationSystemTest, failsToValidateIfOneOfItsObjectsNotValidated)
    {
        Animation* animation = NULL;
        Node* node = NULL;
        createAnimationSequence(&animation, &node);
        this->animationSystem.destroy(*animation);
        this->getScene().destroy(*node);

        EXPECT_NE(StatusOK, animationSystem.validate());

        const ramses_internal::String validationReport = animationSystem.getValidationReport();
        EXPECT_TRUE(validationReport.find("Number of ERamsesObjectType_Animation instances: 0") >= 0);
        EXPECT_TRUE(validationReport.find("Number of ERamsesObjectType_AnimationSequence instances: 1") >= 0);
        EXPECT_TRUE(validationReport.find("Number of ERamsesObjectType_AnimatedProperty instances: 1") >= 0);
        EXPECT_TRUE(validationReport.find("Number of ERamsesObjectType_AnimatedSetter instances: 0") >= 0);
        EXPECT_TRUE(validationReport.find("Number of ERamsesObjectType_SplineLinearVector3f instances: 1") >= 0);
    }

    TEST_F(AnimationSystemTestClientSideProcessing, createAnimationSystemWithClientSideProcessingAndCheckInterpolationResults)
    {
        Node* node = NULL;
        AnimationSequence* sequence = createAnimationSequence(NULL, &node);
        sequence->startAt(0u);

        this->animationSystem.setTime(2500u);

        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
        node->getTranslation(x, y, z);

        const float EPSILON = 1e-6f;
        EXPECT_NEAR(x, 0.5f, EPSILON);
        EXPECT_NEAR(y, 0.5f, EPSILON);
        EXPECT_NEAR(z, 0.5f, EPSILON);
    }

    TEST_F(AnimationSystemTestClientSideProcessing, canAnimatePropertyTranslateNode)
    {
        Node& animatable = this->createObject<Node>("animatable");
        AnimatedProperty* prop = this->animationSystem.createAnimatedProperty(animatable, EAnimatedProperty_Translation, EAnimatedPropertyComponent_All);
        ASSERT_TRUE(prop != NULL);

        SplineLinearVector3f* spline = this->animationSystem.createSplineLinearVector3f();
        ASSERT_TRUE(spline != NULL);
        const ramses_internal::Vector3 startValue(0.f);
        const ramses_internal::Vector3 endValue(1.f);
        EXPECT_EQ(StatusOK, spline->setKey(0u, startValue.x, startValue.y, startValue.z));
        EXPECT_EQ(StatusOK, spline->setKey(100u, endValue.x, endValue.y, endValue.z));

        Animation* animation = this->animationSystem.createAnimation(*prop, *spline);
        ASSERT_TRUE(animation != NULL);

        AnimationSequence* animSequence = this->animationSystem.createAnimationSequence();
        ASSERT_TRUE(animSequence != NULL);
        EXPECT_EQ(StatusOK, animSequence->addAnimation(*animation));
        EXPECT_EQ(StatusOK, animSequence->startAt(0u));

        ramses_internal::Vector3 value(startValue);
        EXPECT_EQ(StatusOK, animatable.setTranslation(value.x, value.y, value.z));

        EXPECT_EQ(StatusOK, this->animationSystem.setTime(50u));
        EXPECT_EQ(StatusOK, animatable.getTranslation(value.x, value.y, value.z));
        EXPECT_NE(startValue, value);

        EXPECT_EQ(StatusOK, this->animationSystem.setTime(150u));
        EXPECT_EQ(StatusOK, animatable.getTranslation(value.x, value.y, value.z));
        EXPECT_EQ(endValue, value);
    }

    TEST_F(AnimationSystemTestClientSideProcessing, canAnimatePropertyRotateNode)
    {
        Node& animatable = this->createObject<Node>("animatable");
        AnimatedProperty* prop = this->animationSystem.createAnimatedProperty(animatable, EAnimatedProperty_Rotation, EAnimatedPropertyComponent_All);
        ASSERT_TRUE(prop != NULL);

        SplineLinearVector3f* spline = this->animationSystem.createSplineLinearVector3f();
        ASSERT_TRUE(spline != NULL);
        const ramses_internal::Vector3 startValue(0.f);
        const ramses_internal::Vector3 endValue(1.f);
        EXPECT_EQ(StatusOK, spline->setKey(0u, startValue.x, startValue.y, startValue.z));
        EXPECT_EQ(StatusOK, spline->setKey(100u, endValue.x, endValue.y, endValue.z));

        Animation* animation = this->animationSystem.createAnimation(*prop, *spline);
        ASSERT_TRUE(animation != NULL);

        AnimationSequence* animSequence = this->animationSystem.createAnimationSequence();
        ASSERT_TRUE(animSequence != NULL);
        EXPECT_EQ(StatusOK, animSequence->addAnimation(*animation));
        EXPECT_EQ(StatusOK, animSequence->startAt(0u));

        ramses_internal::Vector3 value(startValue);
        EXPECT_EQ(StatusOK, animatable.setRotation(value.x, value.y, value.z));

        EXPECT_EQ(StatusOK, this->animationSystem.setTime(50u));
        EXPECT_EQ(StatusOK, animatable.getRotation(value.x, value.y, value.z));
        EXPECT_NE(startValue, value);

        EXPECT_EQ(StatusOK, this->animationSystem.setTime(150u));
        EXPECT_EQ(StatusOK, animatable.getRotation(value.x, value.y, value.z));
        EXPECT_EQ(endValue, value);
    }

    TEST_F(AnimationSystemTestClientSideProcessing, canAnimatePropertyScaleNode)
    {
        Node& animatable = this->createObject<Node>("animatable");
        AnimatedProperty* prop = this->animationSystem.createAnimatedProperty(animatable, EAnimatedProperty_Scaling, EAnimatedPropertyComponent_All);
        ASSERT_TRUE(prop != NULL);

        SplineLinearVector3f* spline = this->animationSystem.createSplineLinearVector3f();
        ASSERT_TRUE(spline != NULL);
        const ramses_internal::Vector3 startValue(0.f);
        const ramses_internal::Vector3 endValue(1.f);
        EXPECT_EQ(StatusOK, spline->setKey(0u, startValue.x, startValue.y, startValue.z));
        EXPECT_EQ(StatusOK, spline->setKey(100u, endValue.x, endValue.y, endValue.z));

        Animation* animation = this->animationSystem.createAnimation(*prop, *spline);
        ASSERT_TRUE(animation != NULL);

        AnimationSequence* animSequence = this->animationSystem.createAnimationSequence();
        ASSERT_TRUE(animSequence != NULL);
        EXPECT_EQ(StatusOK, animSequence->addAnimation(*animation));
        EXPECT_EQ(StatusOK, animSequence->startAt(0u));

        ramses_internal::Vector3 value(startValue);
        EXPECT_EQ(StatusOK, animatable.setScaling(value.x, value.y, value.z));

        EXPECT_EQ(StatusOK, this->animationSystem.setTime(50u));
        EXPECT_EQ(StatusOK, animatable.getScaling(value.x, value.y, value.z));
        EXPECT_NE(startValue, value);

        EXPECT_EQ(StatusOK, this->animationSystem.setTime(150u));
        EXPECT_EQ(StatusOK, animatable.getScaling(value.x, value.y, value.z));
        EXPECT_EQ(endValue, value);
    }

    TEST_F(AnimationSystemTestClientSideProcessing, canAnimatePropertyUniformInput)
    {
        Effect* effect = TestEffectCreator::createEffect(this->client, false);
        ASSERT_TRUE(effect != NULL);
        UniformInput uniformInput;
        EXPECT_EQ(StatusOK, effect->findUniformInput("vec3fInput", uniformInput));

        Appearance* appearance = this->m_scene.createAppearance(*effect);
        ASSERT_TRUE(appearance != NULL);

        AnimatedProperty* prop = this->animationSystem.createAnimatedProperty(uniformInput, *appearance, EAnimatedPropertyComponent_All);
        ASSERT_TRUE(prop != NULL);

        SplineLinearVector3f* spline = this->animationSystem.createSplineLinearVector3f();
        ASSERT_TRUE(spline != NULL);
        const ramses_internal::Vector3 startValue(0.f);
        const ramses_internal::Vector3 endValue(1.f);
        EXPECT_EQ(StatusOK, spline->setKey(0u, startValue.x, startValue.y, startValue.z));
        EXPECT_EQ(StatusOK, spline->setKey(100u, endValue.x, endValue.y, endValue.z));

        Animation* animation = this->animationSystem.createAnimation(*prop, *spline);
        ASSERT_TRUE(animation != NULL);

        AnimationSequence* animSequence = this->animationSystem.createAnimationSequence();
        ASSERT_TRUE(animSequence != NULL);
        EXPECT_EQ(StatusOK, animSequence->addAnimation(*animation));
        EXPECT_EQ(StatusOK, animSequence->startAt(0u));

        ramses_internal::Vector3 value(startValue);
        EXPECT_EQ(StatusOK, appearance->setInputValueVector3f(uniformInput, value.x, value.y, value.z));

        EXPECT_EQ(StatusOK, this->animationSystem.setTime(50u));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector3f(uniformInput, value.x, value.y, value.z));
        EXPECT_NE(startValue, value);

        EXPECT_EQ(StatusOK, this->animationSystem.setTime(150u));
        EXPECT_EQ(StatusOK, appearance->getInputValueVector3f(uniformInput, value.x, value.y, value.z));
        EXPECT_EQ(endValue, value);
    }

    TEST_F(AnimationSystemTestClientSideProcessing, canAnimatePropertyDataObject)
    {
        DataVector3f& animatable = this->createObject<DataVector3f>("animatable");

        AnimatedProperty* prop = this->animationSystem.createAnimatedProperty(animatable, EAnimatedPropertyComponent_All);
        ASSERT_TRUE(prop != NULL);

        SplineLinearVector3f* spline = this->animationSystem.createSplineLinearVector3f();
        ASSERT_TRUE(spline != NULL);
        const ramses_internal::Vector3 startValue(0.f);
        const ramses_internal::Vector3 endValue(1.f);
        EXPECT_EQ(StatusOK, spline->setKey(0u, startValue.x, startValue.y, startValue.z));
        EXPECT_EQ(StatusOK, spline->setKey(100u, endValue.x, endValue.y, endValue.z));

        Animation* animation = this->animationSystem.createAnimation(*prop, *spline);
        ASSERT_TRUE(animation != NULL);

        AnimationSequence* animSequence = this->animationSystem.createAnimationSequence();
        ASSERT_TRUE(animSequence != NULL);
        EXPECT_EQ(StatusOK, animSequence->addAnimation(*animation));
        EXPECT_EQ(StatusOK, animSequence->startAt(0u));

        ramses_internal::Vector3 value(startValue);
        EXPECT_EQ(StatusOK, animatable.setValue(value.x, value.y, value.z));

        EXPECT_EQ(StatusOK, this->animationSystem.setTime(50u));
        EXPECT_EQ(StatusOK, animatable.getValue(value.x, value.y, value.z));
        EXPECT_NE(startValue, value);

        EXPECT_EQ(StatusOK, this->animationSystem.setTime(150u));
        EXPECT_EQ(StatusOK, animatable.getValue(value.x, value.y, value.z));
        EXPECT_EQ(endValue, value);
    }
}
