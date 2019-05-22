//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/SceneExpirationMonitor.h"
#include "RendererLib/RendererScenes.h"
#include "RendererEventCollector.h"
#include "Scene/SceneActionApplier.h"
#include "PlatformAbstraction/PlatformTime.h"

namespace ramses_internal
{
    SceneExpirationMonitor::SceneExpirationMonitor(const RendererScenes& scenes, RendererEventCollector& eventCollector)
        : m_scenes(scenes)
        , m_eventCollector(eventCollector)
    {
    }

    SceneExpirationMonitor::~SceneExpirationMonitor()
    {
        assert(m_sceneTimestamps.count() == 0u);
    }

    void SceneExpirationMonitor::onFlushApplied(SceneId sceneId, FlushTime::Clock::time_point expirationTimestamp, SceneVersionTag versionTag, UInt64 flushIndex)
    {
        TimeStampTag& ts = m_sceneTimestamps[sceneId].expirationTSOfLastAppliedFlush;
        ts.ts = expirationTimestamp;
        ts.tag = versionTag;
        ts.internalIndex = flushIndex;
    }

    void SceneExpirationMonitor::onRendered(SceneId sceneId)
    {
        auto& timestamps = m_sceneTimestamps[sceneId];
        timestamps.expirationTSOfRenderedScene = timestamps.expirationTSOfLastAppliedFlush;
    }

    void SceneExpirationMonitor::onHidden(SceneId sceneId)
    {
        m_sceneTimestamps[sceneId].expirationTSOfRenderedScene = {};
    }

    void SceneExpirationMonitor::checkExpiredScenes(FlushTime::Clock::time_point currentTime)
    {
        const auto isTSExpired = [currentTime](FlushTime::Clock::time_point ts)  { return ts != FlushTime::InvalidTimestamp && currentTime > ts; };

        for (const auto& sceneIt : m_scenes)
        {
            const SceneId sceneId = sceneIt.key;
            bool expired = false;
            auto& timestamps = m_sceneTimestamps[sceneId];

            if (isTSExpired(timestamps.expirationTSOfRenderedScene.ts))
            {
                const UInt64 expirationTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(timestamps.expirationTSOfRenderedScene.ts.time_since_epoch()).count();
                const UInt64 expirationDelay = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - timestamps.expirationTSOfRenderedScene.ts).count();
                LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: Content of rendered scene " << sceneId << " is expired (version tag of rendered scene " << timestamps.expirationTSOfRenderedScene.tag << "). "
                    << "Expiration time stamp " << expirationTimestamp << " ms, "
                    << "expired by " << expirationDelay << " ms. "
                    << "Internal flush index " << timestamps.expirationTSOfRenderedScene.internalIndex);
                expired = true;
            }

            if (isTSExpired(timestamps.expirationTSOfLastAppliedFlush.ts))
            {
                const UInt64 expirationTimestamp = std::chrono::duration_cast<std::chrono::milliseconds>(timestamps.expirationTSOfLastAppliedFlush.ts.time_since_epoch()).count();
                const UInt64 expirationDelay = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - timestamps.expirationTSOfLastAppliedFlush.ts).count();
                LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: Flush applied to scene " << sceneId << " is expired (version tag of applied flush " << timestamps.expirationTSOfLastAppliedFlush.tag << "). "
                    << "Expiration time stamp " << expirationTimestamp << " ms, " << "expired by " << expirationDelay << " ms."
                    << "Internal flush index " << timestamps.expirationTSOfLastAppliedFlush.internalIndex);
                expired = true;
            }

            if (!sceneIt.value.stagingInfo->pendingFlushes.empty()) // early out if no pending flushes
            {
                const auto& pendingFlushes = sceneIt.value.stagingInfo->pendingFlushes;
                const auto& lastPendingFlush = pendingFlushes.back();
                const auto& lastPendingTimeInfo = lastPendingFlush.timeInfo;

                const bool lastPendingFlushExpired = isTSExpired(lastPendingTimeInfo.expirationTimestamp);
                if (expired || lastPendingFlushExpired) // early out if nothing expired
                {
                    const UInt64 expirationTimestampPendingFlush = asMilliseconds(lastPendingTimeInfo.expirationTimestamp);
                    const UInt64 internalTimestampPendingFlush = asMilliseconds(lastPendingTimeInfo.internalTimestamp);

                    if (lastPendingFlushExpired)
                    {
                        const UInt64 expirationDelayPendingFlush = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastPendingTimeInfo.expirationTimestamp).count();
                        LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: Latest pending flush of scene " << sceneId << " is expired (version tag of this flush " << lastPendingFlush.versionTag << "). "
                            << "Expiration time stamp " << expirationTimestampPendingFlush << " ms, "
                            << "expired by " << expirationDelayPendingFlush << " ms. "
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
            else if (expired)
                LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: there are no pending flushes for the expired scene " << sceneId);

            // report event only if state changed from last time
            if (expired != timestamps.inExpiredState)
                m_eventCollector.addEvent(expired ? ERendererEventType_SceneExpired : ERendererEventType_SceneRecoveredFromExpiration, sceneId);
            timestamps.inExpiredState = expired;
        }
    }

    void SceneExpirationMonitor::stopMonitoringScene(SceneId sceneId)
    {
        m_sceneTimestamps.remove(sceneId);
    }

    FlushTime::Clock::time_point SceneExpirationMonitor::getExpirationTimestampOfRenderedScene(SceneId sceneId) const
    {
        auto it = m_sceneTimestamps.find(sceneId);
        return it != m_sceneTimestamps.end() ? it->value.expirationTSOfRenderedScene.ts : FlushTime::InvalidTimestamp;
    }
}
