//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONSYSTEMSYNCTEST_H
#define RAMSES_ANIMATIONSYSTEMSYNCTEST_H

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "Scene/Scene.h"
#include "Animation/AnimationSystem.h"

namespace ramses_internal
{
    struct TestParam
    {
        TestParam(const AnimationTime timeStamp = 0u, bool testOnlyFullProcessingSystems = false)
            : m_timeStamp(timeStamp)
            , m_testOnlyFullProcessingSystems(testOnlyFullProcessingSystems)
        {
        }

        AnimationTime m_timeStamp;
        bool          m_testOnlyFullProcessingSystems;
    };

    // This is a custom printer for gtest.
    // It is needed to prevent valgrind use of unitialized memory warnings
    // when gtest tries to print TestParam as binary blob. TestParam has
    // 'holes' due to alignment that may never be accessed
    inline void PrintTo(const TestParam& param, ::std::ostream* os)
    {
        *os << param.m_timeStamp.getTimeStamp() << "_" << param.m_testOnlyFullProcessingSystems;
    }

    class AnimationSystemSyncTest : public testing::TestWithParam<TestParam>
    {
    public:
        AnimationSystemSyncTest()
        {
            for (UInt32 i = 0u; i < NumSystems; ++i)
            {
                m_scenes[i] = new Scene();
                m_animationSystems[i] = new AnimationSystem((i % 2) ? UInt32(EAnimationSystemFlags_FullProcessing) : 0u, AnimationSystemSizeInformation());
                m_scenes[i]->addAnimationSystem(m_animationSystems[i]);
            }
        }

        virtual ~AnimationSystemSyncTest()
        {
            for (UInt32 i = 0u; i < NumSystems; ++i)
            {
                delete m_scenes[i];
            }
        }

    protected:
        void initScenes();
        void initAnimationSystems();
        void syncAnimationSystems(const AnimationTime& timeStamp);
        void expectScenesEqual();
        void expectScenesWithFullProcessingEqual();

        static const UInt32 NumSystems = 4u;

        IScene* m_scenes[NumSystems];
        IAnimationSystem* m_animationSystems[NumSystems];

        TransformHandle m_transformHandle1;
        TransformHandle m_transformHandle2;

        AnimationHandle m_animationHandle1;
        AnimationHandle m_animationHandle2;
    };
}

#endif
