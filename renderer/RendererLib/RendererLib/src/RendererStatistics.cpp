//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererStatistics.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "Collections/StringOutputStream.h"

namespace ramses_internal
{
    Float RendererStatistics::getFps() const
    {
        const UInt64 reportTimeInterval = PlatformTime::GetMillisecondsMonotonic() - m_timeBase;
        if (reportTimeInterval > 0u)
            return (1000.f * m_frameNumber) / reportTimeInterval;

        return 0.f;
    }

    UInt32 RendererStatistics::getDrawCallsPerFrame() const
    {
        return m_frameNumber <= 0 ? 0u : m_drawCalls / m_frameNumber;
    }

    void RendererStatistics::sceneRendered(SceneId sceneId)
    {
        m_sceneStatistics[sceneId].numRendered++;
    }

    void RendererStatistics::offscreenBufferSwapped(DisplayHandle displayHandle, DeviceResourceHandle offscreenBuffer, bool isInterruptible)
    {
        auto& obStat = m_displayStatistics[displayHandle].offscreenBufferStatistics[offscreenBuffer];
        obStat.numSwapped++;
        obStat.isInterruptible = isInterruptible;
    }

    void RendererStatistics::offscreenBufferInterrupted(DisplayHandle displayHandle, DeviceResourceHandle offscreenBuffer)
    {
        auto& obStat = m_displayStatistics[displayHandle].offscreenBufferStatistics[offscreenBuffer];
        obStat.numInterrupted++;
        obStat.isInterruptible = true;
    }

    void RendererStatistics::framebufferSwapped(DisplayHandle display)
    {
        m_displayStatistics[display].numFrameBufferSwapped++;
    }

    void RendererStatistics::clientResourceUploaded(UInt byteSize)
    {
        m_clientResourcesUploaded++;
        m_clientResourcesBytesUploaded += byteSize;
    }

    void RendererStatistics::sceneResourceUploaded(SceneId sceneId, UInt byteSize)
    {
        auto& sceneStats = m_sceneStatistics[sceneId];
        sceneStats.sceneResourcesUploaded++;
        sceneStats.sceneResourcesBytesUploaded += byteSize;
    }

    void RendererStatistics::streamTextureUpdated(StreamTextureSourceId sourceId, UInt numUpdates)
    {
        auto& strTexStat = m_streamTextureStatistics[sourceId];
        strTexStat.numUpdates += numUpdates;
        if (strTexStat.lastFrameUpdated != m_frameNumber)
        {
            strTexStat.numFramesWhereUpdated++;
            strTexStat.lastFrameUpdated = m_frameNumber;
        }
        strTexStat.maxUpdatesPerFrame = std::max(strTexStat.maxUpdatesPerFrame, numUpdates);
    }

    void RendererStatistics::shaderCompiled(int64_t microsecondsUsed)
    {
        m_shadersCompiled++;
        m_microsecondsForShaderCompilation += microsecondsUsed;
    }

    void RendererStatistics::trackArrivedFlush(SceneId sceneId, UInt numSceneActions, UInt numAddedClientResources, UInt numRemovedClientResources, UInt numSceneResourceActions)
    {
        auto& sceneStats = m_sceneStatistics[sceneId];
        sceneStats.numFlushesArrived++;
        if (sceneStats.lastFrameFlushArrived != m_frameNumber)
        {
            sceneStats.numFramesWhereFlushArrived++;
            sceneStats.lastFrameFlushArrived = m_frameNumber;
        }

        sceneStats.numSceneActionsPerFlush.update(numSceneActions);
        sceneStats.numClientResourcesAddedPerFlush.update(numAddedClientResources);
        sceneStats.numClientResourcesRemovedPerFlush.update(numRemovedClientResources);
        sceneStats.numSceneResourceActionsPerFlush.update(numSceneResourceActions);
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
                sceneStats.currentConsecutiveFramesBlocked++; // keeps being blocked
            else
                sceneStats.currentConsecutiveFramesBlocked = 1u; // starts being blocked
            sceneStats.maxConsecutiveFramesBlocked = std::max(sceneStats.maxConsecutiveFramesBlocked, sceneStats.currentConsecutiveFramesBlocked);
            sceneStats.lastFrameFlushBlocked = m_frameNumber;
        }
    }

    void RendererStatistics::flushApplyInterrupted(SceneId sceneId)
    {
        m_sceneStatistics[sceneId].numFlushApplyInterrupted++;
    }

    void RendererStatistics::untrackScene(SceneId sceneId)
    {
        m_sceneStatistics.erase(sceneId);
    }

    void RendererStatistics::untrackOffscreenBuffer(DisplayHandle displayHandle, DeviceResourceHandle offscreenBuffer)
    {
        m_displayStatistics[displayHandle].offscreenBufferStatistics.erase(offscreenBuffer);
    }

    void RendererStatistics::untrackStreamTexture(StreamTextureSourceId sourceId)
    {
        m_streamTextureStatistics.erase(sourceId);
    }

    void RendererStatistics::frameFinished(UInt32 drawCalls)
    {
        const UInt64 currTick = PlatformTime::GetMicrosecondsMonotonic();
        const UInt32 frameDuration = static_cast<UInt32>(currTick - m_lastFrameTick);
        if (frameDuration < m_frameDurationMin)
            m_frameDurationMin = frameDuration;
        else if (frameDuration > m_frameDurationMax)
            m_frameDurationMax = frameDuration;

        // update 'gap' measurements - max number of consecutive frames with some action happening or not
        for (auto& sceneStat : m_sceneStatistics)
        {
            if (m_frameNumber > sceneStat.second.lastFrameFlushApplied)
                sceneStat.second.maxFramesWithNoFlushApplied = std::max<UInt>(sceneStat.second.maxFramesWithNoFlushApplied, m_frameNumber - sceneStat.second.lastFrameFlushApplied);
        }

        for (auto& strTexStat : m_streamTextureStatistics)
        {
            if (m_frameNumber > strTexStat.second.lastFrameUpdated)
                strTexStat.second.maxFramesWithNoUpdate = std::max<UInt>(strTexStat.second.maxFramesWithNoUpdate, m_frameNumber - strTexStat.second.lastFrameUpdated);
        }

        m_frameNumber++;
        m_drawCalls += drawCalls;

        m_lastFrameTick = currTick;
    }

    void RendererStatistics::reset()
    {
        m_timeBase = PlatformTime::GetMillisecondsMonotonic();
        m_frameNumber = 0;
        m_drawCalls = 0u;
        m_frameDurationMin = std::numeric_limits<UInt32>::max();
        m_frameDurationMax = 0u;
        m_clientResourcesUploaded = 0u;
        m_clientResourcesBytesUploaded = 0u;
        m_shadersCompiled = 0u;
        m_microsecondsForShaderCompilation = 0u;

        for (auto& sceneStatIt : m_sceneStatistics)
        {
            auto& sceneStat = sceneStatIt.second;
            sceneStat.numFlushesArrived = 0u;
            sceneStat.numFlushesApplied = 0u;
            sceneStat.numFramesWhereFlushArrived = 0u;
            sceneStat.numFramesWhereFlushApplied = 0u;
            sceneStat.numFramesWhereFlushBlocked = 0u;
            sceneStat.numFlushApplyInterrupted = 0u;
            sceneStat.maxFramesWithNoFlushApplied = 0u;
            sceneStat.maxConsecutiveFramesBlocked = 0u;
            sceneStat.lastFrameFlushArrived = -1; // treat as if arrived in previous period's last frame (ie. do not measure 'arrive gaps' across periods)
            sceneStat.lastFrameFlushApplied = -1; // treat as if applied in previous period's last frame (ie. do not measure 'apply gaps' across periods)
            sceneStat.lastFrameFlushBlocked = std::numeric_limits<Int32>::min(); // treat as if previous period did not end with blocked flush (ie. do not measure 'consecutive blocks' across periods)
            sceneStat.numSceneActionsPerFlush.reset();
            sceneStat.numClientResourcesAddedPerFlush.reset();
            sceneStat.numClientResourcesRemovedPerFlush.reset();
            sceneStat.numSceneResourceActionsPerFlush.reset();
            sceneStat.sceneResourcesUploaded = 0u;
            sceneStat.sceneResourcesBytesUploaded = 0u;
            sceneStat.numRendered = 0u;
        }

        for (auto& dispStat : m_displayStatistics)
        {
            dispStat.second.numFrameBufferSwapped = 0u;
            for (auto& obStat : dispStat.second.offscreenBufferStatistics)
            {
                obStat.second.numSwapped = 0u;
                obStat.second.numInterrupted = 0u;
            }
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
        if (m_clientResourcesUploaded > 0u)
            str << ", clientResUploaded " << m_clientResourcesUploaded << " (" << m_clientResourcesBytesUploaded << " B)";
        if (m_shadersCompiled > 0u)
            str << ", shadersCompiled " << m_shadersCompiled << " for total ms:" << m_microsecondsForShaderCompilation/1000 ;
        str << "\n";

        for (const auto& dbStat : m_displayStatistics)
        {
            str << "FB" << dbStat.first << ": " << dbStat.second.numFrameBufferSwapped;
            for (const auto& obStat : dbStat.second.offscreenBufferStatistics)
            {
                str << "; OB" << obStat.first << ": " << obStat.second.numSwapped;
                if (obStat.second.isInterruptible)
                    str << " (intr: " << obStat.second.numInterrupted << ")";
            }
            str << "\n";
        }

        for (const auto& sceneStatsIt : m_sceneStatistics)
        {
            const auto& sceneStats = sceneStatsIt.second;
            const auto& numSceneActionsPerFlush = sceneStats.numSceneActionsPerFlush;
            const auto& numClientResourcesAddedPerFlush = sceneStats.numClientResourcesAddedPerFlush;
            const auto& numClientResourcesRemovedPerFlush = sceneStats.numClientResourcesRemovedPerFlush;
            const auto& numSceneResourceActionsPerFlush = sceneStats.numSceneResourceActionsPerFlush;

            str << "Scene " << sceneStatsIt.first.getValue() << ": ";
            str << "rendered " << sceneStats.numRendered;
            str << ", framesFArrived " << sceneStats.numFramesWhereFlushArrived;
            str << ", framesFApplied " << sceneStats.numFramesWhereFlushApplied;
            str << ", framesFBlocked " << sceneStats.numFramesWhereFlushBlocked;
            str << ", maxFramesWithNoFApplied " << sceneStats.maxFramesWithNoFlushApplied;
            str << ", maxFramesFBlocked " << sceneStats.maxConsecutiveFramesBlocked;
            str << ", FArrived " << sceneStats.numFlushesArrived;
            str << ", FApplied " << sceneStats.numFlushesApplied;
            if (sceneStats.numFlushApplyInterrupted > 0u)
                str << ", FApplyInterrupted " << sceneStats.numFlushApplyInterrupted;
            if (sceneStats.numFlushesArrived > 0u)
            {
                str << ", actions/F (" << numSceneActionsPerFlush.minValue << "/" << numSceneActionsPerFlush.maxValue << "/" << static_cast<float>(numSceneActionsPerFlush.sum) / sceneStats.numFlushesArrived << ")";
                str << ", RC+/F (" << numClientResourcesAddedPerFlush.minValue << "/" << numClientResourcesAddedPerFlush.maxValue << "/" << static_cast<float>(numClientResourcesAddedPerFlush.sum) / sceneStats.numFlushesArrived << ")";
                str << ", RC-/F (" << numClientResourcesRemovedPerFlush.minValue << "/" << numClientResourcesRemovedPerFlush.maxValue << "/" << static_cast<float>(numClientResourcesRemovedPerFlush.sum) / sceneStats.numFlushesArrived << ")";
                str << ", RS/F (" << numSceneResourceActionsPerFlush.minValue << "/" << numSceneResourceActionsPerFlush.maxValue << "/" << static_cast<float>(numSceneResourceActionsPerFlush.sum) / sceneStats.numFlushesArrived << ")";
            }
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
