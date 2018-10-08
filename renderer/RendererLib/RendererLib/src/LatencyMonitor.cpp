//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/LatencyMonitor.h"
#include "RendererEventCollector.h"
#include "PlatformAbstraction/PlatformTime.h"

namespace ramses_internal
{
    LatencyMonitor::LatencyMonitor(RendererEventCollector& eventCollector)
        : m_eventCollector(eventCollector)
    {
    }

    LatencyMonitor::~LatencyMonitor()
    {
        assert(m_sceneLatencyInfos.count() == 0u);
    }

    void LatencyMonitor::onFlushApplied(SceneId sceneId, Clock::time_point flushTimeStamp, Clock::duration latencyLimit)
    {
        auto it = m_sceneLatencyInfos.find(sceneId);
        if (it == m_sceneLatencyInfos.end())
        {
            if (latencyLimit.count() > 0u)
                // init last rendered time stamp to the flush time stamp so that it is not reported as exceeded if not rendered in the first frames
                m_sceneLatencyInfos.put(sceneId, { flushTimeStamp, flushTimeStamp, latencyLimit, false });
        }
        else
        {
            it->value.lastAppliedFlushTimeStamp = flushTimeStamp;
            it->value.latencyLimit = latencyLimit;
        }
    }

    void LatencyMonitor::onRendered(SceneId sceneId)
    {
        auto it = m_sceneLatencyInfos.find(sceneId);
        if (it != m_sceneLatencyInfos.end())
            it->value.lastRenderedFlushTimeStamp = it->value.lastAppliedFlushTimeStamp;
    }

    void LatencyMonitor::checkLatency(Clock::time_point currentTime)
    {
        for (auto& latencyInfoIt : m_sceneLatencyInfos)
        {
            auto& latencyInfo = latencyInfoIt.value;
            if (latencyInfo.latencyLimit.count() > 0u)
            {
                const auto timeDiff = (currentTime > latencyInfo.lastRenderedFlushTimeStamp) ? (currentTime - latencyInfo.lastRenderedFlushTimeStamp) : std::chrono::milliseconds(0);
                const bool exceededLatencyLimit = (timeDiff > latencyInfo.latencyLimit);

                // report event only if state changed from last time
                if (exceededLatencyLimit != latencyInfo.inExceededState)
                    m_eventCollector.addEvent(exceededLatencyLimit ? ERendererEventType_SceneUpdateLatencyExceededLimit : ERendererEventType_SceneUpdateLatencyBackBelowLimit, latencyInfoIt.key);
                latencyInfo.inExceededState = exceededLatencyLimit;
            }
        }
    }

    bool LatencyMonitor::isMonitoringScene(SceneId sceneId)
    {
        return m_sceneLatencyInfos.contains(sceneId);
    }

    void LatencyMonitor::stopMonitoringScene(SceneId sceneId)
    {
        m_sceneLatencyInfos.remove(sceneId);
    }
}
