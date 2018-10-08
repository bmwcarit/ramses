//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "AnimationDataTest.h"

using namespace testing;

namespace ramses_internal
{
    TEST_F(AnimationDataTest, InitialResources)
    {
        EXPECT_EQ(0u, m_animationData.getTotalSplineCount());
        EXPECT_EQ(0u, m_animationData.getTotalDataBindCount());
        EXPECT_EQ(0u, m_animationData.getTotalAnimationInstanceCount());
        EXPECT_EQ(0u, m_animationData.getTotalAnimationCount());
    }

    TEST_F(AnimationDataTest, allocateSpline)
    {
        const SplineHandle handle = m_animationData.allocateSpline(m_spline);
        EXPECT_TRUE(m_animationData.containsSpline(handle));
        EXPECT_EQ(1u, m_animationData.getTotalSplineCount());
    }

    TEST_F(AnimationDataTest, removeSpline)
    {
        const SplineHandle handle = m_animationData.allocateSpline(m_spline);
        m_animationData.removeSpline(handle);
        EXPECT_FALSE(m_animationData.containsSpline(handle));
    }

    TEST_F(AnimationDataTest, getSpline)
    {
        const SplineHandle splineHandle = m_animationData.allocateSpline(m_spline);
        const SplineBase* const pSplineManaged = m_animationData.getSpline(splineHandle);
        EXPECT_NE(static_cast<SplineBase*>(0), pSplineManaged);
        EXPECT_EQ(m_spline.getNumKeys(), pSplineManaged->getNumKeys());
    }

    TEST_F(AnimationDataTest, SplineContentsMatch)
    {
        SplineInitializerVec3_Large splineInit;
        splineInit.initValuesRandom();
        splineInit.initSplineWithValues();
        SplineVec3& spline = splineInit.m_spline;

        const SplineHandle splineHandle = m_animationData.allocateSpline(spline);
        const SplineBase* const pSplineManaged = m_animationData.getSpline(splineHandle);
        const SplineVec3& splineManaged = *(static_cast<const SplineVec3*>(pSplineManaged));
        for (SplineKeyIndex i = 0; i < spline.getNumKeys(); ++i)
        {
            EXPECT_EQ(spline.getTimeStamp(i), pSplineManaged->getTimeStamp(i));
            EXPECT_EQ(spline.getKey(i).m_value, splineManaged.getKey(i).m_value);
        }
    }

    TEST_F(AnimationDataTest, SplineRemoveKey)
    {
        SplineInitializerVec3_Large splineInit;
        splineInit.initValuesRandom();
        splineInit.initSplineWithValues();
        SplineVec3& spline = splineInit.m_spline;

        const SplineHandle splineHandle = m_animationData.allocateSpline(spline);
        const SplineBase* const pSplineManaged = m_animationData.getSpline(splineHandle);
        EXPECT_NE(static_cast<SplineBase*>(0), pSplineManaged);
        const SplineVec3& splineManaged = *(static_cast<const SplineVec3*>(pSplineManaged));

        const UInt32 numKeys = spline.getNumKeys();
        EXPECT_EQ(numKeys, splineManaged.getNumKeys());

        EXPECT_CALL(m_dataListener, onSplineChanged(splineHandle));
        m_animationData.removeSplineKey(splineHandle, numKeys / 2);
        EXPECT_EQ(numKeys - 1u, splineManaged.getNumKeys());
        EXPECT_EQ(spline.getTimeStamp(0u), splineManaged.getTimeStamp(0u));
    }

    TEST_F(AnimationDataTest, SplineRemoveKeys)
    {
        SplineInitializerVec3_Large splineInit;
        splineInit.initValuesRandom();
        splineInit.initSplineWithValues();
        SplineVec3& spline = splineInit.m_spline;

        const SplineHandle splineHandle = m_animationData.allocateSpline(spline);
        const SplineBase* const pSplineManaged = m_animationData.getSpline(splineHandle);
        EXPECT_NE(static_cast<SplineBase*>(0), pSplineManaged);
        const SplineVec3& splineManaged = *(static_cast<const SplineVec3*>(pSplineManaged));

        EXPECT_CALL(m_dataListener, onSplineChanged(splineHandle));
        m_animationData.removeSplineKeys(splineHandle);
        EXPECT_EQ(0u, splineManaged.getNumKeys());
    }

    TEST_F(AnimationDataTest, SplineValueChange)
    {
        SplineInitializerVec3_Large splineInit;
        splineInit.initValuesRandom();
        splineInit.initSplineWithValues();
        SplineVec3& spline = splineInit.m_spline;

        const SplineHandle splineHandle = m_animationData.allocateSpline(spline);
        const SplineBase* const pSplineManaged = m_animationData.getSpline(splineHandle);
        EXPECT_NE(static_cast<SplineBase*>(0), pSplineManaged);
        EXPECT_EQ(ESplineKeyType_Tangents, pSplineManaged->getKeyType());
        EXPECT_EQ(EDataTypeID_Vector3f, pSplineManaged->getDataType());
        const SplineVec3& splineManaged = *(static_cast<const SplineVec3*>(pSplineManaged));

        SplineKeyTangents<Vector3> key(Vector3(999.f), Vector2(-111.f), Vector2(0.f));
        EXPECT_CALL(m_dataListener, onSplineChanged(splineHandle));
        const SplineKeyIndex keyIndex2 = m_animationData.setSplineKey(splineHandle, 0u, key);
        EXPECT_EQ(0u, keyIndex2);
        EXPECT_TRUE(AnimationTestUtils::AreEqual(key.m_value, splineManaged.getKey(0u).m_value));
        EXPECT_TRUE(AnimationTestUtils::AreEqual(key.m_tangentIn, splineManaged.getKey(0u).m_tangentIn));
        EXPECT_TRUE(AnimationTestUtils::AreEqual(key.m_tangentOut, splineManaged.getKey(0u).m_tangentOut));
    }

    TEST_F(AnimationDataTest, AllocateDataBinding)
    {
        const DataBindHandle handle = m_animationData.allocateDataBinding(m_dataBind);
        EXPECT_TRUE(m_animationData.containsDataBinding(handle));
        EXPECT_EQ(1u, m_animationData.getTotalDataBindCount());
    }

    TEST_F(AnimationDataTest, RemoveDataBinding)
    {
        const DataBindHandle handle = m_animationData.allocateDataBinding(m_dataBind);
        m_animationData.removeDataBinding(handle);
        EXPECT_FALSE(m_animationData.containsDataBinding(handle));
    }

    TEST_F(AnimationDataTest, GetDataBinding)
    {
        const DataBindHandle handle = m_animationData.allocateDataBinding(m_dataBind);
        const AnimationDataBindBase* const pDataBindManaged = m_animationData.getDataBinding(handle);
        EXPECT_NE(static_cast<AnimationDataBindBase*>(0), pDataBindManaged);
    }

    TEST_F(AnimationDataTest, DataBindingContentsMatch)
    {
        for (MemoryHandle handle(0u); handle < AnimationDataBindTestContainer<UInt>::NumHandles; ++handle)
        {
            testAnimationManagerDataBindingTypes(handle, true, false);
            testAnimationManagerDataBindingTypes(handle, -1, 999);
            testAnimationManagerDataBindingTypes(handle, 1e-6f, 999.999f);
            testAnimationManagerDataBindingTypes(handle, Vector3(1, 2, 3), Vector3(-1, -2, -3));
        }
    }

    TEST_F(AnimationDataTest, allocateAnimationInstance)
    {
        const SplineHandle splineHandle = m_animationData.allocateSpline(m_spline);
        const AnimationInstanceHandle handle = m_animationData.allocateAnimationInstance(splineHandle, EInterpolationType_Linear);
        EXPECT_CALL(m_dataListener, onAnimationInstanceChanged(handle));
        m_animationData.addDataBindingToAnimationInstance(handle, m_animationData.allocateDataBinding(m_dataBind));
        EXPECT_TRUE(m_animationData.containsAnimationInstance(handle));
        EXPECT_EQ(1u, m_animationData.getTotalAnimationInstanceCount());
    }

    TEST_F(AnimationDataTest, removeAnimationInstance)
    {
        const SplineHandle splineHandle = m_animationData.allocateSpline(m_spline);
        const AnimationInstanceHandle handle = m_animationData.allocateAnimationInstance(splineHandle, EInterpolationType_Linear);
        EXPECT_CALL(m_dataListener, onAnimationInstanceChanged(handle));
        m_animationData.addDataBindingToAnimationInstance(handle, m_animationData.allocateDataBinding(m_dataBind));
        m_animationData.removeAnimationInstance(handle);
        EXPECT_FALSE(m_animationData.containsAnimationInstance(handle));
    }

    TEST_F(AnimationDataTest, GetAnimationInstance)
    {
        const SplineHandle splineHandle = m_animationData.allocateSpline(m_spline);
        const AnimationInstanceHandle handle = m_animationData.allocateAnimationInstance(splineHandle, EInterpolationType_Linear);
        m_animationData.addDataBindingToAnimationInstance(handle, m_animationData.allocateDataBinding(m_dataBind));
        EXPECT_TRUE(m_animationData.containsAnimationInstance(handle));
        const AnimationInstance& animInstanceManaged = m_animationData.getAnimationInstance(handle);
        EXPECT_NE(AnimationInstance::InvalidInstance(), animInstanceManaged);
    }

    TEST_F(AnimationDataTest, AnimationInstanceContentsMatch)
    {
        const SplineHandle splineHandle = m_animationData.allocateSpline(m_spline);
        const AnimationInstanceHandle handle = m_animationData.allocateAnimationInstance(splineHandle, EInterpolationType_Linear);
        const DataBindHandle dataBindHandle = m_animationData.allocateDataBinding(m_dataBind);
        EXPECT_CALL(m_dataListener, onAnimationInstanceChanged(handle));
        m_animationData.addDataBindingToAnimationInstance(handle, dataBindHandle);
        const AnimationInstance& animInstanceManaged = m_animationData.getAnimationInstance(handle);
        EXPECT_EQ(splineHandle, animInstanceManaged.getSplineHandle());
        EXPECT_EQ(EInterpolationType_Linear, animInstanceManaged.getInterpolationType());
        EXPECT_EQ(dataBindHandle, *animInstanceManaged.getDataBindings().begin());
    }

    TEST_F(AnimationDataTest, allocateAnimation)
    {
        const SplineHandle splineHandle = m_animationData.allocateSpline(m_spline);
        const AnimationInstanceHandle animInstHandle = m_animationData.allocateAnimationInstance(splineHandle, EInterpolationType_Linear);
        m_animationData.addDataBindingToAnimationInstance(animInstHandle, m_animationData.allocateDataBinding(m_dataBind));

        const AnimationHandle handle = m_animationData.allocateAnimation(animInstHandle);
        EXPECT_TRUE(m_animationData.containsAnimation(handle));
        EXPECT_EQ(1u, m_animationData.getTotalAnimationCount());
    }

    TEST_F(AnimationDataTest, removeAnimation)
    {
        const SplineHandle splineHandle = m_animationData.allocateSpline(m_spline);
        const AnimationInstanceHandle animInstHandle = m_animationData.allocateAnimationInstance(splineHandle, EInterpolationType_Linear);
        m_animationData.addDataBindingToAnimationInstance(animInstHandle, m_animationData.allocateDataBinding(m_dataBind));

        const AnimationHandle handle = m_animationData.allocateAnimation(animInstHandle);
        m_animationData.removeAnimation(handle);
        EXPECT_FALSE(m_animationData.containsAnimation(handle));
    }

    TEST_F(AnimationDataTest, RemoveAnimationNotAffectingOtherAnimationsAndResources)
    {
        const SplineHandle splineHandle = m_animationData.allocateSpline(m_spline);
        AnimationInstance animInst(splineHandle, EInterpolationType_Linear);
        const DataBindHandle dataBindHandle1 = m_animationData.allocateDataBinding(m_dataBind);
        const DataBindHandle dataBindHandle2 = m_animationData.allocateDataBinding(m_dataBind);
        const AnimationInstanceHandle animInstHandle = m_animationData.allocateAnimationInstance(splineHandle, EInterpolationType_Linear);
        m_animationData.addDataBindingToAnimationInstance(animInstHandle, m_animationData.allocateDataBinding(m_dataBind));
        m_animationData.addDataBindingToAnimationInstance(animInstHandle, m_animationData.allocateDataBinding(m_dataBind));

        const AnimationHandle handle1 = m_animationData.allocateAnimation(animInstHandle);
        const AnimationHandle handle2 = m_animationData.allocateAnimation(animInstHandle);

        m_animationData.removeAnimation(handle1);
        EXPECT_FALSE(m_animationData.containsAnimation(handle1));
        EXPECT_TRUE(m_animationData.containsAnimation(handle2));
        EXPECT_TRUE(m_animationData.containsAnimationInstance(animInstHandle));
        EXPECT_TRUE(m_animationData.containsDataBinding(dataBindHandle1));
        EXPECT_TRUE(m_animationData.containsDataBinding(dataBindHandle2));
        EXPECT_TRUE(m_animationData.containsSpline(splineHandle));
    }

    TEST_F(AnimationDataTest, PauseResumeAnimation)
    {
        const SplineHandle splineHandle = m_animationData.allocateSpline(m_spline);
        const AnimationInstanceHandle animInstHandle = m_animationData.allocateAnimationInstance(splineHandle, EInterpolationType_Linear);

        const AnimationHandle handle1 = m_animationData.allocateAnimation(animInstHandle);
        const AnimationHandle handle2 = m_animationData.allocateAnimation(animInstHandle);
        const Animation& anim1 = m_animationData.getAnimation(handle1);
        const Animation& anim2 = m_animationData.getAnimation(handle2);

        EXPECT_FALSE(anim1.m_paused);
        EXPECT_FALSE(anim2.m_paused);

        EXPECT_CALL(m_dataListener, onAnimationPauseChanged(handle1, true));
        m_animationData.setAnimationPaused(handle1, true);
        EXPECT_TRUE(anim1.m_paused);
        EXPECT_FALSE(anim2.m_paused);

        EXPECT_CALL(m_dataListener, onAnimationPauseChanged(handle1, false));
        EXPECT_CALL(m_dataListener, onAnimationPauseChanged(handle2, true));
        m_animationData.setAnimationPaused(handle1, false);
        m_animationData.setAnimationPaused(handle2, true);
        EXPECT_FALSE(anim1.m_paused);
        EXPECT_TRUE(anim2.m_paused);
    }

    TEST_F(AnimationDataTest, ChangeAnimationProperties)
    {
        const SplineHandle splineHandle = m_animationData.allocateSpline(m_spline);
        const AnimationInstanceHandle animInstHandle = m_animationData.allocateAnimationInstance(splineHandle, EInterpolationType_Linear);

        const AnimationHandle handle1 = m_animationData.allocateAnimation(animInstHandle);
        const AnimationHandle handle2 = m_animationData.allocateAnimation(animInstHandle);
        const Animation& anim1 = m_animationData.getAnimation(handle1);
        const Animation& anim2 = m_animationData.getAnimation(handle2);

        EXPECT_FLOAT_EQ(1.f, anim1.m_playbackSpeed);
        EXPECT_FLOAT_EQ(1.f, anim2.m_playbackSpeed);
        EXPECT_EQ(0u, anim1.m_flags);
        EXPECT_EQ(0u, anim2.m_flags);
        EXPECT_EQ(0u, anim1.m_loopDuration);
        EXPECT_EQ(0u, anim2.m_loopDuration);

        m_animationData.setAnimationProperties(handle1, 1.f, 0u, 0u);

        EXPECT_CALL(m_dataListener, onAnimationPropertiesChanged(handle1));
        m_animationData.setAnimationProperties(handle1, 2.f, Animation::EAnimationFlags_Relative, 10u);
        EXPECT_FLOAT_EQ(2.f, anim1.m_playbackSpeed);
        EXPECT_FLOAT_EQ(1.f, anim2.m_playbackSpeed);
        EXPECT_EQ(static_cast<UInt32>(Animation::EAnimationFlags_Relative), anim1.m_flags);
        EXPECT_EQ(0u, anim2.m_flags);
        EXPECT_EQ(10u, anim1.m_loopDuration);
        EXPECT_EQ(0u, anim2.m_loopDuration);

        EXPECT_CALL(m_dataListener, onAnimationPropertiesChanged(handle2));
        m_animationData.setAnimationProperties(handle2, 2.f, Animation::EAnimationFlags_Relative, 10u);
        EXPECT_FLOAT_EQ(2.f, anim1.m_playbackSpeed);
        EXPECT_FLOAT_EQ(2.f, anim2.m_playbackSpeed);
        EXPECT_EQ(static_cast<UInt32>(Animation::EAnimationFlags_Relative), anim1.m_flags);
        EXPECT_EQ(static_cast<UInt32>(Animation::EAnimationFlags_Relative), anim2.m_flags);
        EXPECT_EQ(10u, anim1.m_loopDuration);
        EXPECT_EQ(10u, anim2.m_loopDuration);
    }

    TEST_F(AnimationDataTest, SetAnimationTimeRange)
    {
        const SplineHandle splineHandle = m_animationData.allocateSpline(m_spline);
        const AnimationInstanceHandle animInstHandle = m_animationData.allocateAnimationInstance(splineHandle, EInterpolationType_Linear);

        const AnimationHandle handle1 = m_animationData.allocateAnimation(animInstHandle);
        const AnimationHandle handle2 = m_animationData.allocateAnimation(animInstHandle);
        const Animation& anim1 = m_animationData.getAnimation(handle1);
        const Animation& anim2 = m_animationData.getAnimation(handle2);

        EXPECT_FALSE(anim1.m_startTime.isValid());
        EXPECT_FALSE(anim1.m_stopTime.isValid());
        EXPECT_FALSE(anim2.m_startTime.isValid());
        EXPECT_FALSE(anim2.m_stopTime.isValid());

        EXPECT_CALL(m_dataListener, onAnimationTimeRangeChanged(handle1));
        EXPECT_CALL(m_dataListener, preAnimationTimeRangeChange(handle1));
        m_animationData.setAnimationTimeRange(handle1, 100u, 200u);

        EXPECT_EQ(anim1.m_startTime, 100u);
        EXPECT_EQ(anim1.m_stopTime, 200u);
        EXPECT_FALSE(anim2.m_startTime.isValid());
        EXPECT_FALSE(anim2.m_stopTime.isValid());

        EXPECT_CALL(m_dataListener, onAnimationTimeRangeChanged(handle2));
        EXPECT_CALL(m_dataListener, preAnimationTimeRangeChange(handle2));
        m_animationData.setAnimationTimeRange(handle2, 100u, 200u);

        EXPECT_EQ(anim1.m_startTime, 100u);
        EXPECT_EQ(anim1.m_stopTime, 200u);
        EXPECT_EQ(anim2.m_startTime, 100u);
        EXPECT_EQ(anim2.m_stopTime, 200u);
    }

    TEST_F(AnimationDataTest, PreallocatesMemoryPoolsBasedOnSizeInformation)
    {
        const AnimationSystemSizeInformation sizeInfo(1u, 2u, 3u, 4u);
        AnimationData data(sizeInfo);

        EXPECT_EQ(sizeInfo, data.getTotalSizeInformation());
        EXPECT_EQ(1u, data.getTotalSplineCount());
        EXPECT_EQ(2u, data.getTotalDataBindCount());
        EXPECT_EQ(3u, data.getTotalAnimationInstanceCount());
        EXPECT_EQ(4u, data.getTotalAnimationCount());
    }
}
