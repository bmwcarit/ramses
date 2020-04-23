//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "SplineTest.h"
#include "Animation/Interpolator.h"
#include "SplineTestUtils.h"
#include "AnimationTestTypes.h"
#include "TestRandom.h"

using namespace testing;

namespace ramses_internal
{
    TEST_F(ASpline, InitialState)
    {
        SplineVec3 spline;
        EXPECT_EQ(0u, spline.getNumKeys());
        EXPECT_EQ(InvalidSplineTimeStamp, spline.getTimeStamp(0u));
    }

    TEST_F(ASpline, AddKey)
    {
        const SplineTimeStamp timeStamp = 123u;
        const Vector3 value(1, 2, 3);
        const Vector2 tangent1(4, 5);
        const Vector2 tangent2(6, 7);
        const SplineKeyVec3 key(value, tangent1, tangent2);

        SplineVec3 spline;
        const SplineKeyIndex keyIdx = spline.setKey(timeStamp, key);

        EXPECT_EQ(1u, spline.getNumKeys());
        EXPECT_EQ(timeStamp, spline.getTimeStamp(keyIdx));
        EXPECT_EQ(value, spline.getKey(keyIdx).m_value);
        EXPECT_EQ(tangent1, spline.getKey(keyIdx).m_tangentIn);
        EXPECT_EQ(tangent2, spline.getKey(keyIdx).m_tangentOut);
    }

    TEST_F(ASpline, GetInvalidKeyTime)
    {
        const SplineTimeStamp timeStamp = 123u;
        const Vector3 value(1, 2, 3);
        const Vector2 tangent1(4, 5);
        const Vector2 tangent2(6, 7);
        const SplineKeyVec3 key(value, tangent1, tangent2);

        SplineVec3 spline;
        spline.setKey(timeStamp, key);
        const SplineKeyIndex keyIdx = spline.getNumKeys();

        EXPECT_EQ(1u, spline.getNumKeys());
        EXPECT_EQ(InvalidSplineTimeStamp, spline.getTimeStamp(keyIdx));
    }

    TEST_F(ASpline, ChangeValueOfKey)
    {
        const SplineTimeStamp timeStamp = 123u;
        const Vector3 value(1, 2, 3);
        const Vector2 tangent1(4, 5);
        const Vector2 tangent2(6, 7);
        const SplineKeyVec3 key(value, tangent1, tangent2);

        SplineVec3 spline;
        const SplineKeyIndex keyIdx = spline.setKey(timeStamp, key);

        const Vector3 newValue(4, 5, 6);
        const Vector2 newTangent1(4, 5);
        const Vector2 newTangent2(6, 7);
        const SplineKeyVec3 key2(newValue, newTangent1, newTangent2);
        const SplineKeyIndex keyIdx2 = spline.setKey(timeStamp, key2);
        EXPECT_EQ(keyIdx, keyIdx2);

        EXPECT_EQ(1u, spline.getNumKeys());
        EXPECT_EQ(timeStamp, spline.getTimeStamp(keyIdx));
        EXPECT_EQ(newValue, spline.getKey(keyIdx).m_value);
        EXPECT_EQ(newTangent1, spline.getKey(keyIdx).m_tangentIn);
        EXPECT_EQ(newTangent2, spline.getKey(keyIdx).m_tangentOut);
    }

    TEST_F(ASpline, SplineGettingValues)
    {
        SplineVec3 splineEmpty;
        EXPECT_EQ(0u, splineEmpty.getNumKeys());

        SplineInitializerVec3_Large splineInit;
        splineInit.initValuesSorted();
        splineInit.initSplineWithValues();
        SplineVec3& spline = splineInit.m_spline;

        EXPECT_EQ(1000u, spline.getNumKeys());

        for (UInt32 i = 0; i < spline.getNumKeys(); ++i)
        {
            EXPECT_EQ(splineInit.m_values[i], spline.getKey(i).m_value);
            EXPECT_EQ(splineInit.m_tangentsIn[i], spline.getKey(i).m_tangentIn);
            EXPECT_EQ(splineInit.m_tangentsOut[i], spline.getKey(i).m_tangentOut);
            EXPECT_EQ(splineInit.m_timeStamps[i], spline.getTimeStamp(i));
        }
    }

    TEST_F(ASpline, SplineKeyOrder)
    {
        SplineInitializerVec3_Large splineInit;
        splineInit.initValuesRandom();
        splineInit.initSplineWithValues();
        SplineVec3& spline = splineInit.m_spline;

        for (UInt32 i = 0; i < spline.getNumKeys() - 1; ++i)
        {
            const SplineTimeStamp timeStamp0 = spline.getTimeStamp(i);
            const SplineTimeStamp timeStamp1 = spline.getTimeStamp(i + 1);
            EXPECT_TRUE(timeStamp0 < timeStamp1);
        }
    }

    TEST_F(ASpline, RemoveKey)
    {
        const SplineTimeStamp timeStamp = 123u;
        const Vector3 value(1, 2, 3);
        const Vector2 tangent1(4, 5);
        const Vector2 tangent2(6, 7);
        const SplineKeyVec3 key(value, tangent1, tangent2);

        SplineVec3 spline;
        const SplineKeyIndex keyIdx = spline.setKey(timeStamp, key);

        SplineKeyVec3 key2(value * 2.f, tangent1, tangent2);
        const SplineKeyIndex keyIdx2 = spline.setKey(timeStamp / 2u, key2);

        EXPECT_EQ(2u, spline.getNumKeys());
        spline.removeKey(keyIdx2);

        EXPECT_EQ(1u, spline.getNumKeys());
        EXPECT_EQ(timeStamp, spline.getTimeStamp(keyIdx));
        EXPECT_EQ(value, spline.getKey(keyIdx).m_value);
        EXPECT_EQ(tangent1, spline.getKey(keyIdx).m_tangentIn);
        EXPECT_EQ(tangent2, spline.getKey(keyIdx).m_tangentOut);
    }

    TEST_F(ASpline, RemoveKeyDoesNotChangeOrder)
    {
        SplineInitializerVec3_Small splineInit;
        splineInit.initValuesRandom();
        splineInit.initSplineWithValues();
        SplineVec3& spline = splineInit.m_spline;

        while (spline.getNumKeys() > 0u)
        {
            const UInt32 numKeys = spline.getNumKeys();
            const SplineKeyIndex keyToRemove = ( numKeys > 1 ? static_cast<SplineKeyIndex>(TestRandom::Get(0u, numKeys - 1u)) : 0u );
            spline.removeKey(keyToRemove);

            for (UInt32 i = 1; i < spline.getNumKeys(); ++i)
            {
                const SplineTimeStamp timeStamp0 = spline.getTimeStamp(i - 1u);
                const SplineTimeStamp timeStamp1 = spline.getTimeStamp(i);
                EXPECT_TRUE(timeStamp0 < timeStamp1);
            }
        }
    }

    TEST_F(ASpline, RemoveAllKeys)
    {
        SplineInitializerVec3_Small splineInit;
        splineInit.initValuesRandom();
        splineInit.initSplineWithValues();
        splineInit.m_spline.removeAllKeys();
        EXPECT_EQ(0u, splineInit.m_spline.getNumKeys());
    }

    TEST_F(ASpline, ChangeValue)
    {
        SplineInitializerVec3_Small splineInit;
        splineInit.initValuesSorted();
        splineInit.initSplineWithValues();
        SplineVec3& spline = splineInit.m_spline;

        const Vector3 newVal(1, 2, 3);
        const SplineKeyIndex changeIndex = spline.getNumKeys() / 2u;
        spline.getKey(changeIndex).m_value = newVal;

        EXPECT_EQ(newVal, spline.getKey(changeIndex).m_value);
        for (UInt32 i = 0; i < spline.getNumKeys(); ++i)
        {
            if (i != changeIndex)
            {
                EXPECT_EQ(splineInit.m_values[i], spline.getKey(i).m_value);
                EXPECT_EQ(splineInit.m_tangentsIn[i], spline.getKey(i).m_tangentIn);
                EXPECT_EQ(splineInit.m_tangentsOut[i], spline.getKey(i).m_tangentOut);
            }
            EXPECT_EQ(splineInit.m_timeStamps[i], spline.getTimeStamp(i));
        }
    }
}
