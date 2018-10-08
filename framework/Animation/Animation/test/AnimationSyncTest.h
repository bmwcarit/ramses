//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSYNCTEST_H
#define RAMSES_ANIMATIONSYNCTEST_H

#include "AnimationTestUtils.h"
#include "AnimationTestTypes.h"
#include "Animation/AnimationLogic.h"
#include "Animation/AnimationProcessing.h"
#include "Animation/AnimationProcessingFinished.h"

namespace ramses_internal
{
    struct EmulatedSystemData
    {
        EmulatedSystemData()
        {
            PlatformMemory::Set(&m_container, 0, sizeof(ContainerType));
        }

        ContainerType m_container;
        AnimationData m_animationData;
    };

    class EmulatedSystem
    {
    public:
        enum ESystem
        {
            ESystem_Client = 0,
            ESystem_SceneManager,
            ESystem_Renderer,

            ESystem_NumSystems
        };

        EmulatedSystem()
            : m_logic(m_data.m_animationData)
            , m_finishedProcessing(m_data.m_animationData)
            , m_processing(m_data.m_animationData)
        {
        }

        EmulatedSystemData m_data;
        AnimationLogic m_logic;
        AnimationProcessingFinished m_finishedProcessing;
        AnimationProcessing m_processing;
    };

    class AnimationSyncTest : public testing::Test
    {
    public:
        AnimationSyncTest()
            : m_lastSplineKeyIndex(InvalidSplineKeyIndex)
        {
            for (UInt i = 0u; i < NumAnimations; ++i)
            {
                m_animationInstanceHandle[i] = AnimationInstanceHandle::Invalid();
                m_animationHandle[i] = AnimationHandle::Invalid();
            }
        }

    protected:
        void syncAnimationSystemsTest(const Vector3& initialData1 = Vector3(0), const Vector3& initialData2 = Vector3(0), Animation::Flags flags = 0u);

        void init();
        void initSystem(EmulatedSystem& system);
        void initSystemData(EmulatedSystemData& data);
        SplineHandle initSpline(AnimationData& animationData);
        void initDataBinds(EmulatedSystemData &syncData, Animation::Flags flags);
        void initAnimationRanges(const AnimationTime startTime, const AnimationTime::Duration animDuration, const AnimationTime finishTime, AnimationData& animationData, Animation::Flags flags);
        void initContainers(const Vector3& initialData2, const Vector3& initialData1);

        void runPhase1AnimationsTillFinished(Animation::Flags flags);
        void runPhase2AnimationsDontFinish(Animation::Flags flags);
        void finishPhase2ForAllSystems(AnimationLogic &syncLogic, AnimationData& syncAnimationData);

        void pauseSomeAnimations(AnimationData &resourceData);
        void resumePausedAnimations(AnimationData &resourceData);
        void setNewTargetForSomeAnimations(AnimationData &resourceData);

        void testAllContainersEqual();
        void testContainersNotEqualToRendererContainer();
        void testNewContainerEqualToRendererContainer(EmulatedSystemData &syncData);

        static const UInt NumAnimations = 10u;
        static const AnimationTime::Duration AnimationDuration = 200u;
        static const AnimationTime::TimeStamp Phase1StartTime = 100u;
        static const AnimationTime::TimeStamp Phase1FinishTime = 500u;
        static const AnimationTime::TimeStamp Phase2StartTime = 600u;
        static const AnimationTime::TimeStamp Phase2FinishTime = 1100u;
        static const AnimationTime::TimeStamp PauseTime = 800u;
        static const AnimationTime::TimeStamp SyncTime = 900u;

        SplineVec3 m_spline;
        SplineKeyIndex m_lastSplineKeyIndex;
        SplineHandle m_splineHandle;
        AnimationInstanceHandle m_animationInstanceHandle[NumAnimations];
        AnimationHandle m_animationHandle[NumAnimations];

        EmulatedSystem m_systems[EmulatedSystem::ESystem_NumSystems];
    };
}

#endif
