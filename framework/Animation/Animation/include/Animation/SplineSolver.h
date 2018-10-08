//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SPLINESOLVER_H
#define RAMSES_SPLINESOLVER_H

#include "Animation/Spline.h"
#include "Animation/SplineKey.h"
#include "Animation/SplineSegment.h"
#include "Animation/SplineIterator.h"
#include "Animation/Interpolator.h"
#include "Animation/SplineSolverHelper.h"

namespace ramses_internal
{
    template <template<typename> class Key, typename EDataType>
    class SplineSolver
    {
    };

    template <typename EDataType>
    class SplineSolver<SplineKey, EDataType>
    {
    public:
        SplineSolver(const Spline<SplineKey, EDataType>& spline, const SplineIterator& splineIter, EInterpolationType interpolationType);
        EDataType getInterpolatedValue() const;

    private:
        const Spline<SplineKey, EDataType>& m_spline;
        const SplineIterator& m_splineIter;
        const EInterpolationType m_interpolationType;
    };

    template <typename EDataType>
    class SplineSolver < SplineKeyTangents, EDataType >
    {
    public:
        SplineSolver(const Spline<SplineKeyTangents, EDataType>& spline, const SplineIterator& splineIter, EInterpolationType interpolationType);
        EDataType getInterpolatedValue() const;

    private:
        const Spline<SplineKeyTangents, EDataType>& m_spline;
        const SplineIterator& m_splineIter;
        const EInterpolationType m_interpolationType;
    };

    template <typename EDataType>
    SplineSolver<SplineKey, EDataType>::SplineSolver(const Spline<SplineKey, EDataType>& spline, const SplineIterator& splineIter, EInterpolationType interpolationType)
        : m_spline(spline)
        , m_splineIter(splineIter)
        , m_interpolationType(interpolationType)
    {
    }

    template <typename EDataType>
    EDataType SplineSolver<SplineKey, EDataType>::getInterpolatedValue() const
    {
        const SplineSegment& segment = m_splineIter.getSegment();
        const SplineKey<EDataType> key1 = m_spline.getKey(segment.m_startIndex);
        const SplineKey<EDataType> key2 = m_spline.getKey(segment.m_endIndex);
        const EDataType& startValue = key1.m_value;
        const EDataType& endValue = key2.m_value;
        const Float segmentTime = m_splineIter.getSegmentLocalTime();

        switch (m_interpolationType)
        {
        case EInterpolationType_Step:
            return Interpolator::InterpolateStep(startValue, endValue, segmentTime);
        case EInterpolationType_Linear:
            return Interpolator::InterpolateLinear(startValue, endValue, segmentTime);
        default:
            assert(false);
        }

        return EDataType();
    }

    template <typename EDataType>
    SplineSolver<SplineKeyTangents, EDataType>::SplineSolver(const Spline<SplineKeyTangents, EDataType>& spline, const SplineIterator& splineIter, EInterpolationType interpolationType)
        : m_spline(spline)
        , m_splineIter(splineIter)
        , m_interpolationType(interpolationType)
    {
    }

    template <typename EDataType>
    EDataType SplineSolver<SplineKeyTangents, EDataType>::getInterpolatedValue() const
    {
        const SplineSegment& segment = m_splineIter.getSegment();
        const SplineKeyTangents<EDataType> key1 = m_spline.getKey(segment.m_startIndex);
        const SplineKeyTangents<EDataType> key2 = m_spline.getKey(segment.m_endIndex);
        const Float segmentTime = m_splineIter.getSegmentLocalTime();

        switch (m_interpolationType)
        {
        case EInterpolationType_Step:
            return Interpolator::InterpolateStep(key1.m_value, key2.m_value, segmentTime);
        case EInterpolationType_Linear:
            return Interpolator::InterpolateLinear(key1.m_value, key2.m_value, segmentTime);
        case EInterpolationType_Bezier:
        {
            const SplineTimeStamp key1Time = segment.m_startTimeStamp;
            const SplineTimeStamp key2Time = segment.m_endTimeStamp;
            return SplineSolverHelper::GetInterpolatedValueBezier(key1, key2, key1Time, key2Time, segmentTime);
        }
        default:
            assert(false);
        }

        return EDataType();
    }
}

#endif
