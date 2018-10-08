//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_ANIMATIONTESTUTILS_H
#define RAMSES_ANIMATIONTESTUTILS_H

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "AnimationDataBindTestUtils.h"
#include "AnimationTestTypes.h"
#include "Animation/AnimationCommon.h"
#include "Animation/AnimationData.h"
#include "Animation/AnimationLogicListener.h"
#include "Animation/SplineKeyTangents.h"
#include "TestRandom.h"

namespace ramses_internal
{
    class AnimationTestUtils
    {
    public:
        static const UInt32 RANDOM_RANGE = 1000u;

        template <typename EDataType>
        static EDataType GetRandom();

        template <typename EDataType>
        static Bool AreEqual(const EDataType& a, const EDataType& b);
    };

    template <typename EDataType>
    inline EDataType AnimationTestUtils::GetRandom()
    {
        return EDataType(TestRandom::Get(0u, RANDOM_RANGE));
    }

    template <>
    inline Bool AnimationTestUtils::GetRandom<Bool>()
    {
        return GetRandom<UInt32>() > (RANDOM_RANGE / 2u);
    }

    template <>
    inline Float AnimationTestUtils::GetRandom<Float>()
    {
        return Float(GetRandom<UInt32>()) - Float(RANDOM_RANGE / 2u);
    }

    template <>
    inline Vector2 AnimationTestUtils::GetRandom<Vector2>()
    {
        return Vector2(GetRandom<Float>(), GetRandom<Float>());
    }

    template <>
    inline Vector3 AnimationTestUtils::GetRandom<Vector3>()
    {
        return Vector3(GetRandom<Float>(), GetRandom<Float>(), GetRandom<Float>());
    }

    template <>
    inline Vector4 AnimationTestUtils::GetRandom<Vector4>()
    {
        return Vector4(GetRandom<Float>(), GetRandom<Float>(), GetRandom<Float>(), GetRandom<Float>());
    }

    template <typename EDataType>
    inline Bool AnimationTestUtils::AreEqual(const EDataType& a, const EDataType& b)
    {
        return a == b;
    }

    template <>
    inline Bool AnimationTestUtils::AreEqual(const Float& a, const Float& b)
    {
        const Float fDelta = PlatformMath::Abs(a - b);
        if (fDelta < 1e-5f)
        {
            return true;
        }

        Float relativeError = 0.f;
        if (PlatformMath::Abs(b) > PlatformMath::Abs(a))
        {
            relativeError = PlatformMath::Abs((a - b) / b);
        }
        else
        {
            relativeError = PlatformMath::Abs((a - b) / a);
        }
        return relativeError < 0.0001f;
    }

    template <>
    inline Bool AnimationTestUtils::AreEqual(const Double& a, const Double& b)
    {
        const Double dDelta = PlatformMath::Abs(a - b);
        if (dDelta < 1e-9)
        {
            return true;
        }

        Double relativeError = 0.0;
        if (PlatformMath::Abs(b) > PlatformMath::Abs(a))
        {
            relativeError = PlatformMath::Abs((a - b) / b);
        }
        else
        {
            relativeError = PlatformMath::Abs((a - b) / a);
        }
        return relativeError < 0.00001;
    }

    template <>
    inline Bool AnimationTestUtils::AreEqual<Vector2>(const Vector2& vecA, const Vector2& vecB)
    {
        return AreEqual(vecA.x, vecB.x)
            && AreEqual(vecA.y, vecB.y);
    }

    template <>
    inline Bool AnimationTestUtils::AreEqual<Vector3>(const Vector3& vecA, const Vector3& vecB)
    {
        return AreEqual(vecA.x, vecB.x)
            && AreEqual(vecA.y, vecB.y)
            && AreEqual(vecA.z, vecB.z);
    }

    template <>
    inline Bool AnimationTestUtils::AreEqual<Vector4>(const Vector4& vecA, const Vector4& vecB)
    {
        return AreEqual(vecA.x, vecB.x)
            && AreEqual(vecA.y, vecB.y)
            && AreEqual(vecA.z, vecB.z)
            && AreEqual(vecA.w, vecB.w);
    }

    class MockAnimationDataListener : public AnimationDataListener
    {
    public:
        MockAnimationDataListener();
        virtual ~MockAnimationDataListener();
        MOCK_METHOD1(preAnimationTimeRangeChange, void(AnimationHandle handle));
        MOCK_METHOD1(onAnimationTimeRangeChanged, void(AnimationHandle handle));
        MOCK_METHOD2(onAnimationPauseChanged, void(AnimationHandle handle, Bool pause));
        MOCK_METHOD1(onAnimationPropertiesChanged, void(AnimationHandle handle));
        MOCK_METHOD1(onAnimationInstanceChanged, void(AnimationInstanceHandle handle));
        MOCK_METHOD1(onSplineChanged, void(SplineHandle handle));
    };

    class MockAnimationStateListener : public AnimationLogicListener
    {
    public:
        MockAnimationStateListener();
        virtual ~MockAnimationStateListener();
        MOCK_METHOD1(onAnimationStarted, void(AnimationHandle handle));
        MOCK_METHOD1(onAnimationFinished, void(AnimationHandle handle));
        MOCK_METHOD1(onAnimationPaused, void(AnimationHandle handle));
        MOCK_METHOD1(onAnimationResumed, void(AnimationHandle handle));
        MOCK_METHOD1(onAnimationPropertiesChanged, void(AnimationHandle handle));
        MOCK_METHOD1(onAnimationSplineDataChanged, void(AnimationHandle handle));
        MOCK_METHOD1(onTimeChanged, void(const AnimationTime& time));
    };

    class AnimationTest : public testing::Test
    {
    public:
        AnimationTest()
        {
        }

        virtual void init()
        {
            const SplineHandle splineHandle = initSpline(m_animationData);

            DataBindType dataBind1(m_container, 0, EDataBindAccessorType_Handles_1);
            DataBindType dataBind2(m_container, 1, EDataBindAccessorType_Handles_1);
            const DataBindHandle dataBindHandle1 = m_animationData.allocateDataBinding(dataBind1);
            const DataBindHandle dataBindHandle2 = m_animationData.allocateDataBinding(dataBind2);

            m_animationInstanceHandle = m_animationData.allocateAnimationInstance(splineHandle, EInterpolationType_Linear);
            m_animationData.addDataBindingToAnimationInstance(m_animationInstanceHandle, dataBindHandle1);
            m_animationData.addDataBindingToAnimationInstance(m_animationInstanceHandle, dataBindHandle2);
        }

    protected:
        virtual SplineHandle initSpline(AnimationData& resMgr)
        {
            Spline<SplineKeyTangents, Vector3> spline;
            SplineKeyVec3 key1(Vector3(1, 2, 3), Vector2(1, 1), Vector2(9, 9));
            SplineKeyVec3 key2(Vector3(4, 5, 6), Vector2(1, 1), Vector2(9, 9));
            SplineKeyVec3 key3(Vector3(7, 8, 9), Vector2(1, 1), Vector2(9, 9));
            spline.setKey(10u, key1);
            spline.setKey(20u, key2);
            spline.setKey(30u, key3);
            return resMgr.allocateSpline(spline);
        }

        AnimationInstanceHandle m_animationInstanceHandle;
        ContainerType m_container;

        AnimationData m_animationData;
        testing::NiceMock<MockAnimationStateListener> m_stateListener;
        testing::NiceMock<MockAnimationDataListener> m_dataListener;
    };
}

#endif
