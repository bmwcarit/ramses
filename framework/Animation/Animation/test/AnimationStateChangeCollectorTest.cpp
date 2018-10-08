//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include <gtest/gtest.h>
#include "Animation/AnimationStateChangeCollector.h"

using namespace testing;

namespace ramses_internal
{
    class AnimationStateChangeCollectorTest : public testing::Test
    {
    public:
    protected:
        AnimationStateChangeCollector m_collector;
    };

    TEST_F(AnimationStateChangeCollectorTest, initialState)
    {
        EXPECT_EQ(0u, m_collector.getCollectedStartedAnimations().size());
        EXPECT_EQ(0u, m_collector.getCollectedFinishedAnimations().size());
    }

    TEST_F(AnimationStateChangeCollectorTest, collectsStartedAnimation)
    {
        const AnimationHandle handle(33u);
        m_collector.onAnimationStarted(handle);

        EXPECT_EQ(1u, m_collector.getCollectedStartedAnimations().size());
        EXPECT_EQ(0u, m_collector.getCollectedFinishedAnimations().size());
        EXPECT_EQ(handle, m_collector.getCollectedStartedAnimations()[0]);
    }

    TEST_F(AnimationStateChangeCollectorTest, collectsFinishedAnimation)
    {
        const AnimationHandle handle(33u);
        m_collector.onAnimationFinished(handle);

        EXPECT_EQ(0u, m_collector.getCollectedStartedAnimations().size());
        EXPECT_EQ(1u, m_collector.getCollectedFinishedAnimations().size());
        EXPECT_EQ(handle, m_collector.getCollectedFinishedAnimations()[0]);
    }

    TEST_F(AnimationStateChangeCollectorTest, collectsRestartedAnimationAsStarted)
    {
        const AnimationHandle handle(33u);
        m_collector.onAnimationStarted(handle);
        m_collector.onAnimationFinished(handle);
        m_collector.onAnimationStarted(handle);

        EXPECT_EQ(1u, m_collector.getCollectedStartedAnimations().size());
        EXPECT_EQ(0u, m_collector.getCollectedFinishedAnimations().size());
        EXPECT_EQ(handle, m_collector.getCollectedStartedAnimations()[0]);
    }

    TEST_F(AnimationStateChangeCollectorTest, collectsRestartedAndStoppedAnimationAsFinished)
    {
        const AnimationHandle handle(33u);
        m_collector.onAnimationFinished(handle);
        m_collector.onAnimationStarted(handle);
        m_collector.onAnimationFinished(handle);

        EXPECT_EQ(0u, m_collector.getCollectedStartedAnimations().size());
        EXPECT_EQ(1u, m_collector.getCollectedFinishedAnimations().size());
        EXPECT_EQ(handle, m_collector.getCollectedFinishedAnimations()[0]);
    }

    TEST_F(AnimationStateChangeCollectorTest, canResetCollections)
    {
        const AnimationHandle handle1(33u);
        const AnimationHandle handle2(43u);
        const AnimationHandle handle3(53u);
        m_collector.onAnimationStarted(handle1);
        m_collector.onAnimationFinished(handle2);
        m_collector.onAnimationStarted(handle3);

        m_collector.resetCollections();

        EXPECT_EQ(0u, m_collector.getCollectedStartedAnimations().size());
        EXPECT_EQ(0u, m_collector.getCollectedFinishedAnimations().size());
    }
}
