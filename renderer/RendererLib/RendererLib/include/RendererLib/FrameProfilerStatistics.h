//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_FRAMEPROFILERSTATISTICS_H
#define RAMSES_FRAMEPROFILERSTATISTICS_H

#include "PlatformAbstraction/PlatformTime.h"
#include "Collections/Vector.h"
#include "Collections/String.h"
#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    class StringOutputStream;

    class FrameProfilerStatistics
    {
    public:
        enum class ERegion
        {
            RendererCommands = 0,
            ConsolidateSceneActions,
            UpdateClientResources,
            ApplySceneActions,
            UpdateSceneResources,
            UpdateEmbeddedCompositingResources,
            UpdateStreamTextures,
            UpdateScenesToBeMapped,
            UpdateResourceCache,
            UpdateAnimations,
            UpdateTransformations,
            UpdateDataLinks,
            HandleDisplayEvents,
            DrawScenes,
            SwapBuffersAndNotifyClients,
            MaxFramerateSleep, // do not use this directly with startRegion/stopRegion, its handled internally
            Count
        };

        /*  Regions measure the execution time of marked pieces of code

            For example with 3 different regions per frame the internal data would looks like this:
            The horizontal axis represents all previous measured frames and the vertical axis the accumulated times for each region.
            The first entry for each frame represents the base time of zero.
            The region must be executed in increasing order of their position in the enum ERegion.

            Frame           1   2   3   4   5   6   ...

            Region  2       8   9  10   8   9   7   ...
                    1       5   7   6   5   4   5   ...
                    0       2   1   3   2   1   3   ...
                            0   0   0   0   0   0   ...

            The major benefit of this data layout is the directly usage as a vertex buffer for rendering the region times. No conversion is necessary.
        */

        enum class ECounter
        {
            DrawCalls = 0,
            AppliedSceneActions,
            UsedGPUMemory,
            Count
        };

        /* Counters are simple values changing over each frame like draw calls or memory usage

            Frame           1   2   3   4   5   6   ...

            Counter value   0   9   5   0   9   0   ...

            This counter value data is directly used as vertex buffer for rendering the counter graphs.
        */

        static const UInt32 NumberOfFrames = 600u;
        static const UInt NumberOfRegions = static_cast<UInt>(ERegion::Count);
        static const UInt NumberOfCounters = static_cast<UInt>(ECounter::Count);

        // one additional region acting as base time for one frame, its always zero and crucial for rendering a stacked time line
        static const UInt NumberOfEntries = NumberOfRegions + 1;


        FrameProfilerStatistics();

        void startRegion(ERegion region);
        void endRegion(ERegion region);

        using RegionTimings = std::vector<Float>;
        const RegionTimings& getRegionTimings() const;

        void setCounterValue(ECounter counter, UInt32 value);

        using CounterValues = std::vector<Float>;
        const CounterValues& getCounterValues(ECounter counter) const;

        // only shows the chosen combination of ERegions
        void setFilteredRegionFlags(UInt32 regionFlags);

        void markFrameFinished(std::chrono::microseconds sleepTime);
        UInt32 getCurrentFrameId() const;

        void writeLongestFrameTimingsToStream(StringOutputStream& str) const;
        void resetFrameTimings();

    private:
        UInt getEntryIdForCurrentRegion() const;
        void initNextFrameTimings();
        void setSleepTimeForLastFrame(std::chrono::microseconds sleepTime);

        using RegionTimes = std::vector<UInt64>;
        RegionTimes m_regionStartTimes;

        // region measurements in a circular buffer holding last NumberOfFrames data
        // this data can be directly used for overlay profiler rendering
        RegionTimings m_accumulatedRegionTimes;

        // region measurements for periodic logging
        // these are reset every period
        std::vector<UInt> m_frameTimings;

        using Counters = std::vector<CounterValues>;
        Counters m_counters;

        UInt32 m_currentFrameId;
        UInt m_currentRegionId;

        UInt32 m_filteredRegionFlags = ~0u;
    };

    class ScopedFrameProfilerRegion
    {
    public:
        ScopedFrameProfilerRegion(FrameProfilerStatistics& stats, FrameProfilerStatistics::ERegion region)
            : m_stats(stats)
            , m_region(region)
        {
            m_stats.startRegion(m_region);
        }

        ~ScopedFrameProfilerRegion()
        {
            m_stats.endRegion(m_region);
        }

    private:
        FrameProfilerStatistics &m_stats;
        FrameProfilerStatistics::ERegion m_region;
    };

#define FRAME_PROFILER_REGION(Region) \
    ScopedFrameProfilerRegion region(m_renderer.getProfilerStatistics(), Region)

    static const char* RegionNames[] =
    {
        "RendererCommands",
        "ConsolidateSceneActions",
        "UpdateClientResources",
        "ApplySceneActions",
        "UpdateSceneResources",
        "UpdateEmbeddedCompositingResources",
        "UpdateStreamTextures",
        "UpdateScenesToBeMapped",
        "UpdateResourceCache",
        "UpdateAnimations",
        "UpdateTransformations",
        "UpdateDataLinks",
        "HandleDisplayEvents",
        "DrawScenes",
        "SwapBuffersNotifyClients",
        "MaxFramerateSleep"
    };

    ENUM_TO_STRING(FrameProfilerStatistics::ERegion, RegionNames, FrameProfilerStatistics::ERegion::Count);
}

#endif
