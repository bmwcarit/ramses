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

    void SceneExpirationMonitor::onFlushApplied(SceneId sceneId, FlushTime::Clock::time_point expirationTimestamp)
    {
        m_sceneTimestamps[sceneId].expirationTSOfAppliedFlushesSinceRender.push_back(expirationTimestamp);
    }

    void SceneExpirationMonitor::onRendered(SceneId sceneId)
    {
        // take last applied flush TS as new rendered TS and clear applied flushes
        auto& timestamps = m_sceneTimestamps[sceneId];
        auto& appliedFlushesTS = timestamps.expirationTSOfAppliedFlushesSinceRender;
        if (!appliedFlushesTS.empty())
        {
            timestamps.expirationTSOfRenderedScene = appliedFlushesTS.back();
            appliedFlushesTS.clear();
        }
    }

    void SceneExpirationMonitor::checkExpiredScenes(FlushTime::Clock::time_point currentTime)
    {
        const auto isTSExpired = [currentTime](FlushTime::Clock::time_point ts)  { return ts != FlushTime::InvalidTimestamp && currentTime > ts; };
        const auto isPendingFlushExpired = [isTSExpired](const PendingFlush& pf) { return isTSExpired(pf.timeInfo.expirationTimestamp); };

        for (const auto& sceneIt : m_scenes)
        {
            const SceneId sceneId = sceneIt.key;
            bool expired = false;

            auto& timestamps = m_sceneTimestamps[sceneId];
            if (isTSExpired(timestamps.expirationTSOfRenderedScene))
            {
                LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: Content of rendered scene " << sceneId << " is expired.");
                expired = true;
            }

            if (std::any_of(timestamps.expirationTSOfAppliedFlushesSinceRender.cbegin(), timestamps.expirationTSOfAppliedFlushesSinceRender.cend(), isTSExpired))
            {
                LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: One or more of flushes applied to scene " << sceneId << " is expired.");
                expired = true;
            }

            const auto& pendingFlushes = sceneIt.value.stagingInfo->pendingFlushes;
            if (std::any_of(pendingFlushes.cbegin(), pendingFlushes.cend(), isPendingFlushExpired))
            {
                LOG_ERROR(CONTEXT_RENDERER, "SceneExpirationMonitor: One or more of pending flushes of scene " << sceneId << " is expired.");
                expired = true;
            }

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
        return it != m_sceneTimestamps.end() ? it->value.expirationTSOfRenderedScene : FlushTime::InvalidTimestamp;
    }
}
