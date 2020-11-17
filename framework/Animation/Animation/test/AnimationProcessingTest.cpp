//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AnimationProcessingTest.h"
#include "AnimationTestUtils.h"
#include "SplineTestUtils.h"
#include "Animation/SplineKey.h"
#include "Animation/AnimatableTypeTraits.h"
#include "SceneAPI/Handles.h"

namespace ramses_internal
{
    void AnimationProcessingTest::init()
    {
        AnimationTest::init();
        m_animationHandle = m_animationData.allocateAnimation(m_animationInstanceHandle);

        m_logic.addListener(&m_processing);
    }

    SplineHandle AnimationProcessingTest::initSpline(AnimationData& resMgr)
    {
        static const UInt NumKeys = 10u;
        static const SplineTimeStamp KeyTimeStep = 10u;

        SplineTimeStamp keyTime = 10u;
        for (SplineKeyIndex keyIdx = 0u; keyIdx < NumKeys; ++keyIdx)
        {
            SplineVec3::KeyType key(AnimationTestUtils::GetRandom<Vector3>(), AnimationTestUtils::GetRandom<Vector2>(), AnimationTestUtils::GetRandom<Vector2>());
            m_spline.setKey(keyTime, key);
            keyTime += KeyTimeStep;
        }

        m_lastSplineKeyIndex = m_spline.getNumKeys() - 1u;
        m_splineHandle = resMgr.allocateSpline(m_spline);

        return m_splineHandle;
    }

    TEST_F(AnimationProcessingTest, AnimatedDataNotModifiedBeforeStart)
    {
        init();
        const AnimationTime startTime(100u);
        const AnimationTime stopTime(200u);
        const Vector3 initialValue1 = m_container.getVal1(0);
        const Vector3 initialValue2 = m_container.getVal1(1);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);

        m_logic.setTime(50u);
        const Vector3 containerValue1 = m_container.getVal1(0);
        const Vector3 containerValue2 = m_container.getVal1(1);
        EXPECT_TRUE(AnimationTestUtils::AreEqual(initialValue1, containerValue1));
        EXPECT_TRUE(AnimationTestUtils::AreEqual(initialValue2, containerValue2));
    }

    TEST_F(AnimationProcessingTest, AnimatedDataNotModifiedAfterFinish)
    {
        init();
        const AnimationTime startTime(100u);
        const AnimationTime stopTime(200u);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);

        m_logic.setTime(50u);
        m_logic.setTime(100u);
        m_logic.setTime(150u);
        m_logic.setTime(200u);

        const Vector3 finishedValue1 = m_container.getVal1(0);
        const Vector3 finishedValue2 = m_container.getVal1(1);

        m_logic.setTime(250u);
        const Vector3 containerValue1 = m_container.getVal1(0);
        const Vector3 containerValue2 = m_container.getVal1(1);
        EXPECT_TRUE(AnimationTestUtils::AreEqual(finishedValue1, containerValue1));
        EXPECT_TRUE(AnimationTestUtils::AreEqual(finishedValue2, containerValue2));
    }

    TEST_F(AnimationProcessingTest, AnimatedDataNotModifiedIfSkippedToEnd)
    {
        init();
        const AnimationTime startTime(100u);
        const AnimationTime stopTime(200u);
        const Vector3 initialValue1 = m_container.getVal1(0);
        const Vector3 initialValue2 = m_container.getVal1(1);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);

        m_logic.setTime(200u);
        const Vector3 containerValue1 = m_container.getVal1(0);
        const Vector3 containerValue2 = m_container.getVal1(1);
        EXPECT_TRUE(AnimationTestUtils::AreEqual(initialValue1, containerValue1));
        EXPECT_TRUE(AnimationTestUtils::AreEqual(initialValue2, containerValue2));
    }

    TEST_F(AnimationProcessingTest, SwitchingRelativeOnAndOff)
    {
        init();
        m_container.setVal1(0, Vector3(-9, -99, -999));
        m_container.setVal1(1, Vector3(-9, -99, -999));
        const Vector3 splineValue = m_spline.getKey(m_lastSplineKeyIndex).m_value;

        // Non-relative
        const AnimationTime startTime(100u);
        const AnimationTime stopTime(200u);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);

        m_logic.setTime(50u);
        m_logic.setTime(150u);
        m_logic.setTime(250u);

        Vector3 finishedValue1 = m_container.getVal1(0);
        Vector3 finishedValue2 = m_container.getVal1(1);
        EXPECT_TRUE(AnimationTestUtils::AreEqual(splineValue, finishedValue1));
        EXPECT_TRUE(AnimationTestUtils::AreEqual(splineValue, finishedValue2));

        // Relative
        const Vector3 initialVal1 = finishedValue1;
        const Vector3 initialVal2 = finishedValue2;
        m_animationData.setAnimationProperties(m_animationHandle, 1.f, Animation::EAnimationFlags_Relative, 0u);
        m_animationData.setAnimationTimeRange(m_animationHandle, 300u, 400u);
        m_logic.setTime(350u);
        m_logic.setTime(450u);
        finishedValue1 = m_container.getVal1(0);
        finishedValue2 = m_container.getVal1(1);
        EXPECT_TRUE(AnimationTestUtils::AreEqual(splineValue + initialVal1, finishedValue1));
        EXPECT_TRUE(AnimationTestUtils::AreEqual(splineValue + initialVal2, finishedValue2));

        // Non-relative again
        m_animationData.setAnimationProperties(m_animationHandle, 1.f, 0u, 0u);
        m_animationData.setAnimationTimeRange(m_animationHandle, 500u, 600u);
        m_logic.setTime(550u);
        m_logic.setTime(650u);
        finishedValue1 = m_container.getVal1(0);
        finishedValue2 = m_container.getVal1(1);
        EXPECT_TRUE(AnimationTestUtils::AreEqual(splineValue, finishedValue1));
        EXPECT_TRUE(AnimationTestUtils::AreEqual(splineValue, finishedValue2));
    }

    TEST_F(AnimationProcessingTest, AnimatedDataAtStartKey)
    {
        animatedDataAtStartKey();
    }

    TEST_F(AnimationProcessingTest, AnimatedDataAtEndKey)
    {
        animatedDataAtEndKey();
    }

    TEST_F(AnimationProcessingTest, AnimatedDataAtKeys)
    {
        animatedDataAtKeys();
    }

    TEST_F(AnimationProcessingTest, AnimatedDataInBetweenKeys)
    {
        animatedDataInBetweenKeys();
    }

    TEST_F(AnimationProcessingTest, AnimatedDataIncrementalTimeAdvance)
    {
        animatedDataIncrementalTimeAdvance();
    }

    TEST_F(AnimationProcessingTest, AnimatedDataIncrementalTimeAdvanceWithTargetChanges)
    {
        animatedDataIncrementalTimeAdvanceWithTargetChanges();
    }

    TEST_F(AnimationProcessingTest, RelativeAnimatedDataAtStartKey)
    {
        animatedDataAtStartKey(Vector3(1, 2, 3), Vector3(-4, -5, -6), Animation::EAnimationFlags_Relative);
    }

    TEST_F(AnimationProcessingTest, RelativeAnimatedDataAtEndKey)
    {
        animatedDataAtEndKey(Vector3(1, 2, 3), Vector3(-4, -5, -6), Animation::EAnimationFlags_Relative);
    }

    TEST_F(AnimationProcessingTest, RelativeAnimatedDataAtKeys)
    {
        animatedDataAtKeys(Vector3(1, 2, 3), Vector3(-4, -5, -6), Animation::EAnimationFlags_Relative);
    }

    TEST_F(AnimationProcessingTest, RelativeAnimatedDataInBetweenKeys)
    {
        animatedDataInBetweenKeys(Vector3(1, 2, 3), Vector3(-4, -5, -6), Animation::EAnimationFlags_Relative);
    }

    TEST_F(AnimationProcessingTest, RelativeAnimatedDataIncrementalTimeAdvance)
    {
        animatedDataIncrementalTimeAdvance(Vector3(1, 2, 3), Vector3(-4, -5, -6), Animation::EAnimationFlags_Relative);
    }

    TEST_F(AnimationProcessingTest, RelativeAnimatedDataIncrementalTimeAdvanceWithTargetChanges)
    {
        animatedDataIncrementalTimeAdvanceWithTargetChanges(Vector3(1, 2, 3), Vector3(-4, -5, -6), Animation::EAnimationFlags_Relative);
    }

    TEST_F(AnimationProcessingTest, PauseAnimationResult)
    {
        init();
        m_animationData.setAnimationTimeRange(m_animationHandle, 100u, 200u);
        m_logic.setTime(150u);
        m_animationData.setAnimationPaused(m_animationHandle, true);
        const Vector3 containerValuePause1 = m_container.getVal1(0);
        const Vector3 containerValuePause2 = m_container.getVal1(1);
        m_logic.setTime(175u);
        m_logic.setTime(200u);
        const Vector3 containerValueFinish1 = m_container.getVal1(0);
        const Vector3 containerValueFinish2 = m_container.getVal1(1);

        EXPECT_TRUE(AnimationTestUtils::AreEqual(containerValuePause1, containerValueFinish1));
        EXPECT_TRUE(AnimationTestUtils::AreEqual(containerValuePause2, containerValueFinish2));
    }

    TEST_F(AnimationProcessingTest, ChangeLoopingDuringPlayback)
    {
        init();
        m_animationData.setAnimationTimeRange(m_animationHandle, 100u, 200u);
        m_animationData.setAnimationProperties(m_animationHandle, 1.f, 0u, 50u);
        m_logic.setTime(100u);
        m_animationData.setAnimationProperties(m_animationHandle, 1.f, Animation::EAnimationFlags_Looping, 50u);
        m_logic.setTime(125u);
        const Vector3 containerValueMidLoop1 = m_container.getVal1(0);
        const Vector3 containerValueMidLoop2 = m_container.getVal1(1);
        m_logic.setTime(175u);
        const Vector3 containerValueMid2ndLoop1 = m_container.getVal1(0);
        const Vector3 containerValueMid2ndLoop2 = m_container.getVal1(1);

        EXPECT_TRUE(AnimationTestUtils::AreEqual(containerValueMidLoop1, containerValueMid2ndLoop1));
        EXPECT_TRUE(AnimationTestUtils::AreEqual(containerValueMidLoop2, containerValueMid2ndLoop2));
    }

    TEST_F(AnimationProcessingTest, DoublePlaybackSpeedLooped)
    {
        init();
        const SplineTimeStamp lastKeyTime = m_spline.getTimeStamp(m_lastSplineKeyIndex);
        const AnimationTime startTime(0u);
        const AnimationTime stopTime(lastKeyTime);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);
        m_animationData.setAnimationProperties(m_animationHandle, 2.f, Animation::EAnimationFlags_Looping, 100u);

        for (SplineKeyIndex keyIdx = 0u; keyIdx < m_lastSplineKeyIndex; ++keyIdx)
        {
            const SplineTimeStamp keyTime = m_spline.getTimeStamp(keyIdx);
            m_logic.setTime(keyTime);

            const Vector3 containerValue = m_container.getVal1(0);

            // due the spline starts at time unit 10 and animation at 0, the doubled playbackSpeed will result in the doubled plus one key
            SplineTimeStamp newKeyIdx = (keyIdx * 2u + 1) % (m_lastSplineKeyIndex+1);

            // due lopping the last key will directy become the first key
            if (newKeyIdx == m_lastSplineKeyIndex)
                newKeyIdx = 0;

            const Vector3 splineValue = m_spline.getKey(newKeyIdx).m_value;

            EXPECT_TRUE(AnimationTestUtils::AreEqual(splineValue, containerValue));
        }
    }

    TEST_F(AnimationProcessingTest, HalfPlaybackSpeed)
    {
        init();
        const SplineTimeStamp lastKeyTime = m_spline.getTimeStamp(m_lastSplineKeyIndex);
        const AnimationTime startTime(0u);
        const AnimationTime stopTime(lastKeyTime);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);
        m_animationData.setAnimationProperties(m_animationHandle, 0.5f, 0u, 0u);

        SplineKeyIndex keyIdx1 = 0;
        SplineKeyIndex keyIdx2 = 0;

        for (SplineKeyIndex keyIdx = 1u; keyIdx < m_lastSplineKeyIndex; ++keyIdx)
        {
            const SplineTimeStamp keyTime = m_spline.getTimeStamp(keyIdx);
            m_logic.setTime(keyTime);

            keyIdx1 = (keyIdx-1) / 2;
            keyIdx2 = keyIdx / 2;

            const Vector3 containerValue = m_container.getVal1(0);
            const Vector3 middleSplineValue1 = m_spline.getKey(keyIdx1).m_value;
            const Vector3 middleSplineValue2 = m_spline.getKey(keyIdx2).m_value;
            const Vector3 middleSplineValue = (middleSplineValue1 + middleSplineValue2) * 0.5f;

            EXPECT_TRUE(AnimationTestUtils::AreEqual(middleSplineValue, containerValue));
        }
    }

    TEST_F(AnimationProcessingTest, ApplyInitialValue)
    {
        init();
        const Animation& animation = m_animationData.getAnimation(m_animationHandle);
        const AnimationTime startTime(100u);
        const AnimationTime stopTime(200u);
        const Vector3 initialValue1 = m_container.getVal1(0);
        const Vector3 initialValue2 = m_container.getVal1(1);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);

        m_logic.setTime(150u);
        Vector3 containerValue1 = m_container.getVal1(0);
        Vector3 containerValue2 = m_container.getVal1(1);
        EXPECT_FALSE(AnimationTestUtils::AreEqual(initialValue1, containerValue1));
        EXPECT_FALSE(AnimationTestUtils::AreEqual(initialValue2, containerValue2));

        m_animationData.setAnimationProperties(m_animationHandle, animation.m_playbackSpeed, animation.m_flags | Animation::EAnimationFlags_ApplyInitialValue, animation.m_loopDuration);

        m_logic.setTime(160u);
        containerValue1 = m_container.getVal1(0);
        containerValue2 = m_container.getVal1(1);
        EXPECT_TRUE(AnimationTestUtils::AreEqual(initialValue1, containerValue1));
        EXPECT_TRUE(AnimationTestUtils::AreEqual(initialValue2, containerValue2));
    }

    TEST_F(AnimationProcessingTest, ReversePlaying)
    {
        init();
        const SplineTimeStamp lastKeyTime = m_spline.getTimeStamp(m_lastSplineKeyIndex);
        const AnimationTime startTime(0u);
        const AnimationTime stopTime(lastKeyTime);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);
        m_animationData.setAnimationProperties(m_animationHandle, 1.f, Animation::EAnimationFlags_Reverse, 0u);

        for (SplineKeyIndex keyIdx = 0u; keyIdx < m_lastSplineKeyIndex; ++keyIdx)
        {
            const SplineTimeStamp keyTime = m_spline.getTimeStamp(keyIdx);
            m_logic.setTime(keyTime);

            const Vector3 containerValue = m_container.getVal1(0);

            const SplineTimeStamp newKeyIdx = m_lastSplineKeyIndex - keyIdx - 1;
            const Vector3 splineValue = m_spline.getKey(newKeyIdx).m_value;

            EXPECT_TRUE(AnimationTestUtils::AreEqual(splineValue, containerValue));
        }
    }

    TEST_F(AnimationProcessingTest, AnimateSingleComponentData)
    {
        AnimateSingleComponentDataTest<Vector2>(EVectorComponent_X, EVectorComponent_Y);
        AnimateSingleComponentDataTest<Vector2>(EVectorComponent_Y, EVectorComponent_Y);
        AnimateSingleComponentDataTest<Vector3>(EVectorComponent_X, EVectorComponent_Z);
        AnimateSingleComponentDataTest<Vector3>(EVectorComponent_Y, EVectorComponent_Z);
        AnimateSingleComponentDataTest<Vector3>(EVectorComponent_Z, EVectorComponent_Z);
        AnimateSingleComponentDataTest<Vector4>(EVectorComponent_X, EVectorComponent_W);
        AnimateSingleComponentDataTest<Vector4>(EVectorComponent_Y, EVectorComponent_W);
        AnimateSingleComponentDataTest<Vector4>(EVectorComponent_Z, EVectorComponent_W);
        AnimateSingleComponentDataTest<Vector4>(EVectorComponent_W, EVectorComponent_W);
        AnimateSingleComponentDataTest<Vector2i>(EVectorComponent_X, EVectorComponent_Y);
        AnimateSingleComponentDataTest<Vector2i>(EVectorComponent_Y, EVectorComponent_Y);
        AnimateSingleComponentDataTest<Vector3i>(EVectorComponent_X, EVectorComponent_Z);
        AnimateSingleComponentDataTest<Vector3i>(EVectorComponent_Y, EVectorComponent_Z);
        AnimateSingleComponentDataTest<Vector3i>(EVectorComponent_Z, EVectorComponent_Z);
        AnimateSingleComponentDataTest<Vector4i>(EVectorComponent_X, EVectorComponent_W);
        AnimateSingleComponentDataTest<Vector4i>(EVectorComponent_Y, EVectorComponent_W);
        AnimateSingleComponentDataTest<Vector4i>(EVectorComponent_Z, EVectorComponent_W);
        AnimateSingleComponentDataTest<Vector4i>(EVectorComponent_W, EVectorComponent_W);
    }

    void AnimationProcessingTest::animatedDataAtStartKey(const Vector3& initVal1, const Vector3& initVal2, Animation::Flags flags)
    {
        init();
        m_container.setVal1(0, initVal1);
        m_container.setVal1(1, initVal2);

        const AnimationTime startTime(100u);
        const AnimationTime stopTime(200u);
        m_animationData.setAnimationProperties(m_animationHandle, 1.f, flags, 0u);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);

        m_logic.setTime(startTime);
        const Vector3 splineValue = m_spline.getKey(0u).m_value;
        const Vector3 containerValue1 = m_container.getVal1(0);
        const Vector3 containerValue2 = m_container.getVal1(1);
        EXPECT_TRUE(AnimationTestUtils::AreEqual(initVal1 + splineValue, containerValue1));
        EXPECT_TRUE(AnimationTestUtils::AreEqual(initVal2 + splineValue, containerValue2));
    }

    void AnimationProcessingTest::animatedDataAtEndKey(const Vector3& initVal1, const Vector3& initVal2, Animation::Flags flags)
    {
        init();
        m_container.setVal1(0, initVal1);
        m_container.setVal1(1, initVal2);

        const AnimationTime startTime(100u);
        const AnimationTime stopTime(200u);
        m_animationData.setAnimationProperties(m_animationHandle, 1.f, flags, 0u);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);

        // Must start animation first
        m_logic.setTime(startTime);
        m_logic.setTime(stopTime);

        const Vector3 splineValue = m_spline.getKey(m_lastSplineKeyIndex).m_value;
        const Vector3 containerValue1 = m_container.getVal1(0);
        const Vector3 containerValue2 = m_container.getVal1(1);
        EXPECT_TRUE(AnimationTestUtils::AreEqual(initVal1 + splineValue, containerValue1));
        EXPECT_TRUE(AnimationTestUtils::AreEqual(initVal2 + splineValue, containerValue2));
    }

    void AnimationProcessingTest::animatedDataAtKeys(const Vector3& initVal1, const Vector3& initVal2, Animation::Flags flags)
    {
        init();
        m_container.setVal1(0, initVal1);
        m_container.setVal1(1, initVal2);

        const SplineTimeStamp lastKeyTime = m_spline.getTimeStamp(m_lastSplineKeyIndex);
        const AnimationTime startTime(0u);
        const AnimationTime stopTime(lastKeyTime);
        m_animationData.setAnimationProperties(m_animationHandle, 1.f, flags, 0u);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);

        for (SplineKeyIndex keyIdx = 0u; keyIdx <= m_lastSplineKeyIndex; ++keyIdx)
        {
            const SplineTimeStamp keyTime = m_spline.getTimeStamp(keyIdx);
            m_logic.setTime(keyTime);

            const Vector3 splineValue = m_spline.getKey(keyIdx).m_value;
            const Vector3 containerValue1 = m_container.getVal1(0);
            const Vector3 containerValue2 = m_container.getVal1(1);
            EXPECT_TRUE(AnimationTestUtils::AreEqual(initVal1 + splineValue, containerValue1));
            EXPECT_TRUE(AnimationTestUtils::AreEqual(initVal2 + splineValue, containerValue2));
        }
    }

    void AnimationProcessingTest::animatedDataInBetweenKeys(const Vector3& initVal1, const Vector3& initVal2, Animation::Flags flags)
    {
        init();
        m_container.setVal1(0, initVal1);
        m_container.setVal1(1, initVal2);

        const SplineTimeStamp lastKeyTime = m_spline.getTimeStamp(m_lastSplineKeyIndex);
        const AnimationTime startTime(0u);
        const AnimationTime stopTime(lastKeyTime);
        m_animationData.setAnimationProperties(m_animationHandle, 1.f, flags, 0u);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);

        for (SplineKeyIndex keyIdx1 = 0u; keyIdx1 < m_lastSplineKeyIndex; ++keyIdx1)
        {
            const SplineKeyIndex keyIdx2 = keyIdx1 + 1u;
            const SplineTimeStamp keyTime1 = m_spline.getTimeStamp(keyIdx1);
            const SplineTimeStamp keyTime2 = m_spline.getTimeStamp(keyIdx2);
            const AnimationTime timeBetweenKeys = (keyTime1 + keyTime2) / 2u;
            m_logic.setTime(timeBetweenKeys);

            const Vector3 splineValue1 = m_spline.getKey(keyIdx1).m_value;
            const Vector3 splineValue2 = m_spline.getKey(keyIdx2).m_value;
            const Vector3 splineValueBetweenKeys = (splineValue1 + splineValue2) * 0.5f;

            const Vector3 containerValue1 = m_container.getVal1(0);
            const Vector3 containerValue2 = m_container.getVal1(1);
            EXPECT_TRUE(AnimationTestUtils::AreEqual(initVal1 + splineValueBetweenKeys, containerValue1));
            EXPECT_TRUE(AnimationTestUtils::AreEqual(initVal2 + splineValueBetweenKeys, containerValue2));
        }
    }

    void AnimationProcessingTest::animatedDataIncrementalTimeAdvance(const Vector3& initVal1, const Vector3& initVal2, Animation::Flags flags)
    {
        init();
        m_container.setVal1(0, initVal1);
        m_container.setVal1(1, initVal2);

        const SplineTimeStamp lastKeyTime = m_spline.getTimeStamp(m_lastSplineKeyIndex);
        const AnimationTime startTime(1u);
        const AnimationTime stopTime(startTime + lastKeyTime);
        m_animationData.setAnimationProperties(m_animationHandle, 1.f, flags, 0u);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);
        SplineIterator iter;

        for (AnimationTime timeStamp = startTime; timeStamp <= stopTime; timeStamp += 1u)
        {
            m_logic.setTime(timeStamp);

            const SplineTimeStamp splineTimeStamp = SplineTimeStamp(timeStamp.getDurationSince(startTime));
            iter.setTimeStamp(splineTimeStamp, &m_spline);
            const SplineSegment& segment = iter.getSegment();

            const Vector3 splineValue1 = m_spline.getKey(segment.m_startIndex).m_value;
            const Vector3 splineValue2 = m_spline.getKey(segment.m_endIndex).m_value;
            const Vector3 splineValueBetweenKeys = splineValue1 + (splineValue2 - splineValue1) * iter.getSegmentLocalTime();

            const Vector3 containerValue1 = m_container.getVal1(0);
            const Vector3 containerValue2 = m_container.getVal1(1);
            EXPECT_TRUE(AnimationTestUtils::AreEqual(initVal1 + splineValueBetweenKeys, containerValue1));
            EXPECT_TRUE(AnimationTestUtils::AreEqual(initVal2 + splineValueBetweenKeys, containerValue2));
        }
    }

    void AnimationProcessingTest::animatedDataIncrementalTimeAdvanceWithTargetChanges(const Vector3& initVal1, const Vector3& initVal2, Animation::Flags flags)
    {
        init();
        m_container.setVal1(0, initVal1);
        m_container.setVal1(1, initVal2);

        const SplineBase* const pSpline = m_animationData.getSpline(m_splineHandle);
        EXPECT_NE(static_cast<const SplineBase *>(nullptr), pSpline);
        EXPECT_EQ(ESplineKeyType_Tangents, pSpline->getKeyType());
        EXPECT_EQ(EDataTypeID_Vector3f, pSpline->getDataType());
        const Spline<SplineKeyTangents, Vector3>& spline = static_cast<const Spline<SplineKeyTangents, Vector3>&>(*pSpline);

        const SplineTimeStamp lastKeyTime = spline.getTimeStamp(m_lastSplineKeyIndex);
        const AnimationTime startTime(1u);
        const AnimationTime stopTime(startTime + lastKeyTime);
        m_animationData.setAnimationProperties(m_animationHandle, 1.f, flags, 0u);
        m_animationData.setAnimationTimeRange(m_animationHandle, startTime, stopTime);

        for (AnimationTime timeStamp = startTime; timeStamp <= stopTime; timeStamp += 1u)
        {
            const SplineTimeStamp splineTimeStamp = SplineTimeStamp(timeStamp.getDurationSince(startTime));

            const AnimationTime::Duration stepNo = timeStamp.getDurationSince(startTime) + 1u;
            if ((stepNo % 5) == 0u)
            {
                SplineIterator iter;
                iter.setTimeStamp(splineTimeStamp, pSpline);
                const SplineSegment& segment = iter.getSegment();
                const Float fStep = Float(stepNo);
                const SplineKeyTangents<Vector3> key(Vector3(99.f * fStep), Vector2(-111.f), Vector2(0.f));
                m_animationData.setSplineKey(m_splineHandle, segment.m_endTimeStamp, key);
            }

            m_logic.setTime(timeStamp);

            SplineIterator iter;
            iter.setTimeStamp(splineTimeStamp, pSpline);
            const SplineSegment& segment = iter.getSegment();

            const Vector3 splineValue1 = spline.getKey(segment.m_startIndex).m_value;
            const Vector3 splineValue2 = spline.getKey(segment.m_endIndex).m_value;
            const Vector3 splineValueBetweenKeys = splineValue1 + (splineValue2 - splineValue1) * iter.getSegmentLocalTime();

            const Vector3 containerValue1 = m_container.getVal1(0);
            const Vector3 containerValue2 = m_container.getVal1(1);
            EXPECT_TRUE(AnimationTestUtils::AreEqual(initVal1 + splineValueBetweenKeys, containerValue1));
            EXPECT_TRUE(AnimationTestUtils::AreEqual(initVal2 + splineValueBetweenKeys, containerValue2));
        }
    }

    template <typename VectorData>
    void AnimationProcessingTest::AnimateSingleComponentDataTest(EVectorComponent component, EVectorComponent maxTypeComponent)
    {
        typedef AnimatableTypeTraits<VectorData> TypeTraits;
        using ComponentType = typename TypeTraits::ComponentType;
        using SplineInitializerComponent = SplineInitializer<ComponentType, 10U>;
        using ContainerVec = AnimationDataBindTestContainer<VectorData>;
        using DataBindVec = AnimationDataBind<ContainerVec, VectorData>;
        using DataBindVec1 = AnimationDataBind<ContainerVec, VectorData, MemoryHandle>;

        AnimationData animationData;
        SplineInitializerComponent splineInit;
        splineInit.initValuesRandom();
        splineInit.initSplineWithValues();
        const SplineHandle splineHandle = animationData.allocateSpline(splineInit.m_spline);

        const MemoryHandle handle = 1u;
        ContainerVec container;
        DataBindVec dataBind1(container, EDataBindAccessorType_Handles_None);
        DataBindVec1 dataBind2(container, handle, EDataBindAccessorType_Handles_1);
        DataBindHandle dataBindHandle1 = animationData.allocateDataBinding(dataBind1);
        DataBindHandle dataBindHandle2 = animationData.allocateDataBinding(dataBind2);

        const ComponentType initCompValue1 = 999;
        const ComponentType initCompValue2 = -999;
        VectorData initValue1 = VectorData(initCompValue1);
        VectorData initValue2 = VectorData(initCompValue2);
        container.setVal0(initValue1);
        container.setVal1(handle, initValue2);

        const AnimationInstanceHandle animInstHandle = animationData.allocateAnimationInstance(splineHandle, EInterpolationType_Step, component);
        animationData.addDataBindingToAnimationInstance(animInstHandle, dataBindHandle1);
        animationData.addDataBindingToAnimationInstance(animInstHandle, dataBindHandle2);
        EXPECT_EQ(1u, animationData.getTotalAnimationInstanceCount());

        const AnimationTime animStart = 100u;
        const AnimationTime animEnd = 200u;
        const AnimationTime processTime = animStart + animEnd.getDurationSince(animStart) / 2u;
        const SplineTimeStamp splineStart = SplineTimeStamp(processTime.getDurationSince(animStart));
        const AnimationHandle animationHandle = animationData.allocateAnimation(animInstHandle);
        AnimationProcessing processing(animationData);
        AnimationLogic animLogic(animationData);
        animLogic.addListener(&processing);
        animationData.setAnimationTimeRange(animationHandle, animStart, animEnd);
        animLogic.setTime(processTime);
        splineInit.setTimeStamp(splineStart);

        const VectorData value1 = container.getVal0();
        const VectorData value2 = container.getVal1(handle);
        // Selected component is updated accordingly
        const ComponentType compValue1 = TypeTraits::GetComponent(value1, EVectorComponent(component));
        const ComponentType compValue2 = TypeTraits::GetComponent(value2, EVectorComponent(component));
        const ComponentType expectedValue = splineInit.m_segmentData.m_startValue;
        EXPECT_FLOAT_EQ(static_cast<Float>(expectedValue), static_cast<Float>(compValue1));
        EXPECT_FLOAT_EQ(static_cast<Float>(expectedValue), static_cast<Float>(compValue2));
        // All other components stay untouched
        for (UInt comp = EVectorComponent_X; comp <= UInt(maxTypeComponent); ++comp)
        {
            if (comp != static_cast<UInt>(component))
            {
                const ComponentType otherCompValue1 = TypeTraits::GetComponent(value1, EVectorComponent(comp));
                const ComponentType otherCompValue2 = TypeTraits::GetComponent(value2, EVectorComponent(comp));
                EXPECT_FLOAT_EQ(static_cast<Float>(initCompValue1), static_cast<Float>(otherCompValue1));
                EXPECT_FLOAT_EQ(static_cast<Float>(initCompValue2), static_cast<Float>(otherCompValue2));
            }
        }
    }
}
