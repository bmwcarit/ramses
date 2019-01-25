//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/FrameProfilerStatistics.h"
#include "Collections/StringOutputStream.h"
#include "Utils/LoggingUtils.h"
#include "PlatformAbstraction/PlatformMath.h"

namespace ramses_internal
{
    FrameProfilerStatistics::FrameProfilerStatistics()
        : m_regionStartTimes(NumberOfRegions)
        , m_accumulatedRegionTimes(NumberOfFrames * NumberOfEntries, 0.f)
        , m_counters(NumberOfCounters, CounterValues(NumberOfFrames))
        , m_currentFrameId(0)
        , m_currentRegionId(0)
    {
        m_frameTimings.reserve(NumberOfFrames * NumberOfRegions);
        initNextFrameTimings();
    }

    void FrameProfilerStatistics::startRegion(ERegion region)
    {
        const UInt regionId = static_cast<UInt>(region);
        assert(regionId < m_regionStartTimes.size());
        assert((regionId > m_currentRegionId || m_currentRegionId == 0u) && "Re-entering regions within frame is not supported!");
        assert(region != ERegion::MaxFramerateSleep && "Do not call startRegion() with MaxFramerateSleep, it is handled internally.");

        // due we accumulate the region times we have to set the skipped region times to previous region time
        while (m_currentRegionId + 1 < regionId)
        {
            const UInt skippedEntryId = getEntryIdForCurrentRegion();
            m_accumulatedRegionTimes[skippedEntryId] = m_accumulatedRegionTimes[skippedEntryId - 1];
            m_currentRegionId++;
        }

        m_currentRegionId = regionId;
        m_regionStartTimes[regionId] = PlatformTime::GetMicrosecondsMonotonic();
    }

    void FrameProfilerStatistics::endRegion(ERegion region)
    {
        const UInt regionId = static_cast<UInt>(region);
        assert(m_currentRegionId == regionId);
        assert(region != ERegion::MaxFramerateSleep && "Do not call endRegion() with MaxFramerateSleep, it is handled internally.");

        const UInt totalRegionTime = static_cast<UInt>(PlatformTime::GetMicrosecondsMonotonic() - m_regionStartTimes[regionId]);
        m_frameTimings[m_frameTimings.size() - NumberOfRegions + regionId] = totalRegionTime;

        // add previous region time to current region to get stacked accumulated values which can be used directly by the FrameProfileRenderer
        const UInt entryId = getEntryIdForCurrentRegion();
        m_accumulatedRegionTimes[entryId] = m_accumulatedRegionTimes[entryId - 1];

        if ((1 << static_cast<UInt32>(region)) & m_filteredRegionFlags)
        {
            m_accumulatedRegionTimes[entryId] += static_cast<Float>(totalRegionTime);
        }
    }

    UInt FrameProfilerStatistics::getEntryIdForCurrentRegion() const
    {
        // get correct entry in frame regions, skip the first one as its the base time of zero
        const UInt entryId = m_currentFrameId * NumberOfEntries + m_currentRegionId + 1;
        assert(entryId < m_accumulatedRegionTimes.size());
        return entryId;
    }

    void FrameProfilerStatistics::initNextFrameTimings()
    {
        if (m_frameTimings.size() < NumberOfFrames * NumberOfRegions * 2) // sanity check, do not grow tracked times if never consumed
        {
            m_frameTimings.resize(m_frameTimings.size() + NumberOfRegions);
            std::fill(m_frameTimings.end() - NumberOfRegions, m_frameTimings.end(), 0u);
        }
    }

    void FrameProfilerStatistics::setFilteredRegionFlags(UInt32 regionFlags)
    {
        m_filteredRegionFlags = regionFlags;
    }

    void FrameProfilerStatistics::setCounterValue(ECounter counter, UInt32 value)
    {
        const UInt counterId = static_cast<UInt>(counter);
        assert(counterId < m_counters.size());
        m_counters[counterId][m_currentFrameId] = static_cast<Float>(value);
    }

    const FrameProfilerStatistics::RegionTimings& FrameProfilerStatistics::getRegionTimings() const
    {
        return m_accumulatedRegionTimes;
    }

    const FrameProfilerStatistics::CounterValues& FrameProfilerStatistics::getCounterValues(ECounter counter) const
    {
        const UInt counterId = static_cast<UInt>(counter);
        return m_counters[counterId];
    }

    void FrameProfilerStatistics::markFrameFinished(std::chrono::microseconds sleepTime)
    {
        // due we accumulate the region times we have to set the skipped region times to previous region time
        while (m_currentRegionId + 1 < NumberOfRegions)
        {
            m_currentRegionId++;
            const UInt skippedEntryId = getEntryIdForCurrentRegion();
            m_accumulatedRegionTimes[skippedEntryId] = m_accumulatedRegionTimes[skippedEntryId - 1];
        }

        setSleepTimeForLastFrame(sleepTime);

        m_currentRegionId = 0;

        if (++m_currentFrameId >= NumberOfFrames)
        {
            m_currentFrameId = 0;
        }

        initNextFrameTimings();
    }

    UInt32 FrameProfilerStatistics::getCurrentFrameId() const
    {
        return m_currentFrameId;
    }

    void FrameProfilerStatistics::writeLongestFrameTimingsToStream(StringOutputStream& str) const
    {
        assert(!m_frameTimings.empty());
        assert(m_frameTimings.size() % NumberOfRegions == 0);
        const UInt numFramesTracked = m_frameTimings.size() / NumberOfRegions;

        UInt longestFrameID = 0u;
        UInt lastLongestFrameTime = 0u;
        UInt totalRegionTime[NumberOfRegions] = { 0u };
        for (UInt i = 0u; i < numFramesTracked; ++i)
        {
            UInt totalFrameTime = 0u;
            for (UInt reg = 0u; reg < NumberOfRegions; ++reg)
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
            for (UInt reg = 0u; reg < NumberOfRegions; ++reg)
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

    void FrameProfilerStatistics::setSleepTimeForLastFrame(std::chrono::microseconds sleepTime)
    {
        assert(m_currentRegionId == static_cast<UInt>(ERegion::MaxFramerateSleep));
        assert(sleepTime.count() >= 0);
        if (m_currentFrameId == 0)
            return;

        const UInt currentFrameSleepEntryId = getEntryIdForCurrentRegion();
        const UInt lastFrameSleepEntryId = currentFrameSleepEntryId - NumberOfEntries;

        m_accumulatedRegionTimes[lastFrameSleepEntryId] = m_accumulatedRegionTimes[lastFrameSleepEntryId - 1] + sleepTime.count();
        if (m_frameTimings.size() > NumberOfRegions)
            m_frameTimings[m_frameTimings.size() - NumberOfRegions - 1] = static_cast<UInt>(sleepTime.count());
    }
}
