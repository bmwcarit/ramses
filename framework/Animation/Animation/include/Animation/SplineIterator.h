//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SPLINEITERATOR_H
#define RAMSES_SPLINEITERATOR_H

#include "Animation/AnimationCommon.h"
#include "Animation/SplineSegment.h"

namespace ramses_internal
{
    class SplineBase;

    class SplineIterator
    {
    public:
        SplineIterator();

        void setTimeStamp(SplineTimeStamp timeStamp, const SplineBase* spline, Bool reverse = false);
        SplineTimeStamp getTimeStamp() const;
        const SplineSegment& getSegment() const;
        Float getSegmentLocalTime() const;

        void resetSegment();
        void resetSegmentLocalTime();

        static const Float InvalidLocalTime;

    private:
        void setCurrentSegment();
        SplineSegment findSegmentInRange(SplineTimeStamp timeStamp, SplineKeyIndex keyIndexFrom, SplineKeyIndex keyIndexTo) const;
        SplineSegment getSegmentClampedToValidRange(SplineTimeStamp timeStamp);
        void setSegmentLocalTime();
        void resetForSplineChange(const SplineBase* spline);

        const SplineBase* m_spline;
        SplineTimeStamp m_timeStamp;
        SplineSegment m_segment;
        Float m_segmentLocalTime;
    };

    inline SplineTimeStamp SplineIterator::getTimeStamp() const
    {
        return m_timeStamp;
    }

    inline const SplineSegment& SplineIterator::getSegment() const
    {
        return m_segment;
    }

    inline Float SplineIterator::getSegmentLocalTime() const
    {
        return m_segmentLocalTime;
    }

    inline void SplineIterator::resetSegment()
    {
        m_segment = SplineSegment();
        m_timeStamp = InvalidSplineTimeStamp;
    }

    inline void SplineIterator::resetSegmentLocalTime()
    {
        m_segmentLocalTime = InvalidLocalTime;
    }
}

#endif
