//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "Animation/AnimationSystem.h"
#include "Scene/Scene.h"
#include "Scene/SceneDataBinding.h"
#include "Animation/AnimationLogicListener.h"

using namespace testing;

namespace ramses_internal
{
    // Most animation system functionality is tested via AnimationDataTest,
    // because in most cases AnimationSystem only forwards calls to it

    class MockAnimationStartStopListener : public AnimationLogicListener
    {
    public:
        MOCK_METHOD1(onAnimationStarted, void(AnimationHandle handle));
        MOCK_METHOD1(onAnimationFinished, void(AnimationHandle handle));
    };

    class AnimationSystemTest : public testing::Test
    {
    public:
        AnimationSystemTest()
            : m_scene(*new Scene())
            , m_animationSystem(*new AnimationSystem(EAnimationSystemFlags_Default, AnimationSystemSizeInformation()))
        {
            m_scene.addAnimationSystem(&m_animationSystem);
        }

        virtual ~AnimationSystemTest()
        {
            delete &m_scene;
        }

    protected:
        AnimationHandle createAnimation()
        {
            const NodeHandle nodeHandle1 = m_scene.allocateNode();
            const TransformHandle transHandle1 = m_scene.allocateTransform(nodeHandle1);

            const SplineHandle splineHandle1 = m_animationSystem.allocateSpline(ESplineKeyType_Basic, EDataTypeID_Vector3f);
            m_animationSystem.setSplineKeyBasicVector3f(splineHandle1, 0u, Vector3(111.f, -999.f, 66.f));
            m_animationSystem.setSplineKeyBasicVector3f(splineHandle1, 10u, Vector3(11.f, -99.f, 666.f));

            typedef DataBindContainerToTraitsSelector<IScene>::ContainerTraitsClassType ContainerTraitsClass;
            const DataBindHandle dataBindHandle1 = m_animationSystem.allocateDataBinding(m_scene, ContainerTraitsClass::TransformNode_Rotation, transHandle1.asMemoryHandle(), InvalidMemoryHandle);

            const AnimationInstanceHandle animInstHandle1 = m_animationSystem.allocateAnimationInstance(splineHandle1, EInterpolationType_Linear, EVectorComponent_All);
            m_animationSystem.addDataBindingToAnimationInstance(animInstHandle1, dataBindHandle1);

            return m_animationSystem.allocateAnimation(animInstHandle1);
        }

        IScene&                                    m_scene;
        IAnimationSystem&                          m_animationSystem;
        StrictMock<MockAnimationStartStopListener> m_listener;
    };

    TEST_F(AnimationSystemTest, registeredLogicListenerReceivesStartStopCalls)
    {
        const AnimationHandle animHandle = createAnimation();

        m_animationSystem.setAnimationStartTime(animHandle, 10u);
        m_animationSystem.setAnimationStopTime(animHandle, 20u);

        m_animationSystem.registerAnimationLogicListener(&m_listener);

        EXPECT_CALL(m_listener, onAnimationStarted(animHandle));
        m_animationSystem.setTime(10u);

        EXPECT_CALL(m_listener, onAnimationFinished(animHandle));
        m_animationSystem.setTime(20u);

        m_animationSystem.unregisterAnimationLogicListener(&m_listener);
    }

    TEST_F(AnimationSystemTest, reportsActiveAnimation)
    {
        const AnimationHandle animHandle = createAnimation();
        m_animationSystem.setAnimationStartTime(animHandle, 10u);
        m_animationSystem.setAnimationStopTime(animHandle, 20u);

        EXPECT_FALSE(m_animationSystem.hasActiveAnimations());

        m_animationSystem.setTime(10u);
        EXPECT_TRUE(m_animationSystem.hasActiveAnimations());

        m_animationSystem.setTime(20u);
        EXPECT_FALSE(m_animationSystem.hasActiveAnimations());
    }
}
