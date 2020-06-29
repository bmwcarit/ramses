//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Animation/SplineIterator.h"
#include "Animation/SplineBase.h"
#include "PlatformAbstraction/PlatformMath.h"
#include <assert.h>

namespace ramses_internal
{
    const Float SplineIterator::InvalidLocalTime = -1.f;

    SplineIterator::SplineIterator()
        : m_spline(nullptr)
        , m_timeStamp(InvalidSplineTimeStamp)
        , m_segmentLocalTime(InvalidLocalTime)
    {
    }

    void SplineIterator::setTimeStamp(SplineTimeStamp timeStamp, const SplineBase* spline, bool reverse)
    {
        if (m_spline != spline)
        {
            resetForSplineChange(spline);
        }

        if (m_spline == nullptr)
        {
            return;
        }

        if (reverse)
        {
            const SplineTimeStamp lastKeyIndex = m_spline->getNumKeys() - 1;
            const SplineTimeStamp lastKeyTimeStamp = m_spline->getTimeStamp(lastKeyIndex);
            if (lastKeyTimeStamp > timeStamp)
            {
                timeStamp = lastKeyTimeStamp - timeStamp;
            }
            else
            {
                timeStamp = 0u;
            }
        }

        if (timeStamp != m_timeStamp)
        {
            m_timeStamp = timeStamp;
            if (!m_segment.IsTimeInSegment(timeStamp))
            {
                setCurrentSegment();
            }
            resetSegmentLocalTime();
        }

        if (m_segmentLocalTime == InvalidLocalTime)
        {
            setSegmentLocalTime();
        }
    }

    void SplineIterator::setCurrentSegment()
    {
        const UInt32 numKeys = m_spline->getNumKeys();
        const SplineKeyIndex currSegmentStart = m_segment.m_startIndex;
        if (currSegmentStart != InvalidSplineKeyIndex)
        {
            m_segment = findSegmentInRange(m_timeStamp, m_segment.m_endIndex, numKeys - 1u);
        }

        if (!m_segment.IsValid())
        {
            m_segment = findSegmentInRange(m_timeStamp, 0, std::min(currSegmentStart, numKeys - 1u));
            if (!m_segment.IsValid())
            {
                m_segment = getSegmentClampedToValidRange(m_timeStamp);
            }
        }

        assert(m_segment.IsValid());
    }

    SplineSegment SplineIterator::findSegmentInRange(SplineTimeStamp timeStamp, SplineKeyIndex keyIndexFrom, SplineKeyIndex keyIndexTo) const
    {
        SplineKeyIndex keyStartIndex = keyIndexFrom;
        SplineTimeStamp keyStartTime = m_spline->getTimeStamp(keyStartIndex);
        if (keyStartTime <= timeStamp)
        {
            for (SplineKeyIndex i = keyStartIndex + 1; i <= keyIndexTo; ++i)
            {
                const SplineKeyIndex keyEndIndex = i;
                const SplineTimeStamp keyEndTime = m_spline->getTimeStamp(i);

                const SplineSegment currentSegment(keyStartIndex, keyEndIndex, keyStartTime, keyEndTime);
                if (currentSegment.IsTimeInSegment(timeStamp))
                {
                    return currentSegment;
                }

                keyStartIndex = keyEndIndex;
                keyStartTime = keyEndTime;
            }
        }

        return SplineSegment();
    }

    SplineSegment SplineIterator::getSegmentClampedToValidRange(SplineTimeStamp timeStamp)
    {
        const SplineKeyIndex firstKeyIndex = 0;
        const SplineTimeStamp rangeStartTimeStamp = m_spline->getTimeStamp(firstKeyIndex);
        if (timeStamp < rangeStartTimeStamp)
        {
            return SplineSegment(firstKeyIndex, firstKeyIndex, rangeStartTimeStamp, rangeStartTimeStamp);
        }

        const SplineKeyIndex lastKeyIndex = m_spline->getNumKeys() - 1u;
        const SplineTimeStamp rangeEndTimeStamp = m_spline->getTimeStamp(lastKeyIndex);
        if (m_timeStamp >= rangeEndTimeStamp)
        {
            return SplineSegment(lastKeyIndex, lastKeyIndex, rangeEndTimeStamp, rangeEndTimeStamp);
        }

        return SplineSegment();
    }

    void SplineIterator::setSegmentLocalTime()
    {
        const SplineTimeStamp segLength = m_segment.m_endTimeStamp - m_segment.m_startTimeStamp;
        if (segLength < std::numeric_limits<Float>::min())
        {
            m_segmentLocalTime = 0.f;
        }
        else
        {
            m_segmentLocalTime = static_cast<Float>(m_timeStamp - m_segment.m_startTimeStamp) / segLength;
        }
    }

    void SplineIterator::resetForSplineChange(const SplineBase* spline)
    {
        resetSegment();
        resetSegmentLocalTime();
        m_spline = nullptr;
        if (spline != nullptr && spline->getNumKeys() > 0u)
        {
            m_spline = spline;
        }
    }
}
