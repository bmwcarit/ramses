//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SPLINESEGMENT_H
#define RAMSES_SPLINESEGMENT_H

#include "Animation/AnimationCommon.h"

namespace ramses_internal
{
    class SplineSegment
    {
    public:
        explicit SplineSegment(SplineKeyIndex startIndex = InvalidSplineKeyIndex, SplineKeyIndex endIndex = InvalidSplineKeyIndex, SplineTimeStamp startTime = InvalidSplineTimeStamp, SplineTimeStamp endTime = InvalidSplineTimeStamp);

        bool IsValid() const;
        bool IsTimeInSegment(SplineTimeStamp timeStamp) const;

        SplineKeyIndex m_startIndex;
        SplineKeyIndex m_endIndex;
        SplineTimeStamp m_startTimeStamp;
        SplineTimeStamp m_endTimeStamp;
    };

    inline SplineSegment::SplineSegment(SplineKeyIndex startIndex, SplineKeyIndex endIndex, SplineTimeStamp startTime, SplineTimeStamp endTime)
        : m_startIndex(startIndex)
        , m_endIndex(endIndex)
        , m_startTimeStamp(startTime)
        , m_endTimeStamp(endTime)
    {
    }

    inline bool SplineSegment::IsValid() const
    {
        return (m_startIndex != InvalidSplineKeyIndex) && (m_endIndex != InvalidSplineKeyIndex);
    }

    inline bool SplineSegment::IsTimeInSegment(SplineTimeStamp timeStamp) const
    {
        return (m_startTimeStamp <= timeStamp) && (m_endTimeStamp > timeStamp);
    }
}

#endif
