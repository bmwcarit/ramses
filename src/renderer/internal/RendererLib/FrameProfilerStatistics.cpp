//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/FrameProfilerStatistics.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"
#include "internal/Core/Utils/LoggingUtils.h"
#include "internal/PlatformAbstraction/PlatformMath.h"

namespace ramses::internal
{
    FrameProfilerStatistics::FrameProfilerStatistics()
        : m_regionStartTimes(NumberOfRegions)
    {
        m_frameTimings.reserve(NumberOfFrames * NumberOfRegions);
        initNextFrameTimings();
    }

    void FrameProfilerStatistics::startRegion(ERegion region)
    {
        const auto regionId = static_cast<size_t>(region);
        assert(regionId < m_regionStartTimes.size());
        assert((regionId > m_currentRegionId || m_currentRegionId == 0u) && "Re-entering regions within frame is not supported!");
        assert(region != ERegion::MaxFramerateSleep && "Do not call startRegion() with MaxFramerateSleep, it is handled internally.");

        m_currentRegionId = regionId;
        m_regionStartTimes[regionId] = PlatformTime::GetMicrosecondsMonotonic();
    }

    void FrameProfilerStatistics::endRegion(ERegion region)
    {
        const auto regionId = static_cast<size_t>(region);
        assert(m_currentRegionId == regionId);
        assert(region != ERegion::MaxFramerateSleep && "Do not call endRegion() with MaxFramerateSleep, it is handled internally.");

        const auto totalRegionTime = static_cast<size_t>(PlatformTime::GetMicrosecondsMonotonic() - m_regionStartTimes[regionId]);
        m_frameTimings[m_frameTimings.size() - NumberOfRegions + regionId] = totalRegionTime;
    }

    void FrameProfilerStatistics::initNextFrameTimings()
    {
        if (m_frameTimings.size() < NumberOfFrames * NumberOfRegions * 2) // sanity check, do not grow tracked times if never consumed
            m_frameTimings.insert(m_frameTimings.end(), NumberOfRegions, 0u);
    }

    void FrameProfilerStatistics::markFrameFinished(std::chrono::microseconds prevFrameSleepTime)
    {
        setSleepTimeForPreviousFrame(prevFrameSleepTime);

        m_currentRegionId = 0;

        initNextFrameTimings();
    }

    void FrameProfilerStatistics::writeLongestFrameTimingsToStream(StringOutputStream& str) const
    {
        assert(!m_frameTimings.empty());
        assert(m_frameTimings.size() % NumberOfRegions == 0);
        const size_t numFramesTracked = m_frameTimings.size() / NumberOfRegions;

        size_t longestFrameID = 0u;
        size_t lastLongestFrameTime = 0u;
        std::array<size_t, NumberOfRegions> totalRegionTime = { 0u };
        for (size_t i = 0u; i < numFramesTracked; ++i)
        {
            size_t totalFrameTime = 0u;
            for (size_t reg = 0u; reg < NumberOfRegions; ++reg)
            {
                const auto regionTime = m_frameTimings[NumberOfRegions * i + reg];
                totalFrameTime += regionTime;
                totalRegionTime[reg] += regionTime;
            }

            if (totalFrameTime > lastLongestFrameTime)
            {
                lastLongestFrameTime = totalFrameTime;
                longestFrameID = i;
            }
        }

        str << "Longest frame(us)[avg]:" << lastLongestFrameTime;
        if (numFramesTracked > 0)
        {
            for (size_t reg = 0u; reg < NumberOfRegions; ++reg)
            str << " " << EnumToString(ERegion(reg)) << ":" << m_frameTimings[NumberOfRegions * longestFrameID + reg] << " [" << totalRegionTime[reg] / numFramesTracked << "]";
        }
        else
        {
            str << " no frames tracked";
        }
    }

    void FrameProfilerStatistics::resetFrameTimings()
    {
        m_frameTimings.clear();
        initNextFrameTimings();
    }

    void FrameProfilerStatistics::setSleepTimeForPreviousFrame(std::chrono::microseconds prevFrameSleepTime)
    {
        assert(prevFrameSleepTime.count() >= 0);
        if (m_frameTimings.size() > NumberOfRegions)
            m_frameTimings[m_frameTimings.size() - NumberOfRegions - 1] = static_cast<size_t>(prevFrameSleepTime.count());
    }
}
