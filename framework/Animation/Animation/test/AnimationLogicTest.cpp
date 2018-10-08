//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AnimationLogicTest.h"

using namespace testing;

namespace ramses_internal
{
    TEST_F(AnimationLogicTest, InitializedToEmpty)
    {
        init();
        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());
    }

    TEST_F(AnimationLogicTest, SetTimeNotify)
    {
        init();
        const AnimationTime time(15u);
        EXPECT_CALL(m_stateListener, onTimeChanged(time));
        m_logic.setTime(time);
        m_logic.setTime(time);
    }

    TEST_F(AnimationLogicTest, AddNonExistingAnimation)
    {
        init();
        m_logic.enqueueAnimation(AnimationHandle::Invalid());

        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());
    }

    TEST_F(AnimationLogicTest, AddDefaultInvalidAnimation)
    {
        init();
        const AnimationHandle handle = m_animationData.allocateAnimation(m_animationInstanceHandle);

        m_logic.enqueueAnimation(handle);

        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());
    }

    TEST_F(AnimationLogicTest, AddInvalidAnimation)
    {
        init();
        const AnimationHandle handle = m_animationData.allocateAnimation(m_animationInstanceHandle);

        m_animationData.setAnimationTimeRange(handle, 0u, 0u);
        m_logic.enqueueAnimation(handle);
        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());

        m_animationData.setAnimationTimeRange(handle, AnimationTime::InvalidTimeStamp, AnimationTime::InvalidTimeStamp);
        m_logic.enqueueAnimation(handle);
        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());

        m_animationData.setAnimationTimeRange(handle, 0u, AnimationTime::InvalidTimeStamp);
        m_logic.enqueueAnimation(handle);
        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());

        m_animationData.setAnimationTimeRange(handle, AnimationTime::InvalidTimeStamp, 0u);
        m_logic.enqueueAnimation(handle);
        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());

        m_logic.setTime(1000u);
        m_animationData.setAnimationTimeRange(handle, 0u, 100u);
        m_logic.enqueueAnimation(handle);
        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());
    }

    TEST_F(AnimationLogicTest, AddValidPendingAnimation)
    {
        init();
        const AnimationHandle handle = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle, 1000u, 2000u);

        m_logic.enqueueAnimation(handle);

        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(1u, m_logic.getNumPendingAnimations());
    }

    TEST_F(AnimationLogicTest, AddExistingAnimation)
    {
        init();
        const AnimationHandle handle = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle, 1000u, 2000u);

        m_logic.enqueueAnimation(handle);
        m_logic.enqueueAnimation(handle);

        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(1u, m_logic.getNumPendingAnimations());
    }

    TEST_F(AnimationLogicTest, AddActiveAnimation)
    {
        init();
        const AnimationHandle handle = m_animationData.allocateAnimation(m_animationInstanceHandle);
        EXPECT_CALL(m_stateListener, onAnimationStarted(handle));
        m_animationData.setAnimationTimeRange(handle, 0u, 100u);

        EXPECT_EQ(1u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());
    }

    TEST_F(AnimationLogicTest, AddInactiveAnimationsAndSkipThem)
    {
        init();
        const AnimationHandle handle1 = m_animationData.allocateAnimation(m_animationInstanceHandle);
        const AnimationHandle handle2 = m_animationData.allocateAnimation(m_animationInstanceHandle);
        const AnimationHandle handle3 = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle1, 10u, 20u);
        m_animationData.setAnimationTimeRange(handle2, 30u, 40u);
        m_animationData.setAnimationTimeRange(handle3, 50u, 60u);

        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(3u, m_logic.getNumPendingAnimations());

        EXPECT_CALL(m_stateListener, onAnimationStarted(handle3));
        m_logic.setTime(55u);

        EXPECT_EQ(1u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());
    }

    TEST_F(AnimationLogicTest, SetTimeEqualOrInPastDoesNotChangeQueue)
    {
        init();
        const AnimationHandle handle1 = m_animationData.allocateAnimation(m_animationInstanceHandle);
        const AnimationHandle handle2 = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle1, 10u, 100u);
        m_animationData.setAnimationTimeRange(handle2, 1000u, 2000u);

        m_logic.setTime(20u);

        EXPECT_EQ(1u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(1u, m_logic.getNumPendingAnimations());

        m_logic.setTime(20u);

        EXPECT_EQ(1u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(1u, m_logic.getNumPendingAnimations());

        m_logic.setTime(0u);

        EXPECT_EQ(1u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(1u, m_logic.getNumPendingAnimations());
    }

    TEST_F(AnimationLogicTest, TimeUpdateChangesAnimationOrdering)
    {
        init();
        const AnimationHandle handle = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle, 100u, 200u);

        m_logic.setTime(0u);

        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(1u, m_logic.getNumPendingAnimations());

        m_logic.setTime(50u);
        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(1u, m_logic.getNumPendingAnimations());

        EXPECT_CALL(m_stateListener, onAnimationStarted(handle));
        m_logic.setTime(100u);
        EXPECT_EQ(1u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());

        m_logic.setTime(150u);
        EXPECT_EQ(1u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());

        EXPECT_CALL(m_stateListener, onAnimationFinished(handle));
        m_logic.setTime(200u);
        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());

        m_logic.setTime(250u);
        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());
        EXPECT_TRUE(m_animationData.containsAnimation(handle));
    }

    TEST_F(AnimationLogicTest, RestartAnimation)
    {
        init();
        const AnimationHandle handle = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle, 100u, 200u);

        m_logic.setTime(0u);
        EXPECT_CALL(m_stateListener, onAnimationStarted(handle));
        m_logic.setTime(150u);

        EXPECT_EQ(1u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());

        EXPECT_CALL(m_stateListener, onAnimationFinished(handle));
        m_logic.setTime(250u);
        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());

        const AnimationHandle handle2 = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle2, 300u, 400u);

        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(1u, m_logic.getNumPendingAnimations());

        EXPECT_CALL(m_stateListener, onAnimationStarted(handle2));
        m_logic.setTime(300u);
        EXPECT_EQ(1u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());

        EXPECT_CALL(m_stateListener, onAnimationFinished(handle2));
        m_logic.setTime(400u);
        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(0u, m_logic.getNumPendingAnimations());

        EXPECT_TRUE(m_animationData.containsAnimation(handle));
        EXPECT_TRUE(m_animationData.containsAnimation(handle2));
    }

    TEST_F(AnimationLogicTest, RestartAnimationWhilePlaying)
    {
        init();
        const AnimationHandle handle = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle, 100u, 300u);

        EXPECT_CALL(m_stateListener, onAnimationStarted(handle));
        m_logic.setTime(150u);

        EXPECT_CALL(m_stateListener, onAnimationFinished(handle));
        m_animationData.setAnimationTimeRange(handle, 400u, 600u);

        EXPECT_CALL(m_stateListener, onAnimationStarted(handle));
        m_logic.setTime(500u);

        EXPECT_CALL(m_stateListener, onAnimationStarted(handle));
        EXPECT_CALL(m_stateListener, onAnimationFinished(handle));
        m_animationData.setAnimationTimeRange(handle, 450u, 550u);

        EXPECT_CALL(m_stateListener, onAnimationFinished(handle));
        m_logic.setTime(550u);
    }

    TEST_F(AnimationLogicTest, MultipleOverlappingAnimations)
    {
        init();
        static const UInt LENGTH = 10u;
        static const UInt NUM_ANIMS_PER_LAYER = 10u;
        static const UInt NUM_LAYERS = 10u;
        static const UInt NUM_ANIMS = NUM_LAYERS * NUM_ANIMS_PER_LAYER;

        // Create NUM_LAYERS layers, each with NUM_ANIMS_PER_LAYER non-overlapping animations in it
        // covering whole time range. Each layer is shifted by 1 time unit. At any given point
        // within time range <NUM_LAYERS, LENGTH*NUM_ANIMS_PER_LAYER> there should be NUM_LAYERS animations active.
        m_logic.setTime(0u);
        AnimationTime::TimeStamp offset = 0u;
        for (UInt i = 0u; i < NUM_LAYERS; ++i)
        {
            ++offset;
            for (UInt a = 0u; a < NUM_ANIMS_PER_LAYER; ++a)
            {
                const AnimationHandle handle = m_animationData.allocateAnimation(m_animationInstanceHandle);
                AnimationTime startTime = a * LENGTH + offset;
                AnimationTime stopTime = (a + 1u) * LENGTH + offset;
                m_animationData.setAnimationTimeRange(handle, startTime, stopTime);
            }
        }
        EXPECT_EQ(0u, m_logic.getNumActiveAnimations());
        EXPECT_EQ(NUM_ANIMS, m_logic.getNumPendingAnimations());

        AnimationTime::TimeStamp time = 0u;
        for (UInt i = 0u; i < NUM_LAYERS; ++i, ++time)
        {
            m_logic.setTime(time);
            EXPECT_EQ(i, m_logic.getNumActiveAnimations());
            EXPECT_EQ(NUM_ANIMS - i, m_logic.getNumPendingAnimations());
        }

        for (UInt i = UInt(time); i < LENGTH * NUM_ANIMS_PER_LAYER + 1; ++i, ++time)
        {
            m_logic.setTime(time);
            EXPECT_EQ(NUM_LAYERS, m_logic.getNumActiveAnimations());
        }

        for (Int i = NUM_LAYERS - 1; i >= 0; --i, ++time)
        {
            m_logic.setTime(time);
            EXPECT_EQ(UInt(i), m_logic.getNumActiveAnimations());
            EXPECT_EQ(0u, m_logic.getNumPendingAnimations());
        }
    }

    TEST_F(AnimationLogicTest, CorrectActivationOfPendingAnimations)
    {
        init();
        const AnimationHandle handle0 = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle0, 591u, 10591u);
        const AnimationHandle handle1 = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle1, 591u, 20271u);
        const AnimationHandle handle2 = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle2, 591u, 3591u);
        const AnimationHandle handle3 = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle3, 3591u, 6591u);
        const AnimationHandle handle4 = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle4, 6591u, 9591u);
        const AnimationHandle handle5 = m_animationData.allocateAnimation(m_animationInstanceHandle);
        m_animationData.setAnimationTimeRange(handle5, 9591u, 12591u);

        m_logic.setTime(600u);
        EXPECT_TRUE(m_logic.isAnimationActive(handle0));
        EXPECT_TRUE(m_logic.isAnimationActive(handle1));
        EXPECT_TRUE(m_logic.isAnimationActive(handle2));
        EXPECT_FALSE(m_logic.isAnimationActive(handle3));
        EXPECT_FALSE(m_logic.isAnimationActive(handle4));
        EXPECT_FALSE(m_logic.isAnimationActive(handle5));

        m_logic.setTime(4000u);
        EXPECT_TRUE(m_logic.isAnimationActive(handle0));
        EXPECT_TRUE(m_logic.isAnimationActive(handle1));
        EXPECT_FALSE(m_logic.isAnimationActive(handle2));
        EXPECT_TRUE(m_logic.isAnimationActive(handle3));
        EXPECT_FALSE(m_logic.isAnimationActive(handle4));
        EXPECT_FALSE(m_logic.isAnimationActive(handle5));

        m_logic.setTime(7000u);
        EXPECT_TRUE(m_logic.isAnimationActive(handle0));
        EXPECT_TRUE(m_logic.isAnimationActive(handle1));
        EXPECT_FALSE(m_logic.isAnimationActive(handle2));
        EXPECT_FALSE(m_logic.isAnimationActive(handle3));
        EXPECT_TRUE(m_logic.isAnimationActive(handle4));
        EXPECT_FALSE(m_logic.isAnimationActive(handle5));

        m_logic.setTime(10000u);
        EXPECT_TRUE(m_logic.isAnimationActive(handle0));
        EXPECT_TRUE(m_logic.isAnimationActive(handle1));
        EXPECT_FALSE(m_logic.isAnimationActive(handle2));
        EXPECT_FALSE(m_logic.isAnimationActive(handle3));
        EXPECT_FALSE(m_logic.isAnimationActive(handle4));
        EXPECT_TRUE(m_logic.isAnimationActive(handle5));

        m_logic.setTime(13000u);
        EXPECT_FALSE(m_logic.isAnimationActive(handle0));
        EXPECT_TRUE(m_logic.isAnimationActive(handle1));
        EXPECT_FALSE(m_logic.isAnimationActive(handle2));
        EXPECT_FALSE(m_logic.isAnimationActive(handle3));
        EXPECT_FALSE(m_logic.isAnimationActive(handle4));
        EXPECT_FALSE(m_logic.isAnimationActive(handle5));

        m_logic.setTime(21000u);
        EXPECT_FALSE(m_logic.isAnimationActive(handle0));
        EXPECT_FALSE(m_logic.isAnimationActive(handle1));
        EXPECT_FALSE(m_logic.isAnimationActive(handle2));
        EXPECT_FALSE(m_logic.isAnimationActive(handle3));
        EXPECT_FALSE(m_logic.isAnimationActive(handle4));
        EXPECT_FALSE(m_logic.isAnimationActive(handle5));
    }
}
