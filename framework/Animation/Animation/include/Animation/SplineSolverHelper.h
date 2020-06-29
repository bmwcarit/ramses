//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SPLINESOLVERHELPER_H
#define RAMSES_SPLINESOLVERHELPER_H

#include "Animation/AnimationCommon.h"
#include "Animation/SplineKeyTangents.h"

namespace ramses_internal
{
    class SplineSolverHelper
    {
    public:
        template <typename EDataType>
        static EDataType GetInterpolatedValueBezier(
            const SplineKeyTangents<EDataType>& key1,
            const SplineKeyTangents<EDataType>& key2,
            SplineTimeStamp key1Time,
            SplineTimeStamp key2Time,
            Float segmentTime);

    private:
        static Float getInterpolatedValueBezierScalar(
            Float value1,
            Float value2,
            const Vector2& tangent1,
            const Vector2& tangent2,
            SplineTimeStamp time1,
            SplineTimeStamp time2,
            Float segmentTime);
    };

    template <typename EDataType>
    inline EDataType SplineSolverHelper::GetInterpolatedValueBezier(
        const SplineKeyTangents<EDataType>& key1,
        const SplineKeyTangents<EDataType>& key2,
        SplineTimeStamp key1Time,
        SplineTimeStamp key2Time,
        Float segmentTime)
    {
        typedef AnimatableTypeTraits<EDataType> TypeTraits;
        typedef AnimatableTypeTraits<typename TypeTraits::CorrespondingFloatType> TypeFloatTraits;

        const typename TypeTraits::CorrespondingFloatType key1f = TypeTraits::ToCorrespondingFloatType(key1.m_value);
        const typename TypeTraits::CorrespondingFloatType key2f = TypeTraits::ToCorrespondingFloatType(key2.m_value);

        EDataType result;
        for (UInt32 i = 0u; i < TypeTraits::NumComponents; ++i)
        {
            const Float key1compf = TypeFloatTraits::GetComponent(key1f, EVectorComponent(i));
            const Float key2compf = TypeFloatTraits::GetComponent(key2f, EVectorComponent(i));
            const Float interpolatedValue = getInterpolatedValueBezierScalar(key1compf, key2compf, key1.m_tangentOut, key2.m_tangentIn, key1Time, key2Time, segmentTime);
            TypeTraits::SetComponent(result, static_cast<typename TypeTraits::ComponentType>(interpolatedValue), EVectorComponent(i));
        }

        return result;
    }

    // Specialization for boolean due to unsupported arithmetic operations
    template <>
    inline bool SplineSolverHelper::GetInterpolatedValueBezier(
        const SplineKeyTangents<bool>& key1,
        const SplineKeyTangents<bool>& key2,
        SplineTimeStamp key1Time,
        SplineTimeStamp key2Time,
        Float segmentTime)
    {
        UNUSED(key1Time);
        UNUSED(key2Time);
        return Interpolator::InterpolateLinear(key1.m_value, key2.m_value, segmentTime);
    }

    inline Float SplineSolverHelper::getInterpolatedValueBezierScalar(
        Float value1,
        Float value2,
        const Vector2& tangent1,
        const Vector2& tangent2,
        SplineTimeStamp time1,
        SplineTimeStamp time2,
        Float segmentTime)
    {
        const Vector2 p0(static_cast<Float>(time1), value1);
        const Vector2 p3(static_cast<Float>(time2), value2);
        const Vector2 p1 = p0 + tangent1;
        const Vector2 p2 = p3 + tangent2;

        const Float segmentTimeInMS = p0.x + (p3.x - p0.x) * segmentTime;
        const Float segmentFraction = Interpolator::FindFractionForGivenXOnBezierSpline(p0.x, p1.x, p2.x, p3.x, segmentTimeInMS);
        return Interpolator::InterpolateCubicBezier(p0.y, p1.y, p2.y, p3.y, segmentFraction);
    }
}

#endif
