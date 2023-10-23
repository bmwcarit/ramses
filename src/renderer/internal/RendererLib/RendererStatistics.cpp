//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/RendererStatistics.h"
#include "internal/PlatformAbstraction/PlatformTime.h"
#include "internal/PlatformAbstraction/Collections/StringOutputStream.h"

namespace ramses::internal
{
    float RendererStatistics::getFps() const
    {
        const uint64_t reportTimeInterval = PlatformTime::GetMillisecondsMonotonic() - m_timeBase;
        if (reportTimeInterval > 0u)
            return (1000.f * static_cast<float>(m_frameNumber)) / static_cast<float>(reportTimeInterval);

        return 0.f;
    }

    uint32_t RendererStatistics::getDrawCallsPerFrame() const
    {
        return m_frameNumber <= 0 ? 0u : m_drawCalls / m_frameNumber;
    }

    void RendererStatistics::sceneRendered(SceneId sceneId)
    {
        m_sceneStatistics[sceneId].numRendered++;
    }

    void RendererStatistics::offscreenBufferSwapped(DeviceResourceHandle offscreenBuffer, bool isInterruptible)
    {
        auto& obStat = m_displayStatistics.offscreenBufferStatistics[offscreenBuffer];
        obStat.numSwapped++;
        obStat.isInterruptible = isInterruptible;
    }

    void RendererStatistics::offscreenBufferInterrupted(DeviceResourceHandle offscreenBuffer)
    {
        auto& obStat = m_displayStatistics.offscreenBufferStatistics[offscreenBuffer];
        obStat.numInterrupted++;
        obStat.isInterruptible = true;
    }

    void RendererStatistics::framebufferSwapped()
    {
        m_displayStatistics.numFrameBufferSwapped++;
    }

    void RendererStatistics::resourceUploaded(size_t byteSize)
    {
        m_resourcesUploaded++;
        m_resourcesBytesUploaded += byteSize;
    }

    void RendererStatistics::sceneResourceUploaded(SceneId sceneId, size_t byteSize)
    {
        auto& sceneStats = m_sceneStatistics[sceneId];
        sceneStats.sceneResourcesUploaded++;
        sceneStats.sceneResourcesBytesUploaded += byteSize;
    }

    void RendererStatistics::streamTextureUpdated(WaylandIviSurfaceId iviSurface, size_t numUpdates)
    {
        auto& strTexStat = m_streamTextureStatistics[iviSurface];
        strTexStat.numUpdates += numUpdates;
        if (strTexStat.lastFrameUpdated != m_frameNumber)
        {
            strTexStat.numFramesWhereUpdated++;
            strTexStat.lastFrameUpdated = m_frameNumber;
        }
        strTexStat.maxUpdatesPerFrame = std::max(strTexStat.maxUpdatesPerFrame, numUpdates);
    }

    void RendererStatistics::shaderCompiled(std::chrono::microseconds microsecondsUsed, std::string_view name, SceneId sceneid)
    {
        m_shadersCompiled++;
        m_microsecondsForShaderCompilation += microsecondsUsed.count();
        if (microsecondsUsed > m_maximumDurationShaderTime)
        {
            m_maximumDurationShaderTime = microsecondsUsed;
            m_maximumDurationShaderName = name;
            m_maximumDurationShaderScene = sceneid;
        }
    }

    void RendererStatistics::setVRAMUsage(uint64_t totalUploaded, uint64_t gpuCacheSize)
    {
        m_totalResourceUploadedSize = totalUploaded;
        m_gpuCacheSize = gpuCacheSize;
    }

    void RendererStatistics::trackArrivedFlush(SceneId sceneId, size_t numSceneActions, size_t numAddedResources, size_t numRemovedResources, size_t numSceneResourceActions, std::chrono::milliseconds latency)
    {
        auto& sceneStats = m_sceneStatistics[sceneId];
        sceneStats.numFlushesArrived++;
        if (sceneStats.lastFrameFlushArrived != m_frameNumber)
        {
            sceneStats.numFramesWhereFlushArrived++;
            sceneStats.lastFrameFlushArrived = m_frameNumber;
        }

        sceneStats.numSceneActionsPerFlush.update(numSceneActions);
        sceneStats.numResourcesAddedPerFlush.update(numAddedResources);
        sceneStats.numResourcesRemovedPerFlush.update(numRemovedResources);
        sceneStats.numSceneResourceActionsPerFlush.update(numSceneResourceActions);
        sceneStats.flushLatency.update(static_cast<int64_t>(latency.count()));
    }

    void RendererStatistics::flushApplied(SceneId sceneId)
    {
        auto& sceneStats = m_sceneStatistics[sceneId];
        sceneStats.numFlushesApplied++;
        if (sceneStats.lastFrameFlushApplied != m_frameNumber)
        {
            sceneStats.numFramesWhereFlushApplied++;
            sceneStats.lastFrameFlushApplied = m_frameNumber;
        }
    }

    void RendererStatistics::flushBlocked(SceneId sceneId)
    {
        auto& sceneStats = m_sceneStatistics[sceneId];
        if (sceneStats.lastFrameFlushBlocked != m_frameNumber)
        {
            sceneStats.numFramesWhereFlushBlocked++;
            if (sceneStats.lastFrameFlushBlocked == m_frameNumber - 1)
            {
                sceneStats.currentConsecutiveFramesBlocked++; // keeps being blocked
            }
            else
            {
                sceneStats.currentConsecutiveFramesBlocked = 1u; // starts being blocked
            }
            sceneStats.maxConsecutiveFramesBlocked = std::max(sceneStats.maxConsecutiveFramesBlocked, sceneStats.currentConsecutiveFramesBlocked);
            sceneStats.lastFrameFlushBlocked = m_frameNumber;
        }
    }

    void RendererStatistics::untrackScene(SceneId sceneId)
    {
        m_sceneStatistics.erase(sceneId);
    }

    void RendererStatistics::untrackOffscreenBuffer(DeviceResourceHandle offscreenBuffer)
    {
        m_displayStatistics.offscreenBufferStatistics.erase(offscreenBuffer);
    }

    void RendererStatistics::untrackStreamTexture(WaylandIviSurfaceId iviSurface)
    {
        m_streamTextureStatistics.erase(iviSurface);
    }

    void RendererStatistics::frameFinished(uint32_t drawCalls)
    {
        const uint64_t currTick = PlatformTime::GetMicrosecondsMonotonic();
        const auto frameDuration = static_cast<uint32_t>(currTick - m_lastFrameTick);
        if (frameDuration < m_frameDurationMin)
        {
            m_frameDurationMin = frameDuration;
        }
        else if (frameDuration > m_frameDurationMax)
        {
            m_frameDurationMax = frameDuration;
        }

        // update 'gap' measurements - max number of consecutive frames with some action happening or not
        for (auto& sceneStat : m_sceneStatistics)
        {
            if (m_frameNumber > sceneStat.second.lastFrameFlushApplied)
                sceneStat.second.maxFramesWithNoFlushApplied = std::max<size_t>(sceneStat.second.maxFramesWithNoFlushApplied, m_frameNumber - sceneStat.second.lastFrameFlushApplied);
        }

        for (auto& strTexStat : m_streamTextureStatistics)
        {
            if (m_frameNumber > strTexStat.second.lastFrameUpdated)
                strTexStat.second.maxFramesWithNoUpdate = std::max<size_t>(strTexStat.second.maxFramesWithNoUpdate, m_frameNumber - strTexStat.second.lastFrameUpdated);
        }

        m_frameNumber++;
        m_drawCalls += drawCalls;

        m_lastFrameTick = currTick;
    }

    void RendererStatistics::addExpirationOffset(SceneId sceneId, int64_t expirationOffset)
    {
        m_sceneStatistics[sceneId].expirationOffset.update(expirationOffset);
        m_sceneStatistics[sceneId].numExpirationOffsets++;
        if (expirationOffset > 0)
            m_sceneStatistics[sceneId].numExpiredOffsets++;
    }

    void RendererStatistics::reset()
    {
        m_timeBase = PlatformTime::GetMillisecondsMonotonic();
        m_frameNumber = 0;
        m_drawCalls = 0u;
        m_frameDurationMin = std::numeric_limits<uint32_t>::max();
        m_frameDurationMax = 0u;
        m_resourcesUploaded = 0u;
        m_resourcesBytesUploaded = 0u;
        m_shadersCompiled = 0u;
        m_microsecondsForShaderCompilation = 0u;
        m_maximumDurationShaderName = "";
        m_maximumDurationShaderTime = std::chrono::microseconds(0u);
        m_maximumDurationShaderScene = SceneId::Invalid();

        for (auto& sceneStatIt : m_sceneStatistics)
        {
            auto& sceneStat = sceneStatIt.second;
            sceneStat.numFlushesArrived = 0u;
            sceneStat.numFlushesApplied = 0u;
            sceneStat.numFramesWhereFlushArrived = 0u;
            sceneStat.numFramesWhereFlushApplied = 0u;
            sceneStat.numFramesWhereFlushBlocked = 0u;
            sceneStat.maxFramesWithNoFlushApplied = 0u;
            sceneStat.maxConsecutiveFramesBlocked = 0u;
            sceneStat.lastFrameFlushArrived = -1; // treat as if arrived in previous period's last frame (ie. do not measure 'arrive gaps' across periods)
            sceneStat.lastFrameFlushApplied = -1; // treat as if applied in previous period's last frame (ie. do not measure 'apply gaps' across periods)
            sceneStat.lastFrameFlushBlocked = std::numeric_limits<int32_t>::min(); // treat as if previous period did not end with blocked flush (ie. do not measure 'consecutive blocks' across periods)
            sceneStat.numSceneActionsPerFlush.reset();
            sceneStat.numResourcesAddedPerFlush.reset();
            sceneStat.numResourcesRemovedPerFlush.reset();
            sceneStat.numSceneResourceActionsPerFlush.reset();
            sceneStat.flushLatency.reset();
            sceneStat.expirationOffset.reset();
            sceneStat.numExpirationOffsets = 0u;
            sceneStat.numExpiredOffsets = 0u;
            sceneStat.sceneResourcesUploaded = 0u;
            sceneStat.sceneResourcesBytesUploaded = 0u;
            sceneStat.numRendered = 0u;
        }

        m_displayStatistics.numFrameBufferSwapped = 0u;
        for (auto& obStat : m_displayStatistics.offscreenBufferStatistics)
        {
            obStat.second.numSwapped = 0u;
            obStat.second.numInterrupted = 0u;
        }

        for (auto& strTexStat : m_streamTextureStatistics)
        {
            strTexStat.second.numUpdates = 0u;
            strTexStat.second.numFramesWhereUpdated = 0u;
            strTexStat.second.maxUpdatesPerFrame = 0u;
            strTexStat.second.maxFramesWithNoUpdate = 0u;
            strTexStat.second.lastFrameUpdated = -1;
        }
    }

    void RendererStatistics::writeStatsToStream(StringOutputStream& str) const
    {
        if (m_frameNumber == 0u)
            return;

        str << "Avg framerate: " << getFps() << " FPS"
            " [minFrameTime " << m_frameDurationMin << "us" <<
            ", maxFrameTime " << m_frameDurationMax << "us]" <<
            ", drawcallsPerFrame " << getDrawCallsPerFrame() <<
            ", numFrames " << m_frameNumber;
        if (m_resourcesUploaded > 0u)
            str << ", resUploaded " << m_resourcesUploaded << " (" << m_resourcesBytesUploaded << " B)";
        str << ", RC VRAM usage/cache (" << (m_totalResourceUploadedSize >> 20) << "/" << (m_gpuCacheSize >> 20) << " MB)";
        if (m_shadersCompiled > 0u)
        {
            str << ", shadersCompiled " << m_shadersCompiled << " for total ms:" << m_microsecondsForShaderCompilation / 1000;
            str << ", avg microsec " << m_microsecondsForShaderCompilation / m_shadersCompiled;
            str << "; longest: " << m_maximumDurationShaderName << " from scene:" << m_maximumDurationShaderScene << " ms:" << m_maximumDurationShaderTime.count() / 1000;
        }
        str << "\n";

        str << "FB: " << m_displayStatistics.numFrameBufferSwapped;
        for (const auto& obStat : m_displayStatistics.offscreenBufferStatistics)
        {
            str << "; OB" << obStat.first << ": " << obStat.second.numSwapped;
            if (obStat.second.isInterruptible)
                str << " (intr: " << obStat.second.numInterrupted << ")";
        }
        str << "\n";

        for (const auto& sceneStatsIt : m_sceneStatistics)
        {
            const auto& sceneStats = sceneStatsIt.second;
            const auto& numSceneActionsPerFlush = sceneStats.numSceneActionsPerFlush;
            const auto& numResourcesAddedPerFlush = sceneStats.numResourcesAddedPerFlush;
            const auto& numResourcesRemovedPerFlush = sceneStats.numResourcesRemovedPerFlush;
            const auto& numSceneResourceActionsPerFlush = sceneStats.numSceneResourceActionsPerFlush;
            const auto& flushLatency = sceneStats.flushLatency;
            const auto& expirationOffset = sceneStats.expirationOffset;

            str << "Scene " << sceneStatsIt.first << ": ";
            str << "rendered " << sceneStats.numRendered;
            str << ", framesFArrived " << sceneStats.numFramesWhereFlushArrived;
            str << ", framesFApplied " << sceneStats.numFramesWhereFlushApplied;
            str << ", framesFBlocked " << sceneStats.numFramesWhereFlushBlocked;
            str << ", maxFramesWithNoFApplied " << sceneStats.maxFramesWithNoFlushApplied;
            str << ", maxFramesFBlocked " << sceneStats.maxConsecutiveFramesBlocked;
            str << ", FArrived " << sceneStats.numFlushesArrived;
            str << ", FApplied " << sceneStats.numFlushesApplied;
            if (sceneStats.numFlushesArrived > 0u)
            {
                str << ", actions/F (" << numSceneActionsPerFlush.minValue << "/" << numSceneActionsPerFlush.maxValue << "/" << static_cast<float>(numSceneActionsPerFlush.sum) / static_cast<float>(sceneStats.numFlushesArrived) << ")";
                str << ", dt/F (" << flushLatency.minValue << "/" << flushLatency.maxValue << "/" << static_cast<float>(flushLatency.sum) / static_cast<float>(sceneStats.numFlushesArrived) << ")";
                str << ", RC+/F (" << numResourcesAddedPerFlush.minValue << "/" << numResourcesAddedPerFlush.maxValue << "/" << static_cast<float>(numResourcesAddedPerFlush.sum) / static_cast<float>(sceneStats.numFlushesArrived) << ")";
                str << ", RC-/F (" << numResourcesRemovedPerFlush.minValue << "/" << numResourcesRemovedPerFlush.maxValue << "/" << static_cast<float>(numResourcesRemovedPerFlush.sum) / static_cast<float>(sceneStats.numFlushesArrived) << ")";
                str << ", RS/F (" << numSceneResourceActionsPerFlush.minValue << "/" << numSceneResourceActionsPerFlush.maxValue << "/" << static_cast<float>(numSceneResourceActionsPerFlush.sum) / static_cast<float>(sceneStats.numFlushesArrived) << ")";
            }
            if (sceneStats.numExpirationOffsets > 0u)
                str << ", Exp (" << sceneStats.numExpiredOffsets << "/" << sceneStats.numExpirationOffsets << ":" << expirationOffset.minValue << "/" << expirationOffset.maxValue << "/" << static_cast<float>(expirationOffset.sum) / static_cast<float>(sceneStats.numExpirationOffsets) << ")";

            if (sceneStats.sceneResourcesUploaded > 0u)
                str << ", RSUploaded " << sceneStats.sceneResourcesUploaded << " (" << sceneStats.sceneResourcesBytesUploaded << " B)";
            str << "\n";
        }

        for (const auto& strTexStat : m_streamTextureStatistics)
        {
            str << "SourceId " << strTexStat.first << ": ";
            str << "upd " << strTexStat.second.numUpdates;
            str << ", framesUpd " << strTexStat.second.numFramesWhereUpdated;
            str << ", maxUpdInFrame " << strTexStat.second.maxUpdatesPerFrame;
            str << ", maxFramesWithNoUpd " << strTexStat.second.maxFramesWithNoUpdate;
            str << "\n";
        }
    }
}
