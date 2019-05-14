//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <gtest/gtest.h>

#include "ClientTestUtils.h"
#include "ramses-client-api/Node.h"
#include "ramses-client-api/Animation.h"
#include "ramses-client-api/AnimationSequence.h"
#include "ramses-client-api/SplineLinearFloat.h"
#include "ramses-client-api/AnimatedProperty.h"
#include "AnimationSystemImpl.h"
#include "AnimationImpl.h"

#include "Animation/AnimationTime.h"

using namespace testing;

namespace ramses
{
    class AnimationSequenceTest : public LocalTestClientWithSceneAndAnimationSystem, public testing::Test
    {
    public:
        AnimationSequenceTest()
        {
            m_spline = animationSystem.createSplineLinearFloat("spline");
            m_spline->setKey(0u, 1.f);
            m_spline->setKey(SplineDuration, 0.f);
            Node* node = m_scene.createNode();
            m_animProperty = animationSystem.createAnimatedProperty(*node, EAnimatedProperty_Translation, EAnimatedPropertyComponent_X, "prop");
            m_animation = animationSystem.createAnimation(*m_animProperty, *m_spline, "animation");
            m_sequence = animationSystem.createAnimationSequence("sequence");
        }

    protected:
        typedef std::vector<Animation*> AnimationVector;
        void createSomeAnimationsAndPutToSequence(AnimationVector& animations)
        {
            animations.clear();
            Node* node = m_scene.createNode();
            AnimatedProperty* animProperty2 = animationSystem.createAnimatedProperty(*node, EAnimatedProperty_Translation, EAnimatedPropertyComponent_X, "prop2");
            AnimatedProperty* animProperty3 = animationSystem.createAnimatedProperty(*node, EAnimatedProperty_Translation, EAnimatedPropertyComponent_X, "prop3");
            AnimatedProperty* animProperty4 = animationSystem.createAnimatedProperty(*node, EAnimatedProperty_Translation, EAnimatedPropertyComponent_X, "prop4");
            AnimatedProperty* animProperty5 = animationSystem.createAnimatedProperty(*node, EAnimatedProperty_Translation, EAnimatedPropertyComponent_X, "prop5");
            Animation* animation2 = animationSystem.createAnimation(*animProperty2, *m_spline, "animation2");
            Animation* animation3 = animationSystem.createAnimation(*animProperty3, *m_spline, "animation3");
            Animation* animation4 = animationSystem.createAnimation(*animProperty4, *m_spline, "animation4");
            Animation* animation5 = animationSystem.createAnimation(*animProperty5, *m_spline, "animation5");
            animations.push_back(m_animation);
            animations.push_back(animation2);
            animations.push_back(animation3);
            animations.push_back(animation4);
            animations.push_back(animation5);

            m_sequence->addAnimation(*m_animation);
            m_sequence->addAnimation(*animation2, 3000u);
            m_sequence->addAnimation(*animation3, m_sequence->getAnimationStopTimeInSequence(*animation2));
            m_sequence->addAnimation(*animation4, 1000u + m_sequence->getAnimationStopTimeInSequence(*animation3));
            m_sequence->addAnimation(*animation5, 9999u);
        }

        typedef std::vector<sequenceTimeStamp_t> AnimationSequenceTimes;
        static AnimationSequenceTimes GetStartAndStopTimesWithinSequence(const AnimationVector& animations, const AnimationSequence& sequence)
        {
            AnimationSequenceTimes times;
            times.reserve(animations.size() * 2u);
            for (const auto& anim : animations)
            {
                const sequenceTimeStamp_t startTimeInSequence = sequence.getAnimationStartTimeInSequence(*anim);
                const sequenceTimeStamp_t stopTimeInSequence = sequence.getAnimationStopTimeInSequence(*anim);
                times.push_back(startTimeInSequence);
                times.push_back(stopTimeInSequence);
            }

            return times;
        }

        static void ExpectStartAndStopTimesWithinSequenceAdjustedBySpeedFactor(const AnimationSequenceTimes& expectedTimes, float speedFactor, const AnimationVector& animations, const AnimationSequence& sequence)
        {
            ASSERT_TRUE(expectedTimes.size() == animations.size() * 2u);
            for (ramses_internal::UInt i = 0u; i < animations.size(); ++i)
            {
                const Animation& anim = *animations[i];
                const sequenceTimeStamp_t startTimeInSequence = sequence.getAnimationStartTimeInSequence(anim);
                const sequenceTimeStamp_t stopTimeInSequence = sequence.getAnimationStopTimeInSequence(anim);
                const sequenceTimeStamp_t expectedStartTimeInSequence = static_cast<sequenceTimeStamp_t>(expectedTimes[i * 2u] * speedFactor);
                const sequenceTimeStamp_t expectedStopTimeInSequence  = static_cast<sequenceTimeStamp_t>(expectedTimes[i * 2u + 1u] * speedFactor);
                EXPECT_EQ(expectedStartTimeInSequence, startTimeInSequence);
                EXPECT_EQ(expectedStopTimeInSequence, stopTimeInSequence);
            }
        }

        static void ExpectStartTimeWRTSequence(globalTimeStamp_t sequenceStartTime, const AnimationVector& animations, const AnimationSequence& sequence)
        {
            for (const auto& anim : animations)
            {
                const sequenceTimeStamp_t timeInSeq = sequence.getAnimationStartTimeInSequence(*anim);
                EXPECT_EQ(sequenceStartTime + timeInSeq, anim->getStartTime());
            }
        }

        static void ExpectStartTimeWRTSequence(globalTimeStamp_t sequenceStartTime, const AnimationVector& animations, const AnimationSequenceTimes& expectedTimes)
        {
            ASSERT_TRUE(expectedTimes.size() == animations.size() * 2u);
            for (ramses_internal::UInt i = 0u; i < animations.size(); ++i)
            {
                const sequenceTimeStamp_t timeInSeq = expectedTimes[i * 2u];
                EXPECT_EQ(sequenceStartTime + timeInSeq, animations[i]->getStartTime());
            }
        }

        static void ExpectStopTimeWRTSequence(globalTimeStamp_t sequenceStartTime, const AnimationVector& animations, const AnimationSequence& sequence)
        {
            for (const auto& anim : animations)
            {
                const sequenceTimeStamp_t timeInSeq = sequence.getAnimationStopTimeInSequence(*anim);
                EXPECT_EQ(sequenceStartTime + timeInSeq, anim->getStopTime());
            }
        }

        static void ExpectStopTimeWRTSequence(globalTimeStamp_t sequenceStartTime, const AnimationVector& animations, const AnimationSequenceTimes& expectedTimes)
        {
            ASSERT_TRUE(expectedTimes.size() == animations.size() * 2u);
            for (ramses_internal::UInt i = 0u; i < animations.size(); ++i)
            {
                const sequenceTimeStamp_t timeInSeq = expectedTimes[i * 2u + 1u];
                EXPECT_EQ(sequenceStartTime + timeInSeq, animations[i]->getStopTime());
            }
        }

        static void ExpectStopTime(globalTimeStamp_t timeStamp, const AnimationVector& animations)
        {
            for (const auto& anim : animations)
            {
                EXPECT_EQ(timeStamp, anim->getStopTime());
            }
        }

        static const timeMilliseconds_t SplineDuration = 2000;

        SplineLinearFloat* m_spline;
        AnimatedProperty* m_animProperty;
        Animation* m_animation;
        AnimationSequence* m_sequence;
    };

    TEST_F(AnimationSequenceTest, initialValues)
    {
        EXPECT_EQ(0u, m_sequence->getNumberOfAnimations());
        EXPECT_FALSE(m_sequence->containsAnimation(*m_animation));
        EXPECT_EQ(1.f, m_sequence->getPlaybackSpeed());
        EXPECT_FALSE(m_sequence->isAnimationLooping(*m_animation));
        EXPECT_EQ(0, m_sequence->getAnimationLoopDuration(*m_animation));
        EXPECT_FALSE(m_sequence->isAnimationRelative(*m_animation));
        EXPECT_EQ(InvalidSequenceTimeStamp, m_sequence->getAnimationStartTimeInSequence(*m_animation));
        EXPECT_EQ(InvalidSequenceTimeStamp, m_sequence->getAnimationStopTimeInSequence(*m_animation));
    }

    TEST_F(AnimationSequenceTest, addAnimationDefault)
    {
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation));
        EXPECT_EQ(1u, m_sequence->getNumberOfAnimations());
        EXPECT_TRUE(m_sequence->containsAnimation(*m_animation));
        EXPECT_EQ(0u, m_sequence->getAnimationStartTimeInSequence(*m_animation));
        EXPECT_EQ(timeMilliseconds_t(SplineDuration), m_sequence->getAnimationStopTimeInSequence(*m_animation));
    }

    TEST_F(AnimationSequenceTest, addAnimationWithStartStopTimes)
    {
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation, 2500u, 5000u));
        EXPECT_EQ(1u, m_sequence->getNumberOfAnimations());
        EXPECT_TRUE(m_sequence->containsAnimation(*m_animation));
        EXPECT_EQ(2500u, m_sequence->getAnimationStartTimeInSequence(*m_animation));
        EXPECT_EQ(5000u, m_sequence->getAnimationStopTimeInSequence(*m_animation));
    }

    TEST_F(AnimationSequenceTest, addAnimationAfter)
    {
        Animation* animation2 = animationSystem.createAnimation(*m_animProperty, *m_spline, "animation");

        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation, 2500u, 5000u));
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*animation2, m_sequence->getAnimationStopTimeInSequence(*m_animation) + 3000u));
        EXPECT_EQ(2u, m_sequence->getNumberOfAnimations());
        EXPECT_TRUE(m_sequence->containsAnimation(*m_animation));
        EXPECT_TRUE(m_sequence->containsAnimation(*animation2));
        EXPECT_EQ(2500u, m_sequence->getAnimationStartTimeInSequence(*m_animation));
        EXPECT_EQ(5000u, m_sequence->getAnimationStopTimeInSequence(*m_animation));
        EXPECT_EQ(8000u, m_sequence->getAnimationStartTimeInSequence(*animation2));
        EXPECT_EQ(8000u + SplineDuration, m_sequence->getAnimationStopTimeInSequence(*animation2));
    }

    TEST_F(AnimationSequenceTest, removeAnimation)
    {
        Animation* animation2 = animationSystem.createAnimation(*m_animProperty, *m_spline, "animation");
        Animation* animation3 = animationSystem.createAnimation(*m_animProperty, *m_spline, "animation");

        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation));
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*animation2));
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*animation3));
        EXPECT_EQ(3u, m_sequence->getNumberOfAnimations());
        EXPECT_TRUE(m_sequence->containsAnimation(*m_animation));
        EXPECT_TRUE(m_sequence->containsAnimation(*animation2));
        EXPECT_TRUE(m_sequence->containsAnimation(*animation3));
        EXPECT_EQ(StatusOK, m_sequence->removeAnimation(*animation2));
        EXPECT_TRUE(m_sequence->containsAnimation(*m_animation));
        EXPECT_FALSE(m_sequence->containsAnimation(*animation2));
        EXPECT_TRUE(m_sequence->containsAnimation(*animation3));
    }

    TEST_F(AnimationSequenceTest, canGetSequenceStopTime)
    {
        Animation* animation2 = animationSystem.createAnimation(*m_animProperty, *m_spline, "animation");

        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation, 2500u, 3000u));
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*animation2, 1500u, 5000u));
        EXPECT_EQ(5000u, m_sequence->getAnimationSequenceStopTime());
    }

    TEST_F(AnimationSequenceTest, canGetSequenceStopTimeAfterLastAnimationRemoved)
    {
        Animation* animation2 = animationSystem.createAnimation(*m_animProperty, *m_spline, "animation");

        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation, 2500u, 3000u));
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*animation2, 1500u, 5000u));
        EXPECT_EQ(StatusOK, m_sequence->removeAnimation(*animation2));
        EXPECT_EQ(3000u, m_sequence->getAnimationSequenceStopTime());
    }

    TEST_F(AnimationSequenceTest, canGetSequenceStopTimeAfterSpeedIncrease)
    {
        Animation* animation2 = animationSystem.createAnimation(*m_animProperty, *m_spline, "animation");

        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation, 2500u, 3000u));
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*animation2, 1500u, 5000u));
        EXPECT_EQ(StatusOK, m_sequence->setPlaybackSpeed(2.f));
        EXPECT_EQ(2500u, m_sequence->getAnimationSequenceStopTime());
    }

    TEST_F(AnimationSequenceTest, canGetSequenceStopTimeAfterSpeedDecrease)
    {
        Animation* animation2 = animationSystem.createAnimation(*m_animProperty, *m_spline, "animation");

        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation, 2500u, 3000u));
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*animation2, 1500u, 5000u));
        EXPECT_EQ(StatusOK, m_sequence->setPlaybackSpeed(0.5f));
        EXPECT_EQ(10000u, m_sequence->getAnimationSequenceStopTime());
    }

    TEST_F(AnimationSequenceTest, startSetsStartTimeFromAnimationSystemForAllAnimationsInSequence)
    {
        EXPECT_EQ(StatusOK, animationSystem.setTime(3333u));

        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);

        EXPECT_EQ(StatusOK, m_sequence->start());

        ExpectStartTimeWRTSequence(3333u, animations, *m_sequence);
    }

    TEST_F(AnimationSequenceTest, startWithOffsetSetsStartTimeFromAnimationSystemForAllAnimationsInSequence)
    {
        EXPECT_EQ(StatusOK, animationSystem.setTime(3333u));

        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);

        EXPECT_EQ(StatusOK, m_sequence->start(555));

        ExpectStartTimeWRTSequence(3333u + 555u, animations, *m_sequence);
    }

    TEST_F(AnimationSequenceTest, startAtSetsStartTimeForAllAnimationsInSequence)
    {
        EXPECT_EQ(StatusOK, animationSystem.setTime(3333u));

        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);

        EXPECT_EQ(StatusOK, m_sequence->startAt(6666u));

        ExpectStartTimeWRTSequence(6666u, animations, *m_sequence);
    }

    TEST_F(AnimationSequenceTest, stopSetsStopTimeFromAnimationSystemForAllAnimationsInSequence)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);

        EXPECT_EQ(StatusOK, animationSystem.setTime(3333u));
        EXPECT_EQ(StatusOK, m_sequence->start());

        EXPECT_EQ(StatusOK, animationSystem.setTime(6666u));
        EXPECT_EQ(StatusOK, m_sequence->stop());

        ExpectStopTime(6666u, animations);
    }

    TEST_F(AnimationSequenceTest, stopWithOffsetSetsStopTimeFromAnimationSystemForAllAnimationsInSequence)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);

        EXPECT_EQ(StatusOK, animationSystem.setTime(3333u));
        EXPECT_EQ(StatusOK, m_sequence->start());

        EXPECT_EQ(StatusOK, animationSystem.setTime(6666u));
        EXPECT_EQ(StatusOK, m_sequence->stop(999u));

        ExpectStopTime(6666u + 999u, animations);
    }

    TEST_F(AnimationSequenceTest, stopAtSetsStopTimeForAllAnimationsInSequence)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);

        EXPECT_EQ(StatusOK, m_sequence->start());
        EXPECT_EQ(StatusOK, m_sequence->stopAt(9999u));

        ExpectStopTime(9999u, animations);
    }

    TEST_F(AnimationSequenceTest, restartSetsProperStartAndStopTimeForAllAnimationsInSequence)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);

        EXPECT_EQ(StatusOK, animationSystem.setTime(100u));
        EXPECT_EQ(StatusOK, m_sequence->start());
        EXPECT_EQ(StatusOK, animationSystem.setTime(4500u));
        EXPECT_EQ(StatusOK, m_sequence->stop());
        EXPECT_EQ(StatusOK, m_sequence->start());
        EXPECT_EQ(StatusOK, m_sequence->stopAt(9999u));

        ExpectStartTimeWRTSequence(4500u, animations, *m_sequence);
        ExpectStopTime(9999u, animations);
    }

    TEST_F(AnimationSequenceTest, defaultStopTimeStampMatchesSplineDuration)
    {
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation));

        EXPECT_EQ(StatusOK, m_sequence->startAt(5000u));
        EXPECT_EQ(static_cast<globalTimeStamp_t>(5000u + SplineDuration), m_animation->getStopTime());
    }

    TEST_F(AnimationSequenceTest, stopWithoutStartDoesNothing)
    {
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation));

        const globalTimeStamp_t invalidTime = ramses_internal::AnimationTime::InvalidTimeStamp;
        EXPECT_EQ(StatusOK, m_sequence->stop());
        EXPECT_EQ(invalidTime, m_animation->getStartTime());
        EXPECT_EQ(invalidTime, m_animation->getStopTime());
        EXPECT_EQ(StatusOK, m_sequence->stopAt(999u));
        EXPECT_EQ(invalidTime, m_animation->getStartTime());
        EXPECT_EQ(invalidTime, m_animation->getStopTime());
    }

    TEST_F(AnimationSequenceTest, setSequencePlaybackSpeedInvalidArg)
    {
        EXPECT_NE(StatusOK, m_sequence->setPlaybackSpeed(-1.f));
        EXPECT_NE(StatusOK, m_sequence->setPlaybackSpeed(0.f));
    }

    TEST_F(AnimationSequenceTest, setSequencePlaybackSpeed)
    {
        EXPECT_EQ(StatusOK, m_sequence->setPlaybackSpeed(10.f));
        EXPECT_EQ(10.f, m_sequence->getPlaybackSpeed());
        EXPECT_EQ(StatusOK, m_sequence->setPlaybackSpeed(1.f));
        EXPECT_EQ(1.f, m_sequence->getPlaybackSpeed());
    }

    TEST_F(AnimationSequenceTest, stoppingWhenAlreadyStoppedDoesNotChangeAnything)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);
        const AnimationSequenceTimes originalSequenceTimes = GetStartAndStopTimesWithinSequence(animations, *m_sequence);

        const globalTimeStamp_t sequenceStart = 10000u;
        EXPECT_EQ(StatusOK, animationSystem.setTime(sequenceStart));
        EXPECT_EQ(StatusOK, m_sequence->start());

        const globalTimeStamp_t sequenceStop = 16000u;
        EXPECT_EQ(StatusOK, animationSystem.setTime(sequenceStop));
        EXPECT_EQ(StatusOK, m_sequence->stop());

        EXPECT_EQ(StatusOK, animationSystem.setTime(sequenceStop + 1000u));
        EXPECT_EQ(StatusOK, m_sequence->stop());

        ExpectStartAndStopTimesWithinSequenceAdjustedBySpeedFactor(originalSequenceTimes, 1.f, animations, *m_sequence);
        ExpectStartTimeWRTSequence(sequenceStart, animations, *m_sequence);
        ExpectStopTime(sequenceStop, animations);
    }

    TEST_F(AnimationSequenceTest, stoppingAfterFinishedDoesNotChangeAnything)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);
        const AnimationSequenceTimes originalSequenceTimes = GetStartAndStopTimesWithinSequence(animations, *m_sequence);

        const globalTimeStamp_t sequenceStart = 10000u;
        EXPECT_EQ(StatusOK, animationSystem.setTime(sequenceStart));
        EXPECT_EQ(StatusOK, m_sequence->start());

        EXPECT_EQ(StatusOK, animationSystem.setTime(100000u));
        EXPECT_EQ(StatusOK, m_sequence->stop());

        ExpectStartAndStopTimesWithinSequenceAdjustedBySpeedFactor(originalSequenceTimes, 1.f, animations, *m_sequence);
        ExpectStartTimeWRTSequence(sequenceStart, animations, *m_sequence);
        ExpectStopTimeWRTSequence(sequenceStart, animations, *m_sequence);
    }

    TEST_F(AnimationSequenceTest, changeSequencePlaybackSpeedBeforeStart)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);
        const AnimationSequenceTimes originalSequenceTimes = GetStartAndStopTimesWithinSequence(animations, *m_sequence);

        const float newSpeed = 0.5f;
        const float speedFactor = 1.f / newSpeed;
        EXPECT_EQ(StatusOK, m_sequence->setPlaybackSpeed(newSpeed));

        const globalTimeStamp_t sequenceStart = 10000u;
        EXPECT_EQ(StatusOK, m_sequence->start(sequenceStart));

        ExpectStartAndStopTimesWithinSequenceAdjustedBySpeedFactor(originalSequenceTimes, speedFactor, animations, *m_sequence);
        ExpectStartTimeWRTSequence(sequenceStart, animations, *m_sequence);
        ExpectStopTimeWRTSequence(sequenceStart, animations, *m_sequence);
    }

    TEST_F(AnimationSequenceTest, changeSequencePlaybackSpeedAfterStart)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);
        const AnimationSequenceTimes originalSequenceTimes = GetStartAndStopTimesWithinSequence(animations, *m_sequence);

        const globalTimeStamp_t sequenceStart = 10000u;
        EXPECT_EQ(StatusOK, animationSystem.setTime(sequenceStart));
        EXPECT_EQ(StatusOK, m_sequence->start());

        const float newSpeed = 0.5f;
        const float speedFactor = 1.f / newSpeed;
        EXPECT_EQ(StatusOK, m_sequence->setPlaybackSpeed(newSpeed));

        ExpectStartAndStopTimesWithinSequenceAdjustedBySpeedFactor(originalSequenceTimes, speedFactor, animations, *m_sequence);
        ExpectStartTimeWRTSequence(sequenceStart, animations, *m_sequence);
        ExpectStopTimeWRTSequence(sequenceStart, animations, *m_sequence);
    }

    TEST_F(AnimationSequenceTest, changeSequencePlaybackSpeedDuringRestart)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);
        const AnimationSequenceTimes originalSequenceTimes = GetStartAndStopTimesWithinSequence(animations, *m_sequence);

        EXPECT_EQ(StatusOK, m_sequence->start());
        EXPECT_EQ(StatusOK, animationSystem.setTime(8500u));
        EXPECT_EQ(StatusOK, m_sequence->stop());

        const globalTimeStamp_t sequenceStart = 10000u;
        EXPECT_EQ(StatusOK, animationSystem.setTime(sequenceStart));

        const float newSpeed = 2.f;
        const float speedFactor = 1.f / newSpeed;
        EXPECT_EQ(StatusOK, m_sequence->setPlaybackSpeed(newSpeed));
        EXPECT_EQ(StatusOK, m_sequence->start());

        ExpectStartAndStopTimesWithinSequenceAdjustedBySpeedFactor(originalSequenceTimes, speedFactor, animations, *m_sequence);
        ExpectStartTimeWRTSequence(sequenceStart, animations, *m_sequence);
        ExpectStopTimeWRTSequence(sequenceStart, animations, *m_sequence);
    }

    TEST_F(AnimationSequenceTest, slowDownSequencePlaybackSpeedWhilePlaying)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);
        const AnimationSequenceTimes originalSequenceTimes = GetStartAndStopTimesWithinSequence(animations, *m_sequence);

        const globalTimeStamp_t sequenceStart = 10000u;
        EXPECT_EQ(StatusOK, m_sequence->start(sequenceStart));

        const globalTimeStamp_t currentTime = 16000u;
        EXPECT_EQ(StatusOK, animationSystem.setTime(currentTime));

        const float newSpeed = 0.5f;
        const float speedFactor = 1.f / newSpeed;
        EXPECT_EQ(StatusOK, m_sequence->setPlaybackSpeed(newSpeed));

        ExpectStartAndStopTimesWithinSequenceAdjustedBySpeedFactor(originalSequenceTimes, speedFactor, animations, *m_sequence);
        const globalTimeStamp_t newSequenceStart = currentTime - static_cast<globalTimeStamp_t>((currentTime - sequenceStart) * speedFactor);
        ExpectStartTimeWRTSequence(newSequenceStart, animations, *m_sequence);
        ExpectStopTimeWRTSequence(newSequenceStart, animations, *m_sequence);
    }

    TEST_F(AnimationSequenceTest, speedUpSequencePlaybackSpeedWhilePlaying)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);
        const AnimationSequenceTimes originalSequenceTimes = GetStartAndStopTimesWithinSequence(animations, *m_sequence);

        const globalTimeStamp_t sequenceStart = 10000u;
        EXPECT_EQ(StatusOK, m_sequence->start(sequenceStart));

        const globalTimeStamp_t currentTime = 16000u;
        EXPECT_EQ(StatusOK, animationSystem.setTime(currentTime));

        const float newSpeed = 2.f;
        const float speedFactor = 1.f / newSpeed;
        EXPECT_EQ(StatusOK, m_sequence->setPlaybackSpeed(newSpeed));

        ExpectStartAndStopTimesWithinSequenceAdjustedBySpeedFactor(originalSequenceTimes, speedFactor, animations, *m_sequence);
        const globalTimeStamp_t newSequenceStart = currentTime - static_cast<globalTimeStamp_t>((currentTime - sequenceStart) * speedFactor);
        ExpectStartTimeWRTSequence(newSequenceStart, animations, *m_sequence);
        ExpectStopTimeWRTSequence(newSequenceStart, animations, *m_sequence);
    }

    TEST_F(AnimationSequenceTest, changingSequencePlaybackSpeedAfterItFinishedDoesNotChangeGlobalTimeRangesOfItsAnimations)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);
        const AnimationSequenceTimes originalSequenceTimes = GetStartAndStopTimesWithinSequence(animations, *m_sequence);

        const globalTimeStamp_t sequenceStart = 10000u;
        EXPECT_EQ(StatusOK, animationSystem.setTime(sequenceStart));
        EXPECT_EQ(StatusOK, m_sequence->start());

        EXPECT_EQ(StatusOK, animationSystem.setTime(100000u));

        const float newSpeed = 0.5f;
        const float speedFactor = 1.f / newSpeed;
        EXPECT_EQ(StatusOK, m_sequence->setPlaybackSpeed(newSpeed));

        ExpectStartAndStopTimesWithinSequenceAdjustedBySpeedFactor(originalSequenceTimes, speedFactor, animations, *m_sequence);
        ExpectStartTimeWRTSequence(sequenceStart, animations, originalSequenceTimes);
        ExpectStopTimeWRTSequence(sequenceStart, animations, originalSequenceTimes);
    }

    TEST_F(AnimationSequenceTest, changingSequencePlaybackSpeedAfterItStoppedDoesNotChangeGlobalTimeRangesOfItsAnimations)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);
        const AnimationSequenceTimes originalSequenceTimes = GetStartAndStopTimesWithinSequence(animations, *m_sequence);

        const globalTimeStamp_t sequenceStart = 10000u;
        EXPECT_EQ(StatusOK, animationSystem.setTime(sequenceStart));
        EXPECT_EQ(StatusOK, m_sequence->start());

        const globalTimeStamp_t sequenceStop = 16000u;
        EXPECT_EQ(StatusOK, animationSystem.setTime(sequenceStop));
        EXPECT_EQ(StatusOK, m_sequence->stop());

        const float newSpeed = 0.5f;
        const float speedFactor = 1.f / newSpeed;
        EXPECT_EQ(StatusOK, m_sequence->setPlaybackSpeed(newSpeed));

        ExpectStartAndStopTimesWithinSequenceAdjustedBySpeedFactor(originalSequenceTimes, speedFactor, animations, *m_sequence);
        ExpectStartTimeWRTSequence(sequenceStart, animations, originalSequenceTimes);
        ExpectStopTime(sequenceStop, animations);
    }

    TEST_F(AnimationSequenceTest, setAnimationLooping)
    {
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation));

        EXPECT_EQ(StatusOK, m_sequence->setAnimationLooping(*m_animation));
        EXPECT_TRUE(m_sequence->isAnimationLooping(*m_animation));
        EXPECT_EQ(0, m_sequence->getAnimationLoopDuration(*m_animation));
    }

    TEST_F(AnimationSequenceTest, setAnimationLoopingWithDuration)
    {
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation));

        EXPECT_EQ(StatusOK, m_sequence->setAnimationLooping(*m_animation, 2000));
        EXPECT_TRUE(m_sequence->isAnimationLooping(*m_animation));
        EXPECT_EQ(2000, m_sequence->getAnimationLoopDuration(*m_animation));
    }

    TEST_F(AnimationSequenceTest, setAnimationLoopingWithNegativeDurationTreatedAsZeroDuration)
    {
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation));

        EXPECT_EQ(StatusOK, m_sequence->setAnimationLooping(*m_animation, -2000));
        EXPECT_TRUE(m_sequence->isAnimationLooping(*m_animation));
        EXPECT_EQ(0, m_sequence->getAnimationLoopDuration(*m_animation));
    }

    TEST_F(AnimationSequenceTest, setAnimationRelative)
    {
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation));

        EXPECT_EQ(StatusOK, m_sequence->setAnimationRelative(*m_animation));
        EXPECT_TRUE(m_sequence->isAnimationRelative(*m_animation));
    }

    TEST_F(AnimationSequenceTest, setAnimationAbsolute)
    {
        EXPECT_EQ(StatusOK, m_sequence->addAnimation(*m_animation));

        EXPECT_EQ(StatusOK, m_sequence->setAnimationRelative(*m_animation));
        EXPECT_EQ(StatusOK, m_sequence->setAnimationAbsolute(*m_animation));
        EXPECT_FALSE(m_sequence->isAnimationRelative(*m_animation));
    }

    TEST_F(AnimationSequenceTest, canValidate)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);
        EXPECT_EQ(StatusOK, m_sequence->validate());
    }

    TEST_F(AnimationSequenceTest, failsValidationIfEmpty)
    {
        EXPECT_NE(StatusOK, m_sequence->validate());
    }

    TEST_F(AnimationSequenceTest, failsValidationIfOneOfAnimationsDangling)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);
        this->animationSystem.destroy(*animations.back());
        EXPECT_NE(StatusOK, m_sequence->validate());
    }

    TEST_F(AnimationSequenceTest, failsValidationIfOneOfAnimationsInvalid)
    {
        AnimationVector animations;
        createSomeAnimationsAndPutToSequence(animations);
        this->animationSystem.destroy(*m_spline);
        EXPECT_NE(StatusOK, m_sequence->validate());
    }
}
