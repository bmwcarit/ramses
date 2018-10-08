//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#include "PlatformAbstraction/PlatformConsole.h"
#include "SplineSolverTest.h"
#include "Animation/Interpolator.h"
#include "Animation/SplineSolver.h"
#include "AnimationTestTypes.h"
#include "SplineTestUtils.h"
#include "TestRandom.h"

using namespace testing;

namespace ramses_internal
{
    // Step
    TEST_F(ASpline, SplineSolverStepOutOfRange)
    {
        splineSolverOutOfRange(EInterpolationType_Step);
    }

    TEST_F(ASpline, SplineSolverStepAtKeys)
    {
        splineSolverValuesAtKeys(EInterpolationType_Step);
    }

    TEST_F(ASpline, SplineSolverStepInterpolateTypes)
    {
        splineSolverInterpolateTypesTest(EInterpolationType_Step);
    }

    TEST_F(ASpline, DISABLED_SplineSolverStepVisualTest)
    {
        splineSolverVisualTest(EInterpolationType_Step);
    }

    // Linear
    TEST_F(ASpline, SplineSolverLinearOutOfRange)
    {
        splineSolverOutOfRange(EInterpolationType_Linear);
    }

    TEST_F(ASpline, SplineSolverLinearAtKeys)
    {
        splineSolverValuesAtKeys(EInterpolationType_Linear);
    }

    TEST_F(ASpline, SplineSolverLinearInterpolateTypes)
    {
        splineSolverInterpolateTypesTest(EInterpolationType_Linear);
    }

    TEST_F(ASpline, DISABLED_SplineSolverLinearVisualTest)
    {
        splineSolverVisualTest(EInterpolationType_Linear);
    }

    // Bezier
    TEST_F(ASpline, SplineSolverBezierOutOfRange)
    {
        splineSolverOutOfRange(EInterpolationType_Bezier);
    }

    TEST_F(ASpline, SplineSolverBezierAtKeys)
    {
        splineSolverValuesAtKeys(EInterpolationType_Bezier);
    }

    TEST_F(ASpline, DISABLED_SplineSolverBezierInterpolateTypes)
    {
        splineSolverInterpolateTypesTest(EInterpolationType_Bezier);
    }

    TEST_F(ASpline, DISABLED_SplineSolverBezierVisualTest)
    {
        splineSolverVisualTest(EInterpolationType_Bezier);
    }

    void ASpline::splineSolverValuesAtKeys(EInterpolationType interpolationType)
    {
        SplineInitializerVec3_Medium splineInit;
        splineInit.initValuesRandom();
        splineInit.initSplineWithValues();
        const SplineVec3& spline = splineInit.m_spline;
        SplineSolverVec3 solver(spline, splineInit.m_iterator, interpolationType);

        for (SplineKeyIndex i = 0; i < spline.getNumKeys(); ++i)
        {
            const Vector3& keyValue = spline.getKey(i).m_value;
            const SplineTimeStamp timeStamp = spline.getTimeStamp(i);
            splineInit.setTimeStamp(timeStamp);
            const Vector3& value = solver.getInterpolatedValue();
            EXPECT_TRUE(AnimationTestUtils::AreEqual(keyValue, value));
        }
    }

    void ASpline::splineSolverOutOfRange(EInterpolationType interpolationType)
    {
        SplineInitializer<Vector3, 1> splineInit;
        splineInit.initValuesSorted();
        splineInit.initSplineWithValues();
        const SplineVec3& spline = splineInit.m_spline;
        SplineSolverVec3 solver(spline, splineInit.m_iterator, interpolationType);

        // get first key value for time before start
        const SplineTimeStamp firstKeyTimeStamp = spline.getTimeStamp(0);
        splineInit.setTimeStamp(firstKeyTimeStamp - 1u);
        EXPECT_EQ(splineInit.m_values[0], solver.getInterpolatedValue());

        // get last key value for time after end
        const SplineKeyIndex lastKeyIndex = spline.getNumKeys() - 1;
        const SplineTimeStamp lastKeyTimeStamp = spline.getTimeStamp(lastKeyIndex);
        splineInit.setTimeStamp(lastKeyTimeStamp + 1u);
        EXPECT_EQ(splineInit.m_values[lastKeyIndex], solver.getInterpolatedValue());
    }

    void ASpline::splineSolverInterpolateTypesTest(EInterpolationType interpolationType)
    {
        splineSolverInterpolateTest<Int32>(interpolationType, "Int32");
        splineSolverInterpolateTest<UInt32>(interpolationType, "UInt32");
        splineSolverInterpolateTest<UInt64>(interpolationType, "UInt64");
        splineSolverInterpolateTest<Float>(interpolationType, "Float");
        splineSolverInterpolateTest<Double>(interpolationType, "Double");

        if (interpolationType != EInterpolationType_Bezier)
        {
            // Too complex to evaluate without using solver
            splineSolverInterpolateTest<Bool>(interpolationType, "Bool");
            splineSolverInterpolateTest<Vector2>(interpolationType, "Vector2");
            splineSolverInterpolateTest<Vector3>(interpolationType, "Vector3");
            splineSolverInterpolateTest<Vector4>(interpolationType, "Vector4");
        }
    }

    // Helpers to be able to compile test for certain type instantiations
    template <typename T>
    Float ToFloat(const T& val)
    {
        return Float(val);
    }
    template <>
    Float ToFloat<Vector2>(const Vector2& val)
    {
        return val.x;
    }
    template <>
    Float ToFloat<Vector3>(const Vector3& val)
    {
        return val.x;
    }
    template <>
    Float ToFloat<Vector4>(const Vector4& val)
    {
        return val.x;
    }
    template <typename T>
    T FromFloat(Float val)
    {
        return T(val);
    }
    template <>
    Bool FromFloat(Float val)
    {
        return val > 0.5f;
    }

    template <typename EDataType>
    void ASpline::splineSolverInterpolateTest(EInterpolationType interpolationType, const char* typeName)
    {
        SCOPED_TRACE(typeName);

        SplineInitializer<EDataType, 100> splineInit;
        splineInit.initValuesRandom();
        splineInit.initSplineWithValues();
        const Spline<SplineKeyTangents, EDataType>& spline = splineInit.m_spline;
        SplineSolver<SplineKeyTangents, EDataType> solver(spline, splineInit.m_iterator, interpolationType);

        for (UInt32 seg = 0; seg < spline.getNumKeys() - 1u; ++seg)
        {
            const SplineKeyTangents<EDataType>& keyStart = spline.getKey(seg);
            const SplineKeyTangents<EDataType>& keyEnd = spline.getKey(seg + 1u);

            const SplineTimeStamp segStart = spline.getTimeStamp(seg);
            const SplineTimeStamp segEnd = spline.getTimeStamp(seg + 1u);
            SplineTimeStamp timeStampRand = static_cast<SplineTimeStamp>(TestRandom::Get(segStart, segEnd));
            // Force compiler to store the value in a Float variable. Otherwise the compiler can store the value
            // in a machine register resulting in different precision calculations
            const volatile Float localTimeRatio = Float(timeStampRand - segStart) / Float(segEnd - segStart);

            const EDataType& valueStart = keyStart.m_value;
            const EDataType& valueEnd = keyEnd.m_value;
            EDataType expectedValue(0);
            switch (interpolationType)
            {
            case EInterpolationType_Step:
                expectedValue = Interpolator::InterpolateStep(valueStart, valueEnd, localTimeRatio);
                break;
            case EInterpolationType_Linear:
                expectedValue = Interpolator::InterpolateLinear(valueStart, valueEnd, localTimeRatio);
                break;
            case EInterpolationType_Bezier:
            {
                const Vector2 p0(ToFloat(segStart), ToFloat(valueStart));
                const Vector2 p3(ToFloat(segEnd), ToFloat(valueEnd));
                const Vector2 p1 = p0 + keyStart.m_tangentOut;
                const Vector2 p2 = p3 + keyEnd.m_tangentIn;

                const Float segmentTimeInMS = p0.x + (p3.x - p0.x) * localTimeRatio;
                const Float segmentFraction = Interpolator::FindFractionForGivenXOnBezierSpline(p0.x, p1.x, p2.x, p3.x, segmentTimeInMS);
                expectedValue = FromFloat<EDataType>(Interpolator::InterpolateCubicBezier(p0.y, p1.y, p2.y, p3.y, segmentFraction));
            }
                break;
            default:
                EXPECT_TRUE(false);
                break;
            }

            splineInit.setTimeStamp(timeStampRand);
            const EDataType interpolatedValue = solver.getInterpolatedValue();
            EXPECT_TRUE(AnimationTestUtils::AreEqual(expectedValue, interpolatedValue));
        }
    }

    void ASpline::splineSolverVisualTest(EInterpolationType interpolationType)
    {
        static const UInt32 WIDTH = 80u;
        static const UInt32 HEIGHT = 40u;
        static const Float TANGENT_MAG = 20.f;
        Bool matrix[WIDTH + 1u][HEIGHT + 1u];
        PlatformMemory::Set(matrix, 0, sizeof(matrix));

        Spline<SplineKeyTangents, Int32> spline;

        SplineKeyTangents<Int32> key1;
        key1.m_value = 0;
        key1.m_tangentIn = Vector2(0);
        key1.m_tangentOut = Vector2(0.f, TANGENT_MAG);
        spline.setKey(0u, key1);

        SplineKeyTangents<Int32> key2;
        key2.m_value = HEIGHT;
        key2.m_tangentIn = Vector2(-2.f * TANGENT_MAG, 0.f);
        key2.m_tangentOut = Vector2(0.f, -TANGENT_MAG);
        spline.setKey(WIDTH / 2u, key2);

        SplineKeyTangents<Int32> key3;
        key3.m_value = 0;
        key3.m_tangentIn = Vector2(-2.f * TANGENT_MAG, 0.f);
        key3.m_tangentOut = Vector2(0);
        spline.setKey(WIDTH, key3);

        SplineIterator iter;
        SplineSolver<SplineKeyTangents, Int32> solver(spline, iter, interpolationType);

        for (UInt32 i = 0u; i <= WIDTH; ++i)
        {
            iter.setTimeStamp(i, &spline);
            const Int32 value = solver.getInterpolatedValue();
            if (value <= Int32(HEIGHT))
            {
                matrix[i][value] = true;
            }
        }

        char line[WIDTH + 3u];
        for (UInt32 h = 0u; h <= HEIGHT; ++h)
        {
            for (UInt32 w = 0u; w <= WIDTH; ++w)
            {
                line[w] = (matrix[w][h] ? 'X' : '.');
            }
            line[WIDTH + 1u] = '\n';
            line[WIDTH + 2u] = '\0';
            Console::Print(line);
        }
    }
}
