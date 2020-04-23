//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_SPLINETESTUTILS_H
#define RAMSES_SPLINETESTUTILS_H

#include "framework_common_gmock_header.h"
#include "Math3d/Vector2.h"
#include "Math3d/Vector3.h"
#include "Math3d/Vector4.h"
#include "AnimationTestUtils.h"
#include "Animation/Spline.h"
#include "Animation/SplineKeyTangents.h"
#include "Animation/SplineIterator.h"
#include "TestRandom.h"

namespace ramses_internal
{
    template <typename EDataType>
    struct SplineSegmentData
    {
        EDataType m_startValue;
        EDataType m_endValue;
        Vector2 m_tangent1;
        Vector2 m_tangent2;
        Float m_segmentTime;
    };

    template <typename EDataType, UInt NumKeys>
    class SplineInitializer
    {
    public:
        void initValuesRandom();
        void initValuesSorted();
        void initValues(const SplineTimeStamp timeStamps[NumKeys], const EDataType vecs[NumKeys]);
        void initSplineWithValues();
        void setTimeStamp(SplineTimeStamp timeStamp);

        static EDataType GetRandom();

        Spline<SplineKeyTangents, EDataType> m_spline;
        SplineTimeStamp m_timeStamps[NumKeys];
        EDataType m_values[NumKeys];
        Vector2 m_tangentsIn[NumKeys];
        Vector2 m_tangentsOut[NumKeys];

        SplineIterator m_iterator;
        SplineSegmentData<EDataType> m_segmentData;
    };

    template <typename EDataType, UInt NumKeys>
    void SplineInitializer<EDataType, NumKeys>::initValuesRandom()
    {
        for (UInt i = 0; i < NumKeys; ++i)
        {
            m_timeStamps[i] = static_cast<SplineTimeStamp>(TestRandom::Get(1u, 1000u));
            m_values[i] = AnimationTestUtils::GetRandom<EDataType>();
            m_tangentsIn[i] = AnimationTestUtils::GetRandom<Vector2>();
            m_tangentsOut[i] = AnimationTestUtils::GetRandom<Vector2>();
        }
    }

    template <typename EDataType, UInt NumKeys>
    void SplineInitializer<EDataType, NumKeys>::initValuesSorted()
    {
        for (UInt i = 0; i < NumKeys; ++i)
        {
            m_timeStamps[i] = static_cast<SplineTimeStamp>((i + 1) * 100 + TestRandom::Get(0, 100u));
            m_values[i] = AnimationTestUtils::GetRandom<EDataType>();
            m_tangentsIn[i] = AnimationTestUtils::GetRandom<Vector2>();
            m_tangentsOut[i] = AnimationTestUtils::GetRandom<Vector2>();
        }
    }

    template <typename EDataType, UInt NumKeys>
    void SplineInitializer<EDataType, NumKeys>::initSplineWithValues()
    {
        m_spline = Spline<SplineKeyTangents, EDataType>();
        for (UInt i = 0; i < NumKeys; ++i)
        {
            SplineKeyTangents<EDataType> key;
            key.m_value = m_values[i];
            key.m_tangentIn = m_tangentsIn[i];
            key.m_tangentOut = m_tangentsOut[i];
            m_spline.setKey(m_timeStamps[i], key);
        }
    }

    template <typename EDataType, UInt NumKeys>
    void SplineInitializer<EDataType, NumKeys>::setTimeStamp(SplineTimeStamp timeStamp)
    {
        m_iterator.setTimeStamp(timeStamp, &m_spline);
        const SplineSegment& segment = m_iterator.getSegment();
        const SplineKeyTangents<EDataType> key1 = m_spline.getKey(segment.m_startIndex);
        const SplineKeyTangents<EDataType> key2 = m_spline.getKey(segment.m_endIndex);
        m_segmentData.m_segmentTime = m_iterator.getSegmentLocalTime();
        m_segmentData.m_startValue = key1.m_value;
        m_segmentData.m_endValue = key2.m_value;
        m_segmentData.m_tangent1 = key1.m_tangentOut;
        m_segmentData.m_tangent2 = key2.m_tangentIn;
    }
}

#endif
