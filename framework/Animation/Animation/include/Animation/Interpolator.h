//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_INTERPOLATOR_H
#define RAMSES_INTERPOLATOR_H

#include "Animation/AnimatableTypeTraits.h"

namespace ramses_internal
{
    class Interpolator
    {
    public:
        template <typename EDataType>
        static EDataType InterpolateStep(const EDataType& startValue, const EDataType& endValue, Float fraction);

        template <typename EDataType>
        static EDataType InterpolateLinear(const EDataType& startValue, const EDataType& endValue, Float fraction);

        template <typename EDataType>
        static EDataType InterpolateCubicBezier(
            const EDataType& startValue,
            const EDataType& endValue,
            const EDataType& startTangent,
            const EDataType& endTangent,
            Float fraction);

        template <typename EDataType>
        static EDataType InterpolateCubicBezierCoeffs(
            const EDataType& coeffA,
            const EDataType& coeffB,
            const EDataType& coeffC,
            const EDataType& coeffD,
            Float fraction);

        template <typename EDataType>
        static void ComputeCoeffsCubicBezier(
            const EDataType& p0,
            const EDataType& p1,
            const EDataType& p2,
            const EDataType& p3,
            EDataType& coeffA,
            EDataType& coeffB,
            EDataType& coeffC,
            EDataType& coeffD);

        static Float FindFractionForGivenXOnBezierSpline(
            Float P0,
            Float P1,
            Float P2,
            Float P3,
            Float Px);
    };

    template <typename EDataType>
    EDataType Interpolator::InterpolateStep(const EDataType& startValue, const EDataType& endValue, Float fraction)
    {
        return ( fraction < 1.f ? startValue : endValue );
    }

    template <typename EDataType>
    EDataType Interpolator::InterpolateLinear(const EDataType& startValue, const EDataType& endValue, Float fraction)
    {
        using TypeTraits = AnimatableTypeTraits<EDataType>;
        const typename TypeTraits::CorrespondingFloatType valDiff = TypeTraits::ToCorrespondingFloatType(endValue) - TypeTraits::ToCorrespondingFloatType(startValue);
        const typename TypeTraits::CorrespondingFloatType result = TypeTraits::ToCorrespondingFloatType(startValue) + valDiff * fraction;
        return TypeTraits::FromCorrespondingFloatType(result);
    }

    template <typename EDataType>
    EDataType Interpolator::InterpolateCubicBezier(
        const EDataType& startValue,
        const EDataType& endValue,
        const EDataType& startTangent,
        const EDataType& endTangent,
        Float fraction)
    {
        using TypeTraits = AnimatableTypeTraits<EDataType>;
        typename TypeTraits::CorrespondingFloatType coeffA;
        typename TypeTraits::CorrespondingFloatType coeffB;
        typename TypeTraits::CorrespondingFloatType coeffC;
        typename TypeTraits::CorrespondingFloatType coeffD;
        const typename TypeTraits::CorrespondingFloatType p0f = TypeTraits::ToCorrespondingFloatType(startValue);
        const typename TypeTraits::CorrespondingFloatType p1f = TypeTraits::ToCorrespondingFloatType(endValue);
        const typename TypeTraits::CorrespondingFloatType p2f = TypeTraits::ToCorrespondingFloatType(startTangent);
        const typename TypeTraits::CorrespondingFloatType p3f = TypeTraits::ToCorrespondingFloatType(endTangent);
        Interpolator::ComputeCoeffsCubicBezier(p0f, p1f, p2f, p3f, coeffA, coeffB, coeffC, coeffD);

        const typename TypeTraits::CorrespondingFloatType ret = Interpolator::InterpolateCubicBezierCoeffs(coeffA, coeffB, coeffC, coeffD, fraction);
        return TypeTraits::FromCorrespondingFloatType(ret);
    }

    template <typename EDataType>
    EDataType Interpolator::InterpolateCubicBezierCoeffs(
        const EDataType& coeffA,
        const EDataType& coeffB,
        const EDataType& coeffC,
        const EDataType& coeffD,
        Float fraction)
    {
        const Float fraction2 = fraction * fraction;
        const Float fraction3 = fraction2 * fraction;
        return EDataType(fraction3 * coeffA + fraction2 * coeffB + fraction * coeffC + coeffD);
    }

    template <typename EDataType>
    void Interpolator::ComputeCoeffsCubicBezier(
        const EDataType& p0,
        const EDataType& p1,
        const EDataType& p2,
        const EDataType& p3,
        EDataType& coeffA,
        EDataType& coeffB,
        EDataType& coeffC,
        EDataType& coeffD)
    {
        const EDataType p0_3 = p0 * 3;
        const EDataType p1_3 = p1 * 3;
        const EDataType p2_3 = p2 * 3;

        coeffA = p3 - p2_3 + p1_3 - p0;
        coeffB = p2_3 - p1 * 6 + p0_3;
        coeffC = p1_3 - p0_3;
        coeffD = p0;
    }

    inline Float Interpolator::FindFractionForGivenXOnBezierSpline(
        Float P0,
        Float P1,
        Float P2,
        Float P3,
        Float Px)
    {
        static const Float ErrorTreshold = 1.f;

        if (Px <= P0)
        {
            return 0.f;
        }
        if (Px >= P3)
        {
            return 1.f;
        }

        Float coeffA = 0.f;
        Float coeffB = 0.f;
        Float coeffC = 0.f;
        Float coeffD = 0.f;
        Interpolator::ComputeCoeffsCubicBezier(P0, P1, P2, P3, coeffA, coeffB, coeffC, coeffD);

        Float rangeLeft = P0;
        Float rangeRight = P3;
        Float delta = 1000.f;
        const Float fracCoeff = 1.f / (P3 - P0);
        Float tempFraction = (Px - P0) * fracCoeff;

        while (delta > ErrorTreshold && (rangeRight - rangeLeft) > ErrorTreshold)
        {
            const Float tempX = (rangeLeft + rangeRight) * 0.5f;
            tempFraction = (tempX - P0) * fracCoeff;

            const Float tempResult = InterpolateCubicBezierCoeffs(coeffA, coeffB, coeffC, coeffD, tempFraction);
            delta = std::abs(tempResult - Px);

            if (tempResult > Px)
            {
                rangeRight = tempX;
            }
            else
            {
                rangeLeft = tempX;
            }
        }

        return tempFraction;
    }

    // Specializations for boolean due to unsupported arithmetic operations
    template <>
    inline bool Interpolator::InterpolateLinear<bool>(const bool& startValue, const bool& endValue, Float fraction)
    {
        return (fraction < 0.5f ? startValue : endValue);
    }

    template <>
    inline bool Interpolator::InterpolateCubicBezier<bool>(
        const bool& startValue,
        const bool& endValue,
        const bool& startTangent,
        const bool& endTangent,
        Float fraction)
    {
        UNUSED(startTangent);
        UNUSED(endTangent);
        return Interpolator::InterpolateLinear(startValue, endValue, fraction);
    }
}

#endif
