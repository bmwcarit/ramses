//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AnimationSystemSyncTest.h"
#include "Scene/SceneDataBinding.h"
#include "AnimationTestUtils.h"

namespace ramses_internal
{
    INSTANTIATE_TEST_SUITE_P(
        TestSyncAtDifferentTimeStamps,
        AnimationSystemSyncTest,
        testing::Values(
            TestParam( 2000u, false ),
            TestParam( 5000u, true ),
            TestParam( 7000u, true ),
            TestParam( 9000u, true ),
            TestParam( 10000u, true ),
            TestParam( 50000u, true ),
            TestParam( 99000u, true ),
            TestParam( 100000u, false ),
            TestParam( 101000u, false ) ));

    TEST_P(AnimationSystemSyncTest, TestWithSyncedTime)
    {
        initScenes();
        initAnimationSystems();
        syncAnimationSystems(0u);
        expectScenesEqual();
        syncAnimationSystems(GetParam().m_timeStamp);

        if (GetParam().m_testOnlyFullProcessingSystems)
        {
            expectScenesWithFullProcessingEqual();
        }
        else
        {
            expectScenesEqual();
        }
    }

    TEST_P(AnimationSystemSyncTest, TestWithOutOfSyncUpdates)
    {
        initScenes();
        initAnimationSystems();
        syncAnimationSystems(0u);

        for (UInt32 i = 0u; i < NumSystems; ++i)
        {
            for (AnimationTime::TimeStamp timeStamp = 50u; timeStamp < GetParam().m_timeStamp.getTimeStamp(); timeStamp += 50u + 10u * i)
            {
                m_animationSystems[i]->setTime(timeStamp);
            }
        }

        syncAnimationSystems(GetParam().m_timeStamp);

        if (GetParam().m_testOnlyFullProcessingSystems)
        {
            expectScenesWithFullProcessingEqual();
        }
        else
        {
            expectScenesEqual();
        }
    }

    TEST_P(AnimationSystemSyncTest, TestWithOutOfSyncUpdatesWithChanges)
    {
        initScenes();
        initAnimationSystems();
        syncAnimationSystems(0u);

        const AnimationTime::TimeStamp syncTime = GetParam().m_timeStamp.getTimeStamp() / 2u;

        for (UInt32 i = 0u; i < NumSystems; ++i)
        {
            for (AnimationTime::TimeStamp timeStamp = 50u; timeStamp < syncTime; timeStamp += 50u + 10u * i)
            {
                m_animationSystems[i]->setTime(timeStamp);
            }
        }

        // apply change to all systems with time stamp of the first one
        const AnimationTime changeTimeStamp = m_animationSystems[0]->getTime();
        const Animation& anim = m_animationSystems[0]->getAnimation(m_animationHandle1);
        for (UInt32 i = 0u; i < NumSystems; ++i)
        {
            m_animationSystems[i]->setAnimationProperties(m_animationHandle1, 10.f, anim.m_flags, anim.m_loopDuration, changeTimeStamp);
            m_animationSystems[i]->stopAnimationAndRollback(m_animationHandle2);
        }

        // keep updating asynchronously till the testing end
        for (UInt32 i = 0u; i < NumSystems; ++i)
        {
            const AnimationTime::TimeStamp currTimeStamp = m_animationSystems[i]->getTime().getTimeStamp();
            for (AnimationTime::TimeStamp timeStamp = currTimeStamp; timeStamp < GetParam().m_timeStamp.getTimeStamp(); timeStamp += 50u + 10u * i)
            {
                m_animationSystems[i]->setTime(timeStamp);
            }
        }

        syncAnimationSystems(GetParam().m_timeStamp);

        if (GetParam().m_testOnlyFullProcessingSystems)
        {
            expectScenesWithFullProcessingEqual();
        }
        else
        {
            expectScenesEqual();
        }
    }

    void AnimationSystemSyncTest::initScenes()
    {
        for (UInt32 i = 0u; i < NumSystems; ++i)
        {
            IScene& scene = *m_scenes[i];

            const NodeHandle nodeHandle1 = scene.allocateNode();
            const NodeHandle nodeHandle2 = scene.allocateNode();
            m_transformHandle1 = scene.allocateTransform(nodeHandle1);
            m_transformHandle2 = scene.allocateTransform(nodeHandle2);

            scene.setRotation(m_transformHandle1, Vector3(1.0f, 1.0f, 1.0f));
            scene.setTranslation(m_transformHandle2, Vector3(1.0f, 1.0f, 1.0f));
        }
    }

    void AnimationSystemSyncTest::initAnimationSystems()
    {
        for (UInt32 i = 0u; i < NumSystems; ++i)
        {
            IScene& scene = *m_scenes[i];
            IAnimationSystem& animSystem = *m_animationSystems[i];

            const SplineHandle splineHandle1 = animSystem.allocateSpline(ESplineKeyType_Basic, EDataTypeID_Vector3f);
            const SplineHandle splineHandle2 = animSystem.allocateSpline(ESplineKeyType_Tangents, EDataTypeID_Float);
            animSystem.setSplineKeyBasicVector3f(splineHandle1, 10u, Vector3(111.f, -999.f, 66.f));
            animSystem.setSplineKeyBasicVector3f(splineHandle1, 5000u, Vector3(11.f, -99.f, 666.f));
            animSystem.setSplineKeyTangentsFloat(splineHandle2, 11u, 55.f, Vector2(111.f, -999.f), Vector2(11.f, -99.f));
            animSystem.setSplineKeyTangentsFloat(splineHandle2, 990u, -88.f, Vector2(11.f, -99.f), Vector2(111.f, -999.f));

            typedef DataBindContainerToTraitsSelector<IScene>::ContainerTraitsClassType ContainerTraitsClass;
            const DataBindHandle dataBindHandle1 = animSystem.allocateDataBinding(scene, ContainerTraitsClass::TransformNode_Rotation, m_transformHandle1.asMemoryHandle(), InvalidMemoryHandle);
            const DataBindHandle dataBindHandle2 = animSystem.allocateDataBinding(scene, ContainerTraitsClass::TransformNode_Translation, m_transformHandle2.asMemoryHandle(), InvalidMemoryHandle);

            const AnimationInstanceHandle animInstHandle1 = animSystem.allocateAnimationInstance(splineHandle1, EInterpolationType_Linear, EVectorComponent_All);
            const AnimationInstanceHandle animInstHandle2 = animSystem.allocateAnimationInstance(splineHandle2, EInterpolationType_Bezier, EVectorComponent_Y);
            animSystem.addDataBindingToAnimationInstance(animInstHandle1, dataBindHandle1);
            animSystem.addDataBindingToAnimationInstance(animInstHandle2, dataBindHandle2);

            m_animationHandle1 = animSystem.allocateAnimation(animInstHandle1);
            m_animationHandle2 = animSystem.allocateAnimation(animInstHandle2);
            animSystem.setAnimationProperties(m_animationHandle1, 2.f, Animation::EAnimationFlags_Relative, 0u, 0u);
            animSystem.setAnimationProperties(m_animationHandle2, 1.f, Animation::EAnimationFlags_Looping, 0u, 0u);
            animSystem.setAnimationStartTime(m_animationHandle1, 5000u);
            animSystem.setAnimationStartTime(m_animationHandle2, 9000u);
            animSystem.setAnimationStopTime(m_animationHandle1, 10000u);
            animSystem.setAnimationStopTime(m_animationHandle2, 100000u);
        }
    }

    void AnimationSystemSyncTest::syncAnimationSystems(const AnimationTime& timeStamp)
    {
        for (UInt32 i = 0u; i < NumSystems; ++i)
        {
            m_animationSystems[i]->setTime(timeStamp);
        }
    }

    void AnimationSystemSyncTest::expectScenesEqual()
    {
        const Vector3 nodeRot = m_scenes[0]->getRotation(m_transformHandle1);
        const Vector3 nodeTrans = m_scenes[0]->getTranslation(m_transformHandle2);

        for (UInt32 i = 1u; i < NumSystems; ++i)
        {
            const Vector3& nodeRotOther = m_scenes[i]->getRotation(m_transformHandle1);
            const Vector3& nodeTransOther = m_scenes[i]->getTranslation(m_transformHandle2);
            EXPECT_TRUE(AnimationTestUtils::AreEqual(nodeRot, nodeRotOther));
            EXPECT_TRUE(AnimationTestUtils::AreEqual(nodeTrans, nodeTransOther));
        }
    }

    void AnimationSystemSyncTest::expectScenesWithFullProcessingEqual()
    {
        // find first fully processed system
        UInt32 i = 0u;
        for (; i < NumSystems; ++i)
        {
            if ((m_animationSystems[i]->getFlags() & EAnimationSystemFlags_FullProcessing) != 0u)
            {
                break;
            }
        }

        if (i < NumSystems)
        {
            const Vector3 nodeRot = m_scenes[i]->getRotation(m_transformHandle1);
            const Vector3 nodeTrans = m_scenes[i]->getTranslation(m_transformHandle2);

            for (++i; i < NumSystems; ++i)
            {
                // skip system without full processing
                if ((m_animationSystems[i]->getFlags() & EAnimationSystemFlags_FullProcessing) == 0u)
                {
                    continue;
                }

                const Vector3& nodeRotOther = m_scenes[i]->getRotation(m_transformHandle1);
                const Vector3& nodeTransOther = m_scenes[i]->getTranslation(m_transformHandle2);
                EXPECT_TRUE(AnimationTestUtils::AreEqual(nodeRot, nodeRotOther));
                EXPECT_TRUE(AnimationTestUtils::AreEqual(nodeTrans, nodeTransOther));
            }
        }
    }
}
