//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSTATISTICS_H
#define RAMSES_RENDERERSTATISTICS_H

#include "SceneAPI/SceneId.h"
#include "RendererAPI/Types.h"
#include "Utils/StatisticCollection.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "PlatformAbstraction/PlatformTypes.h"
#include "Components/FlushTimeInformation.h"

#include <map>
#include <string>
#include <string_view>

namespace ramses_internal
{
    class StringOutputStream;

    class RendererStatistics
    {
    public:
        [[nodiscard]] float  getFps() const;
        [[nodiscard]] UInt32 getDrawCallsPerFrame() const;

        void sceneRendered(SceneId sceneId);
        void trackArrivedFlush(SceneId sceneId, UInt numSceneActions, UInt numAddedResources, UInt numRemovedResources, UInt numSceneResourceActions, std::chrono::milliseconds latency);
        void flushApplied(SceneId sceneId);
        void flushBlocked(SceneId sceneId);

        void offscreenBufferSwapped(DeviceResourceHandle offscreenBuffer, bool isInterruptible);
        void offscreenBufferInterrupted(DeviceResourceHandle offscreenBuffer);
        void framebufferSwapped();

        void resourceUploaded(UInt byteSize);
        void sceneResourceUploaded(SceneId sceneId, UInt byteSize);
        void streamTextureUpdated(WaylandIviSurfaceId sourceId, UInt numUpdates);
        void shaderCompiled(std::chrono::microseconds microsecondsUsed, std::string_view name, SceneId sceneid);
        void setVRAMUsage(uint64_t totalUploaded, uint64_t gpuCacheSize);

        void untrackScene(SceneId sceneId);
        void untrackOffscreenBuffer(DeviceResourceHandle offscreenBuffer);
        void untrackStreamTexture(WaylandIviSurfaceId sourceId);

        void addExpirationOffset(SceneId sceneId, int64_t expirationOffset);

        void frameFinished(UInt32 drawCalls);
        void reset();

        void writeStatsToStream(StringOutputStream& str) const;

    private:
        Int32 m_frameNumber = 0;
        uint64_t m_timeBase = PlatformTime::GetMillisecondsMonotonic();
        UInt32 m_drawCalls = 0u;
        uint64_t m_lastFrameTick = 0u;
        UInt32 m_frameDurationMin = std::numeric_limits<UInt32>::max();
        UInt32 m_frameDurationMax = 0u;
        UInt m_resourcesUploaded = 0u;
        UInt m_resourcesBytesUploaded = 0u;
        UInt m_shadersCompiled = 0u;
        uint64_t m_totalResourceUploadedSize = 0u;
        uint64_t m_gpuCacheSize = 0u;
        std::string m_maximumDurationShaderName;
        uint64_t m_microsecondsForShaderCompilation = 0u;
        std::chrono::microseconds m_maximumDurationShaderTime = {};
        SceneId m_maximumDurationShaderScene;

        struct SceneStatistics
        {
            UInt numFlushesArrived = 0u;
            UInt numFlushesApplied = 0u;
            UInt numFramesWhereFlushArrived = 0u;
            UInt numFramesWhereFlushApplied = 0u;
            UInt numFramesWhereFlushBlocked = 0u;
            UInt maxFramesWithNoFlushApplied = 0u;
            UInt maxConsecutiveFramesBlocked = 0u;
            UInt currentConsecutiveFramesBlocked = 0u;
            Int32 lastFrameFlushArrived = -1;
            Int32 lastFrameFlushApplied = -1;
            Int32 lastFrameFlushBlocked = std::numeric_limits<Int32>::min();

            SummaryEntry<UInt> numSceneActionsPerFlush;
            SummaryEntry<UInt> numResourcesAddedPerFlush;
            SummaryEntry<UInt> numResourcesRemovedPerFlush;
            SummaryEntry<UInt> numSceneResourceActionsPerFlush;
            SummaryEntry<int64_t> flushLatency;

            // expiration offset in milliseconds, can be negative and zero (=healthy) or positive (=expired)
            SummaryEntry<int64_t> expirationOffset;
            UInt numExpirationOffsets;
            UInt numExpiredOffsets;

            UInt sceneResourcesUploaded = 0u;
            UInt sceneResourcesBytesUploaded = 0u;

            UInt numRendered = 0u;
        };

        struct OffscreenBufferStatistics
        {
            UInt numSwapped = 0u;
            UInt numInterrupted = 0u;
            bool isInterruptible = false;
        };

        struct DisplayStatistics
        {
            UInt numFrameBufferSwapped = 0;
            std::map<DeviceResourceHandle, OffscreenBufferStatistics> offscreenBufferStatistics;
        };

        struct StreamTextureStatistics
        {
            UInt numUpdates = 0u;
            UInt numFramesWhereUpdated = 0u;
            UInt maxUpdatesPerFrame = 0u;
            UInt maxFramesWithNoUpdate = 0u;
            Int32 lastFrameUpdated = -1;
        };

        template <typename T>
        struct StronglyTypedValueComparator
        {
            bool operator()(const T& a, const T& b) const
            {
                return a.getValue() < b.getValue();
            }
        };

        // using map so that items are logged ordered
        std::map< SceneId, SceneStatistics, StronglyTypedValueComparator<SceneId> > m_sceneStatistics;
        DisplayStatistics m_displayStatistics;
        std::map< WaylandIviSurfaceId, StreamTextureStatistics, StronglyTypedValueComparator<WaylandIviSurfaceId> > m_streamTextureStatistics;
    };
}

#endif
