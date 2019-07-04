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
#include <map>

namespace ramses_internal
{
    class StringOutputStream;

    class RendererStatistics
    {
    public:
        Float  getFps() const;
        UInt32 getDrawCallsPerFrame() const;

        void sceneRendered(SceneId sceneId);
        void trackArrivedFlush(SceneId sceneId, UInt numSceneActions, UInt numAddedClientResources, UInt numRemovedClientResources, UInt numSceneResourceActions);
        void flushApplied(SceneId sceneId);
        void flushBlocked(SceneId sceneId);
        void flushApplyInterrupted(SceneId sceneId);

        void offscreenBufferSwapped(DisplayHandle displayHandle, DeviceResourceHandle offscreenBuffer, bool isInterruptible);
        void offscreenBufferInterrupted(DisplayHandle displayHandle, DeviceResourceHandle offscreenBuffer);
        void framebufferSwapped(DisplayHandle display);

        void clientResourceUploaded(UInt byteSize);
        void sceneResourceUploaded(SceneId sceneId, UInt byteSize);
        void streamTextureUpdated(StreamTextureSourceId sourceId, UInt numUpdates);
        void shaderCompiled(int64_t microsecondsUsed);

        void untrackScene(SceneId sceneId);
        void untrackOffscreenBuffer(DisplayHandle displayHandle, DeviceResourceHandle offscreenBuffer);
        void untrackStreamTexture(StreamTextureSourceId sourceId);

        void frameFinished(UInt32 drawCalls);
        void reset();

        void writeStatsToStream(StringOutputStream& str) const;

    private:
        Int32 m_frameNumber = 0;
        UInt64 m_timeBase = PlatformTime::GetMillisecondsMonotonic();
        UInt32 m_drawCalls = 0u;
        UInt64 m_lastFrameTick = 0u;
        UInt32 m_frameDurationMin = std::numeric_limits<UInt32>::max();
        UInt32 m_frameDurationMax = 0u;
        UInt m_clientResourcesUploaded = 0u;
        UInt m_clientResourcesBytesUploaded = 0u;
        UInt m_shadersCompiled = 0u;
        UInt64 m_microsecondsForShaderCompilation = 0u;

        struct SceneStatistics
        {
            UInt numFlushesArrived = 0u;
            UInt numFlushesApplied = 0u;
            UInt numFramesWhereFlushArrived = 0u;
            UInt numFramesWhereFlushApplied = 0u;
            UInt numFramesWhereFlushBlocked = 0u;
            UInt numFlushApplyInterrupted = 0u;
            UInt maxFramesWithNoFlushApplied = 0u;
            UInt maxConsecutiveFramesBlocked = 0u;
            UInt currentConsecutiveFramesBlocked = 0u;
            Int32 lastFrameFlushArrived = -1;
            Int32 lastFrameFlushApplied = -1;
            Int32 lastFrameFlushBlocked = std::numeric_limits<Int32>::min();

            SummaryEntry<UInt> numSceneActionsPerFlush;
            SummaryEntry<UInt> numClientResourcesAddedPerFlush;
            SummaryEntry<UInt> numClientResourcesRemovedPerFlush;
            SummaryEntry<UInt> numSceneResourceActionsPerFlush;

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
        std::map< DisplayHandle, DisplayStatistics > m_displayStatistics;
        std::map< StreamTextureSourceId, StreamTextureStatistics, StronglyTypedValueComparator<StreamTextureSourceId> > m_streamTextureStatistics;
    };
}

#endif
