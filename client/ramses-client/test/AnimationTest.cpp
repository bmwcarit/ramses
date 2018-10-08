//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "ramses-client-api/TranslateNode.h"
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/SplineLinearFloat.h"
#include "ramses-client-api/AnimatedProperty.h"
#include "ramses-client-api/DataFloat.h"
#include "AnimationSystemImpl.h"
#include "AnimationImpl.h"

#include "Animation/AnimationTime.h"

using namespace testing;

namespace ramses
{
    class AnimationTest : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    public:
        virtual void SetUp()
        {
            m_animatedNode = this->m_scene.createTranslateNode();

            m_spline = this->animationSystem.createSplineLinearFloat("spline");
            m_animProperty = this->animationSystem.createAnimatedProperty(*m_animatedNode, EAnimatedPropertyComponent_X, "prop");
            m_animation = this->animationSystem.createAnimation(*m_animProperty, *m_spline, "animation");
        }

    protected:
        void setUpSpline()
        {
            m_spline->setKey(0u, 1.f);
            m_spline->setKey(2000, 0.f);
        }

        TranslateNode* m_animatedNode;
        SplineLinearFloat* m_spline;
        AnimatedProperty* m_animProperty;
        Animation* m_animation;
    };

    TEST_F(AnimationTest, initialValues)
    {
        const globalTimeStamp_t invalidTime = ramses_internal::AnimationTime::InvalidTimeStamp;
        EXPECT_EQ(invalidTime, m_animation->getStartTime());
        EXPECT_EQ(invalidTime, m_animation->getStopTime());
        EXPECT_NE(ramses_internal::AnimationHandle::Invalid(), m_animation->impl.getAnimationHandle());
    }

    TEST_F(AnimationTest, canValidate)
    {
        setUpSpline();
        EXPECT_EQ(StatusOK, m_animation->validate());
    }

    TEST_F(AnimationTest, failsToValidateIfAnimatedPropertyDestroyed)
    {
        setUpSpline();
        this->animationSystem.destroy(*m_animProperty);
        EXPECT_NE(StatusOK, m_animation->validate());
    }

    TEST_F(AnimationTest, failsToValidateIfAnimatedNodeDestroyed)
    {
        setUpSpline();
        this->m_scene.destroy(*m_animatedNode);
        EXPECT_NE(StatusOK, m_animation->validate());
    }

    TEST_F(AnimationTest, failsToValidateIfSplineDestroyed)
    {
        setUpSpline();
        this->animationSystem.destroy(*m_spline);
        EXPECT_NE(StatusOK, m_animation->validate());
    }

    TEST_F(AnimationTest, failsToValidateIfSplineEmpty)
    {
        EXPECT_NE(StatusOK, m_animation->validate());
    }

    TEST_F(AnimationTest, validatesWithAnimatedDataObject)
    {
        DataFloat& animatable = this->createObject<DataFloat>();
        setUpSpline();

        AnimatedProperty* animProperty = this->animationSystem.createAnimatedProperty(animatable, EAnimatedPropertyComponent_X, "prop");
        Animation* animation = this->animationSystem.createAnimation(*animProperty, *m_spline, "animation");
        EXPECT_EQ(StatusOK, animation->validate());
    }

    TEST_F(AnimationTest, failsToValidateIfAnimatedDataObjectDestroyed)
    {
        DataFloat& animatable = this->createObject<DataFloat>();
        setUpSpline();

        AnimatedProperty* animProperty = this->animationSystem.createAnimatedProperty(animatable, EAnimatedPropertyComponent_X, "prop");
        Animation* animation = this->animationSystem.createAnimation(*animProperty, *m_spline, "animation");

        this->getScene().destroy(animatable);

        EXPECT_NE(StatusOK, animation->validate());
    }
}
