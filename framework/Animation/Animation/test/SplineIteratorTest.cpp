//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "SplineIteratorTest.h"
#include "TestRandom.h"

namespace ramses_internal
{
    TEST_F(ASplineIterator, InitialState)
    {
        SplineIterator iter;
        EXPECT_EQ(iter.getTimeStamp(), InvalidSplineTimeStamp);
        EXPECT_EQ(iter.getSegment().m_startIndex, InvalidSplineKeyIndex);
        EXPECT_EQ(iter.getSegment().m_endIndex, InvalidSplineKeyIndex);
        EXPECT_EQ(iter.getSegment().m_startTimeStamp, InvalidSplineTimeStamp);
        EXPECT_EQ(iter.getSegment().m_endTimeStamp, InvalidSplineTimeStamp);
        EXPECT_EQ(iter.getSegmentLocalTime(), SplineIterator::InvalidLocalTime);
    }

    TEST_F(ASplineIterator, StateBeforeSplineStart)
    {
        stateBeforeSplineStart();
    }

    TEST_F(ASplineIterator, StateAfterSplineStart)
    {
        stateAfterSplineStart();
    }

    TEST_F(ASplineIterator, StateAtSplineKeys)
    {
        stateAtSplineKeys();
    }

    TEST_F(ASplineIterator, StateInBetweenSplineKeys)
    {
        stateInBetweenSplineKeys();
    }

    TEST_F(ASplineIterator, SplineSwapDoesntInvalidateIterator)
    {
        SplineVec3 spline1;
        SplineKeyVec3 key1_1;
        SplineKeyVec3 key1_2;
        SplineKeyVec3 key1_3;
        spline1.setKey(100u, key1_1);
        spline1.setKey(200u, key1_2);
        spline1.setKey(300u, key1_3);

        SplineVec3 spline2;
        SplineKeyVec3 key2_1;
        SplineKeyVec3 key2_2;
        spline2.setKey(100u, key2_1);
        spline2.setKey(300u, key2_2);

        SplineIterator iter;
        iter.setTimeStamp(250u, &spline1);
        EXPECT_EQ(1u, iter.getSegment().m_startIndex);
        EXPECT_EQ(2u, iter.getSegment().m_endIndex);

        iter.setTimeStamp(260u, &spline2);
        EXPECT_EQ(0u, iter.getSegment().m_startIndex);
        EXPECT_EQ(1u, iter.getSegment().m_endIndex);
        EXPECT_EQ(100u, iter.getSegment().m_startTimeStamp);
        EXPECT_EQ(300u, iter.getSegment().m_endTimeStamp);
    }

    TEST_F(ASplineIterator, SplineChangeDoesntInvalidateIterator)
    {
        SplineVec3 spline;
        SplineKeyVec3 key1;
        SplineKeyVec3 key2;
        SplineKeyVec3 key3;
        spline.setKey(100u, key1);
        spline.setKey(200u, key2);
        spline.setKey(300u, key3);

        SplineIterator iter;
        iter.setTimeStamp(250u, &spline);
        EXPECT_EQ(1u, iter.getSegment().m_startIndex);
        EXPECT_EQ(2u, iter.getSegment().m_endIndex);

        spline.removeKey(1u);
        iter.resetSegment();
        iter.resetSegmentLocalTime();

        iter.setTimeStamp(260u, &spline);
        EXPECT_EQ(0u, iter.getSegment().m_startIndex);
        EXPECT_EQ(1u, iter.getSegment().m_endIndex);
        EXPECT_EQ(100u, iter.getSegment().m_startTimeStamp);
        EXPECT_EQ(300u, iter.getSegment().m_endTimeStamp);

        const Float localTime = static_cast<Float>(260u - 100u) / (300u - 100u);
        EXPECT_FLOAT_EQ(localTime, iter.getSegmentLocalTime());
    }

    TEST_F(ASplineIterator, RemoveKeysDontInvalidateIterator)
    {
        SplineVec3& spline = m_splineInit.m_spline;
        while (spline.getNumKeys() > 0u)
        {
            stateInBetweenSplineKeys();
            stateAtSplineKeys();
            stateAfterSplineStart();
            stateBeforeSplineStart();

            const UInt32 numKeys = spline.getNumKeys();
            const SplineKeyIndex keyToRemove = (numKeys > 1 ? static_cast<SplineKeyIndex>(TestRandom::Get(0u, numKeys - 1u)) : 0u);
            spline.removeKey(keyToRemove);
        }
    }

    TEST_F(ASplineIterator, ReverseIterator)
    {
        const SplineTimeStamp lastKeyTimeStamp = m_splineInit.m_spline.getTimeStamp(m_splineInit.m_spline.getNumKeys() - 1);

        SplineIterator& iter = m_splineInit.m_iterator;
        iter.setTimeStamp(0u, &m_splineInit.m_spline, true);
        EXPECT_EQ(lastKeyTimeStamp, iter.getTimeStamp());

        iter.setTimeStamp(lastKeyTimeStamp, &m_splineInit.m_spline, true);
        EXPECT_EQ(0u, iter.getTimeStamp());

        iter.setTimeStamp(lastKeyTimeStamp + 1, &m_splineInit.m_spline, true);
        EXPECT_EQ(0u, iter.getTimeStamp());
    }

    void ASplineIterator::stateBeforeSplineStart()
    {
        m_splineInit.setTimeStamp(0u);
        const SplineIterator& iter = m_splineInit.m_iterator;
        EXPECT_EQ(0u, iter.getTimeStamp());

        const SplineVec3& spline = m_splineInit.m_spline;
        const SplineSegment& segment = iter.getSegment();
        EXPECT_EQ(0u, segment.m_startIndex);
        EXPECT_EQ(0u, segment.m_endIndex);
        EXPECT_EQ(spline.getTimeStamp(0u), segment.m_startTimeStamp);
        EXPECT_EQ(spline.getTimeStamp(0u), segment.m_endTimeStamp);
        EXPECT_FLOAT_EQ(0.f, iter.getSegmentLocalTime());
    }

    void ASplineIterator::stateAfterSplineStart()
    {
        const SplineIterator& iter = m_splineInit.m_iterator;
        const SplineVec3& spline = m_splineInit.m_spline;
        const SplineSegment& segment = iter.getSegment();

        const SplineKeyIndex lastKeyIdx = spline.getNumKeys() - 1u;
        const SplineTimeStamp beyondEndTime = spline.getTimeStamp(lastKeyIdx) + 1u;
        m_splineInit.setTimeStamp(beyondEndTime);
        EXPECT_EQ(beyondEndTime, iter.getTimeStamp());

        EXPECT_EQ(lastKeyIdx, segment.m_startIndex);
        EXPECT_EQ(lastKeyIdx, segment.m_endIndex);
        EXPECT_EQ(spline.getTimeStamp(lastKeyIdx), segment.m_startTimeStamp);
        EXPECT_EQ(spline.getTimeStamp(lastKeyIdx), segment.m_endTimeStamp);
        EXPECT_FLOAT_EQ(0.f, iter.getSegmentLocalTime());
    }

    void ASplineIterator::stateAtSplineKeys()
    {
        const SplineIterator& iter = m_splineInit.m_iterator;
        const SplineVec3& spline = m_splineInit.m_spline;
        const SplineSegment& segment = iter.getSegment();

        const UInt32 numKeys = spline.getNumKeys();
        for (SplineKeyIndex keyIdx = 0u; keyIdx < numKeys; ++keyIdx)
        {
            const SplineTimeStamp keyTime = spline.getTimeStamp(keyIdx);
            m_splineInit.setTimeStamp(keyTime);

            const SplineKeyIndex endKeyIdx = min(keyIdx + 1u, numKeys - 1u);
            EXPECT_EQ(keyIdx, segment.m_startIndex);
            EXPECT_EQ(endKeyIdx, segment.m_endIndex);
            EXPECT_EQ(spline.getTimeStamp(keyIdx), segment.m_startTimeStamp);
            EXPECT_EQ(spline.getTimeStamp(endKeyIdx), segment.m_endTimeStamp);
            EXPECT_FLOAT_EQ(0.f, iter.getSegmentLocalTime());
        }
    }

    void ASplineIterator::stateInBetweenSplineKeys()
    {
        const SplineIterator& iter = m_splineInit.m_iterator;
        const SplineVec3& spline = m_splineInit.m_spline;
        const SplineSegment& segment = iter.getSegment();

        const UInt32 numKeys = spline.getNumKeys();
        for (SplineKeyIndex keyIdx = 0u; keyIdx < numKeys - 1u; ++keyIdx)
        {
            const SplineTimeStamp keyTime1 = spline.getTimeStamp(keyIdx);
            const SplineTimeStamp keyTime2 = spline.getTimeStamp(keyIdx + 1u);
            const SplineTimeStamp timeStamp = (keyTime1 + keyTime2) / 2u;
            m_splineInit.setTimeStamp(timeStamp);

            const SplineKeyIndex endKeyIdx = keyIdx + 1u;
            EXPECT_EQ(keyIdx, segment.m_startIndex);
            EXPECT_EQ(endKeyIdx, segment.m_endIndex);
            EXPECT_EQ(spline.getTimeStamp(keyIdx), segment.m_startTimeStamp);
            EXPECT_EQ(spline.getTimeStamp(endKeyIdx), segment.m_endTimeStamp);

            const Float localTime = static_cast<Float>(timeStamp - segment.m_startTimeStamp) / (segment.m_endTimeStamp - segment.m_startTimeStamp);
            EXPECT_FLOAT_EQ(localTime, iter.getSegmentLocalTime());
        }
    }
}
