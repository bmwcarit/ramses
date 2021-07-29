//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/SceneExpirationMonitor.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/RendererStatistics.h"
#include "RendererEventCollector.h"
#include "Scene/SceneActionApplier.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "Utils/ThreadLocalLogForced.h"

namespace ramses_internal
{
    SceneExpirationMonitor::SceneExpirationMonitor(const RendererScenes& scenes, RendererEventCollector& eventCollector, RendererStatistics& statistics)
        : m_scenes(scenes)
        , m_eventCollector(eventCollector)
        , m_statistics(statistics)
    {
    }

    SceneExpirationMonitor::~SceneExpirationMonitor()
    {
        assert(m_monitoredScenes.empty());
    }

    void SceneExpirationMonitor::onFlushApplied(SceneId sceneId, FlushTime::Clock::time_point expirationTimestamp, SceneVersionTag versionTag, UInt64 flushIndex)
    {
        if (expirationTimestamp != FlushTime::InvalidTimestamp)
        {
            if (m_monitoredScenes.count(sceneId) == 0)
            {
                LOG_INFO_P(CONTEXT_RENDERER, "SceneExpirationMonitor: expiration monitoring for scene {} enabled", sceneId);
                m_eventCollector.addSceneExpirationEvent(ERendererEventType::SceneExpirationMonitoringEnabled, sceneId);
            }

            TimeStampTag& ts = m_monitoredScenes[sceneId].expirationTSOfLastAppliedFlush;
            ts.ts = expirationTimestamp;
            ts.tag = versionTag;
            ts.internalIndex = flushIndex;
        }
        else if (m_monitoredScenes.count(sceneId) != 0)
        {
            LOG_INFO_P(CONTEXT_RENDERER, "SceneExpirationMonitor: expiration monitoring for scene {} disabled, last state expired={}", sceneId, m_monitoredScenes[sceneId].inExpiredState);
            m_eventCollector.addSceneExpirationEvent(ERendererEventType::SceneExpirationMonitoringDisabled, sceneId);
            m_monitoredScenes.erase(sceneId);
        }
    }

    void SceneExpirationMonitor::onRendered(SceneId sceneId)
    {
        auto it = m_monitoredScenes.find(sceneId);
        if (it != m_monitoredScenes.end())
            it->second.expirationTSOfRenderedScene = it->second.expirationTSOfLastAppliedFlush;
    }

    void SceneExpirationMonitor::onHidden(SceneId sceneId)
    {
        auto it = m_monitoredScenes.find(sceneId);
        if (it != m_monitoredScenes.end())
            it->second.expirationTSOfRenderedScene = {};
    }

    void SceneExpirationMonitor::checkAndTriggerExpirationEvent(SceneId sceneId, SceneTimestamps& timestamps, bool expired)
    {
        if (expired != timestamps.inExpiredState)
            m_eventCollector.addSceneExpirationEvent(expired ? ERendererEventType::SceneExpired : ERendererEventType::SceneRecoveredFromExpiration, sceneId);

        timestamps.inExpiredState = expired;
    }

    void SceneExpirationMonitor::checkExpiredScenes(FlushTime::Clock::time_point currentTime)
    {
        if (currentTime == FlushTime::InvalidTimestamp) // early out if current time is invalid
        {
            LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: Current time is invalid. This is an error. Marking all monitored scenes as expired.");
            for (auto& it : m_monitoredScenes)
                checkAndTriggerExpirationEvent(it.first, it.second, true);

            return;
        }

        for (auto& it : m_monitoredScenes)
        {
            const SceneId sceneId = it.first;
            bool expired = false;
            auto expDelayRendered = FlushTime::Clock::milliseconds::min();
            auto expDelayApplied = FlushTime::Clock::milliseconds::min();
            auto expDelayPending = FlushTime::Clock::milliseconds::min();
            auto& timestamps = it.second;

            if (timestamps.expirationTSOfRenderedScene.ts != FlushTime::InvalidTimestamp)
            {
                expDelayRendered = std::chrono::duration_cast<FlushTime::Clock::milliseconds>(currentTime - timestamps.expirationTSOfRenderedScene.ts);
                if (expDelayRendered > FlushTime::Clock::milliseconds::zero())
                {
                    const UInt64 expirationTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(timestamps.expirationTSOfRenderedScene.ts.time_since_epoch()).count();
                    LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: Content of rendered scene " << sceneId << " is expired (version tag of rendered scene " << timestamps.expirationTSOfRenderedScene.tag << "). "
                        << "Expiration time stamp " << expirationTimestamp << " ms, " << "expired by " << expDelayRendered.count() << " ms. "
                        << "Internal flush index " << timestamps.expirationTSOfRenderedScene.internalIndex);
                    expired = true;
                }
            }

            if (timestamps.expirationTSOfLastAppliedFlush.ts != FlushTime::InvalidTimestamp)
            {
                expDelayApplied = std::chrono::duration_cast<FlushTime::Clock::milliseconds>(currentTime - timestamps.expirationTSOfLastAppliedFlush.ts);
                if (expDelayApplied > FlushTime::Clock::milliseconds::zero())
                {
                    const UInt64 expirationTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(timestamps.expirationTSOfLastAppliedFlush.ts.time_since_epoch()).count();
                    LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: Flush applied to scene " << sceneId << " is expired (version tag of applied flush " << timestamps.expirationTSOfLastAppliedFlush.tag << "). "
                        << "Expiration time stamp " << expirationTimestamp << " ms, " << "expired by " << expDelayApplied.count() << " ms."
                        << "Internal flush index " << timestamps.expirationTSOfLastAppliedFlush.internalIndex);
                    expired = true;
                }
            }

            const auto& pendingFlushes = m_scenes.getStagingInfo(sceneId).pendingData.pendingFlushes;
            if (!pendingFlushes.empty()) // early out if no pending flushes
            {
                const auto& lastPendingFlush = pendingFlushes.back();
                const auto& lastPendingTimeInfo = lastPendingFlush.timeInfo;

                if (lastPendingTimeInfo.expirationTimestamp != FlushTime::InvalidTimestamp)
                {
                    expDelayPending = std::chrono::duration_cast<FlushTime::Clock::milliseconds>(currentTime - lastPendingTimeInfo.expirationTimestamp);
                    const bool lastPendingFlushExpired = expDelayPending > FlushTime::Clock::milliseconds::zero();
                    if (expired || lastPendingFlushExpired) // early out if nothing expired
                    {
                        const UInt64 expirationTimestampPendingFlush = asMilliseconds(lastPendingTimeInfo.expirationTimestamp);
                        const UInt64 internalTimestampPendingFlush = asMilliseconds(lastPendingTimeInfo.internalTimestamp);

                        if (lastPendingFlushExpired)
                        {
                            LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: Latest pending flush of scene " << sceneId << " is expired (version tag of this flush " << lastPendingFlush.versionTag << "). "
                                << "Expiration time stamp " << expirationTimestampPendingFlush << " ms, "
                                << "expired by " << expDelayPending.count() << " ms. "
                                << "Timestamp of flush creation on client side: " << internalTimestampPendingFlush << " ms. "
                                << "Internal flush index " << lastPendingFlush.flushIndex << ". "
                                << "There is " << pendingFlushes.size() << " pending flushes in total, only latest was checked.");
                            expired = true;
                        }
                        else if (expired)
                        {
                            LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: for the expired scene " << sceneId << " there is pending flush which is not expired "
                                << "Expiration time stamp " << expirationTimestampPendingFlush << " ms, timestamp of flush creation on client side: " << internalTimestampPendingFlush << " ms. "
                                << "Internal flush index " << timestamps.expirationTSOfLastAppliedFlush.internalIndex << ". "
                                << "There is " << pendingFlushes.size() << " pending flushes in total, only latest was checked.");
                        }

                        LOG_INFO_F(CONTEXT_RENDERER, ([&](StringOutputStream& logStream)
                            {
                                logStream << "Pending flushes for expired scene " << sceneId << "[internalIndex, expirationTS, versionTag] : ";
                                for (const auto& pendingFlush : pendingFlushes)
                                    logStream << "[" << pendingFlush.flushIndex << ", " << asMilliseconds(pendingFlush.timeInfo.expirationTimestamp) << ", " << pendingFlush.versionTag << "] ";
                            }));
                    }
                }
            }
            else if (expired)
                LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: there are no pending flushes for the expired scene " << sceneId);

            checkAndTriggerExpirationEvent(sceneId, timestamps, expired);

            // log the worst case to statistics
            auto maxDelay = std::max(expDelayApplied, std::max(expDelayPending, expDelayRendered));
            m_statistics.addExpirationOffset(sceneId, maxDelay.count());
        }
    }

    void SceneExpirationMonitor::onDestroyed(SceneId sceneId)
    {
        if (m_monitoredScenes.erase(sceneId) != 0)
        {
            LOG_INFO_P(CONTEXT_RENDERER, "SceneExpirationMonitor: expiration monitoring for scene {} disabled because scene was unsubscribed from renderer", sceneId);
            m_eventCollector.addSceneExpirationEvent(ERendererEventType::SceneExpirationMonitoringDisabled, sceneId);
        }
    }

    FlushTime::Clock::time_point SceneExpirationMonitor::getExpirationTimestampOfRenderedScene(SceneId sceneId) const
    {
        const auto it = m_monitoredScenes.find(sceneId);
        return it != m_monitoredScenes.cend() ? it->second.expirationTSOfRenderedScene.ts : FlushTime::InvalidTimestamp;
    }
}
