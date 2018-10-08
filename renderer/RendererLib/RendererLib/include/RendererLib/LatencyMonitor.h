//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LATENCYMONITOR_H
#define RAMSES_LATENCYMONITOR_H

#include "SceneAPI/SceneId.h"
#include "Components/FlushTimeInformation.h"
#include "Collections/HashMap.h"
#include <chrono>

namespace ramses_internal
{
    class RendererEventCollector;

    class LatencyMonitor
    {
    public:
        using Clock = FlushTimeClock;

        LatencyMonitor(RendererEventCollector& eventCollector);
        ~LatencyMonitor();

        void onFlushApplied(SceneId sceneId, Clock::time_point flushTimeStamp, Clock::duration latencyLimit);
        void onRendered(SceneId sceneId);
        void checkLatency(Clock::time_point currentTime);

        bool isMonitoringScene(SceneId sceneId);
        void stopMonitoringScene(SceneId sceneId);

    private:
        struct SceneLatencyInfo
        {
            Clock::time_point lastAppliedFlushTimeStamp;
            Clock::time_point lastRenderedFlushTimeStamp;
            Clock::duration latencyLimit;
            bool inExceededState;
        };
        HashMap<SceneId, SceneLatencyInfo> m_sceneLatencyInfos;

        RendererEventCollector& m_eventCollector;
    };
}

#endif
