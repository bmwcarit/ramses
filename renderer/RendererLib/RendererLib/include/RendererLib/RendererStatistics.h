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
        [[nodiscard]] uint32_t getDrawCallsPerFrame() const;

        void sceneRendered(SceneId sceneId);
        void trackArrivedFlush(SceneId sceneId, size_t numSceneActions, size_t numAddedResources, size_t numRemovedResources, size_t numSceneResourceActions, std::chrono::milliseconds latency);
        void flushApplied(SceneId sceneId);
        void flushBlocked(SceneId sceneId);

        void offscreenBufferSwapped(DeviceResourceHandle offscreenBuffer, bool isInterruptible);
        void offscreenBufferInterrupted(DeviceResourceHandle offscreenBuffer);
        void framebufferSwapped();

        void resourceUploaded(size_t byteSize);
        void sceneResourceUploaded(SceneId sceneId, size_t byteSize);
        void streamTextureUpdated(WaylandIviSurfaceId sourceId, size_t numUpdates);
        void shaderCompiled(std::chrono::microseconds microsecondsUsed, std::string_view name, SceneId sceneid);
        void setVRAMUsage(uint64_t totalUploaded, uint64_t gpuCacheSize);

        void untrackScene(SceneId sceneId);
        void untrackOffscreenBuffer(DeviceResourceHandle offscreenBuffer);
        void untrackStreamTexture(WaylandIviSurfaceId sourceId);

        void addExpirationOffset(SceneId sceneId, int64_t expirationOffset);

        void frameFinished(uint32_t drawCalls);
        void reset();

        void writeStatsToStream(StringOutputStream& str) const;

    private:
        int32_t m_frameNumber = 0;
        uint64_t m_timeBase = PlatformTime::GetMillisecondsMonotonic();
        uint32_t m_drawCalls = 0u;
        uint64_t m_lastFrameTick = 0u;
        uint32_t m_frameDurationMin = std::numeric_limits<uint32_t>::max();
        uint32_t m_frameDurationMax = 0u;
        size_t m_resourcesUploaded = 0u;
        size_t m_resourcesBytesUploaded = 0u;
        size_t m_shadersCompiled = 0u;
        uint64_t m_totalResourceUploadedSize = 0u;
        uint64_t m_gpuCacheSize = 0u;
        std::string m_maximumDurationShaderName;
        uint64_t m_microsecondsForShaderCompilation = 0u;
        std::chrono::microseconds m_maximumDurationShaderTime = {};
        SceneId m_maximumDurationShaderScene;

        struct SceneStatistics
        {
            size_t numFlushesArrived = 0u;
            size_t numFlushesApplied = 0u;
            size_t numFramesWhereFlushArrived = 0u;
            size_t numFramesWhereFlushApplied = 0u;
            size_t numFramesWhereFlushBlocked = 0u;
            size_t maxFramesWithNoFlushApplied = 0u;
            size_t maxConsecutiveFramesBlocked = 0u;
            size_t currentConsecutiveFramesBlocked = 0u;
            int32_t lastFrameFlushArrived = -1;
            int32_t lastFrameFlushApplied = -1;
            int32_t lastFrameFlushBlocked = std::numeric_limits<int32_t>::min();

            SummaryEntry<size_t> numSceneActionsPerFlush;
            SummaryEntry<size_t> numResourcesAddedPerFlush;
            SummaryEntry<size_t> numResourcesRemovedPerFlush;
            SummaryEntry<size_t> numSceneResourceActionsPerFlush;
            SummaryEntry<int64_t> flushLatency;

            // expiration offset in milliseconds, can be negative and zero (=healthy) or positive (=expired)
            SummaryEntry<int64_t> expirationOffset;
            size_t numExpirationOffsets;
            size_t numExpiredOffsets;

            size_t sceneResourcesUploaded = 0u;
            size_t sceneResourcesBytesUploaded = 0u;

            size_t numRendered = 0u;
        };

        struct OffscreenBufferStatistics
        {
            size_t numSwapped = 0u;
            size_t numInterrupted = 0u;
            bool isInterruptible = false;
        };

        struct DisplayStatistics
        {
            size_t numFrameBufferSwapped = 0;
            std::map<DeviceResourceHandle, OffscreenBufferStatistics> offscreenBufferStatistics;
        };

        struct StreamTextureStatistics
        {
            size_t numUpdates = 0u;
            size_t numFramesWhereUpdated = 0u;
            size_t maxUpdatesPerFrame = 0u;
            size_t maxFramesWithNoUpdate = 0u;
            int32_t lastFrameUpdated = -1;
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
