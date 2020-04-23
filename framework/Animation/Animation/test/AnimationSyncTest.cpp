//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AnimationSyncTest.h"
#include "AnimationTestUtils.h"

namespace ramses_internal
{
    TEST_F(AnimationSyncTest, SyncAnimationSystems)
    {
        syncAnimationSystemsTest();
    }

    TEST_F(AnimationSyncTest, SyncAnimationSystemsRelative)
    {
        syncAnimationSystemsTest(Vector3(1, 2, 3), Vector3(-4, -5, -6), Animation::EAnimationFlags_Relative);
    }

    void AnimationSyncTest::syncAnimationSystemsTest(const Vector3& initialData1, const Vector3& initialData2, Animation::Flags flags)
    {
        init();

        initContainers(initialData2, initialData1);
        testAllContainersEqual();

        runPhase1AnimationsTillFinished(flags);
        testAllContainersEqual();

        runPhase2AnimationsDontFinish(flags);
        testContainersNotEqualToRendererContainer();

        // New system connects
        // Assuming all animation resources are transfered from scene manager
        // Assuming all animated data (containers) are transformed from scene manager
        // Using scene manager data for new connected system
        EmulatedSystemData& syncData = m_systems[EmulatedSystem::ESystem_SceneManager].m_data;

        AnimationLogic syncLogic(syncData.m_animationData);
        AnimationProcessing syncProcessing(syncData.m_animationData);
        syncLogic.addListener(&syncProcessing);
        syncLogic.setTime(SyncTime);

        testNewContainerEqualToRendererContainer(syncData);

        finishPhase2ForAllSystems(syncLogic, syncData.m_animationData);
        testAllContainersEqual();
    }

    void AnimationSyncTest::init()
    {
        for (UInt i = 0u; i < EmulatedSystem::ESystem_NumSystems; ++i)
        {
            initSystem(m_systems[i]);
        }

        // Only renderer system uses full animation processing, the others are applying finished animations only
        m_systems[EmulatedSystem::ESystem_Renderer].m_logic.removeListener(&m_systems[EmulatedSystem::ESystem_Renderer].m_finishedProcessing);
        m_systems[EmulatedSystem::ESystem_Renderer].m_logic.addListener(&m_systems[EmulatedSystem::ESystem_Renderer].m_processing);
    }

    void AnimationSyncTest::initSystem(EmulatedSystem& system)
    {
        initSystemData(system.m_data);

        for (UInt i = 0u; i < NumAnimations; ++i)
        {
            m_animationHandle[i] = system.m_data.m_animationData.allocateAnimation(m_animationInstanceHandle[i]);
        }

        system.m_logic.addListener(&system.m_finishedProcessing);
    }

    void AnimationSyncTest::initSystemData(EmulatedSystemData& data)
    {
        AnimationData& animationData = data.m_animationData;
        ContainerType& container = data.m_container;

        m_splineHandle = initSpline(animationData);

        for (UInt32 i = 0u; i < NumAnimations; ++i)
        {
            DataBindType dataBind1(container, i, EDataBindAccessorType_Handles_1);
            DataBindType2Handles dataBind2(container, i, i, EDataBindAccessorType_Handles_2);
            const DataBindHandle dataBindHandle1 = animationData.allocateDataBinding(dataBind1);
            const DataBindHandle dataBindHandle2 = animationData.allocateDataBinding(dataBind2);

            m_animationInstanceHandle[i] = animationData.allocateAnimationInstance(m_splineHandle, EInterpolationType_Linear);
            animationData.addDataBindingToAnimationInstance(m_animationInstanceHandle[i], dataBindHandle1);
            animationData.addDataBindingToAnimationInstance(m_animationInstanceHandle[i], dataBindHandle2);
        }
    }

    SplineHandle AnimationSyncTest::initSpline(AnimationData& animationData)
    {
        static const UInt NumKeys = 10u;
        static const SplineTimeStamp KeyTimeStep = 10u;

        SplineTimeStamp keyTime = 10u;
        for (SplineKeyIndex keyIdx = 0u; keyIdx < NumKeys; ++keyIdx, ++keyTime)
        {
            const Vector3 val = Vector3(Float(keyIdx));
            const Vector2 tangent = Vector2(val.x, val.x);
            SplineKeyVec3 key(val, tangent, tangent);
            m_spline.setKey(keyTime, key);
            keyTime += KeyTimeStep;
        }

        m_lastSplineKeyIndex = m_spline.getNumKeys() - 1u;

        return animationData.allocateSpline(m_spline);
    }

    void AnimationSyncTest::initContainers(const Vector3& initialData2, const Vector3& initialData1)
    {
        for (UInt i = 0u; i < EmulatedSystem::ESystem_NumSystems; ++i)
        {
            for (MemoryHandle j = 0u; j < m_systems[i].m_data.m_container.NumHandles; ++j)
            {
                m_systems[i].m_data.m_container.setVal2(j, j, initialData2);
            }
            m_systems[i].m_data.m_container.setVal1(0u, initialData1);
        }
    }

    void AnimationSyncTest::initAnimationRanges(const AnimationTime startTime, const AnimationTime::Duration animDuration, const AnimationTime finishTime, AnimationData& animationData, Animation::Flags flags)
    {
        const AnimationTime stopTime = startTime + animDuration;
        const AnimationTime::Duration timeOffset = finishTime.getDurationSince(startTime + animDuration) / (NumAnimations - 1u);
        for (UInt a = 0u; a < NumAnimations; ++a)
        {
            const Animation& animation = animationData.getAnimation(m_animationHandle[a]);
            animationData.setAnimationProperties(m_animationHandle[a], animation.m_playbackSpeed, flags, animation.m_loopDuration);
            animationData.setAnimationTimeRange(m_animationHandle[a], startTime + a*timeOffset, stopTime + a*timeOffset);
        }
    }

    void AnimationSyncTest::runPhase1AnimationsTillFinished(Animation::Flags flags)
    {
        for (UInt i = 0u; i < EmulatedSystem::ESystem_NumSystems; ++i)
        {
            AnimationLogic& logic = m_systems[i].m_logic;
            AnimationData& animationData = m_systems[i].m_data.m_animationData;
            initAnimationRanges(Phase1StartTime, AnimationDuration, Phase1FinishTime, animationData, flags);

            logic.setTime(Phase1StartTime);
            logic.setTime(Phase1StartTime + AnimationDuration);
            logic.setTime(Phase1FinishTime);
        }
    }

    void AnimationSyncTest::runPhase2AnimationsDontFinish(Animation::Flags flags)
    {
        for (UInt i = 0u; i < EmulatedSystem::ESystem_NumSystems; ++i)
        {
            AnimationLogic& logic = m_systems[i].m_logic;
            AnimationData& animationData = m_systems[i].m_data.m_animationData;
            initAnimationRanges(Phase2StartTime, AnimationDuration, Phase2FinishTime, animationData, flags);

            logic.setTime(Phase2StartTime);
            logic.setTime(PauseTime);
            pauseSomeAnimations(animationData);
            setNewTargetForSomeAnimations(animationData);

            logic.setTime(SyncTime);
        }
    }

    void AnimationSyncTest::finishPhase2ForAllSystems(AnimationLogic &syncLogic, AnimationData& syncAnimationData)
    {
        // Update all systems except for scene manager
        for (UInt i = 0u; i < EmulatedSystem::ESystem_NumSystems; ++i)
        {
            if (i != EmulatedSystem::ESystem_SceneManager)
            {
                AnimationLogic& logic = m_systems[i].m_logic;
                AnimationData& animationData = m_systems[i].m_data.m_animationData;
                resumePausedAnimations(animationData);
                logic.setTime(Phase2FinishTime);
            }
        }
        // Update new system using scene manager's resources and container
        resumePausedAnimations(syncAnimationData);
        syncLogic.setTime(Phase2FinishTime);
    }

    void AnimationSyncTest::pauseSomeAnimations(AnimationData &resourceData)
    {
        for (UInt a = 0u; a < NumAnimations; ++a)
        {
            if (a % 2)
            {
                resourceData.setAnimationPaused(m_animationHandle[a], true);
            }
        }
    }

    void AnimationSyncTest::resumePausedAnimations(AnimationData &resourceData)
    {
        for (UInt a = 0u; a < NumAnimations; ++a)
        {
            resourceData.setAnimationPaused(m_animationHandle[a], false);
        }
    }

    void AnimationSyncTest::setNewTargetForSomeAnimations(AnimationData &resourceData)
    {
        for (UInt a = 0u; a < NumAnimations; ++a)
        {
            if (a % 3)
            {
                SplineIterator iter;
                iter.setTimeStamp(PauseTime, &m_spline);
                SplineKeyTangents<Vector3> key(Vector3(999.f), Vector2(-111.f), Vector2(0.f));
                resourceData.setSplineKey(m_splineHandle, iter.getSegment().m_endTimeStamp, key);
            }
        }
    }

    void AnimationSyncTest::testAllContainersEqual()
    {
        for (UInt i = 0u; i < EmulatedSystem::ESystem_NumSystems - 1u; ++i)
        {
            EXPECT_EQ(0, PlatformMemory::Compare(&m_systems[i].m_data.m_container, &m_systems[i + 1u].m_data.m_container, sizeof(ContainerType)));
        }
    }

    void AnimationSyncTest::testContainersNotEqualToRendererContainer()
    {
        for (UInt i = 0u; i < EmulatedSystem::ESystem_NumSystems; ++i)
        {
            if (i != EmulatedSystem::ESystem_Renderer)
            {
                EXPECT_NE(0, PlatformMemory::Compare(&m_systems[i].m_data.m_container, &m_systems[EmulatedSystem::ESystem_Renderer].m_data.m_container, sizeof(ContainerType)));
            }
        }
    }

    void AnimationSyncTest::testNewContainerEqualToRendererContainer(EmulatedSystemData &syncData)
    {
        EXPECT_EQ(0, PlatformMemory::Compare(&m_systems[EmulatedSystem::ESystem_Renderer].m_data.m_container, &syncData.m_container, sizeof(ContainerType)));
    }
}
