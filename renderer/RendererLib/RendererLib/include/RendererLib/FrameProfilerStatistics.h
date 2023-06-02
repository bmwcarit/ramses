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
#include "Utils/LoggingUtils.h"

namespace ramses_internal
{
    class StringOutputStream;

    class FrameProfilerStatistics
    {
    public:
        enum class ERegion
        {
            ExecuteRendererCommands = 0,
            UpdateClientResources,
            ApplySceneActions,
            UpdateSceneResources,
            UpdateEmbeddedCompositingResources,
            UpdateStreamTextures,
            UpdateScenesToBeMapped,
            UpdateResourceCache,
            UpdateTransformations,
            UpdateDataLinks,
            HandleDisplayEvents,
            DrawScenes,
            SwapBuffersAndNotifyClients,
            MaxFramerateSleep, // do not use this directly with startRegion/endRegion, it is handled internally
            Count
        };

        FrameProfilerStatistics();

        void startRegion(ERegion region);
        void endRegion(ERegion region);

        void markFrameFinished(std::chrono::microseconds prevFrameSleepTime);

        void writeLongestFrameTimingsToStream(StringOutputStream& str) const;
        void resetFrameTimings();

    private:
        void initNextFrameTimings();
        void setSleepTimeForPreviousFrame(std::chrono::microseconds prevFrameSleepTime);

        using RegionTimes = std::vector<uint64_t>;
        RegionTimes m_regionStartTimes;

        // region measurements for periodic logging
        // these are reset every period
        std::vector<UInt> m_frameTimings;

        UInt m_currentRegionId;

        static const uint32_t NumberOfRegions = static_cast<uint32_t>(ERegion::Count);
        static const uint32_t NumberOfFrames = 600u;
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

    const std::array RegionNames =
    {
        "RendererCommands",
        "UpdateClientResources",
        "ApplySceneActions",
        "UpdateSceneResources",
        "UpdateEmbeddedCompositingResources",
        "UpdateStreamTextures",
        "UpdateScenesToBeMapped",
        "UpdateResourceCache",
        "UpdateAnimations",
        "UpdateDataLinks",
        "HandleDisplayEvents",
        "DrawScenes",
        "SwapBuffersNotifyClients",
        "MaxFramerateSleep"
    };

    ENUM_TO_STRING(FrameProfilerStatistics::ERegion, RegionNames, FrameProfilerStatistics::ERegion::Count);
}

#endif
