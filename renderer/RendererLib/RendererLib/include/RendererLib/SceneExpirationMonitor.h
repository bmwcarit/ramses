//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_SCENEEXPIRATIONMONITOR_H
#define RAMSES_SCENEEXPIRATIONMONITOR_H

#include "SceneAPI/SceneId.h"
#include "Components/FlushTimeInformation.h"
#include "Collections/HashMap.h"
#include <chrono>

namespace ramses_internal
{
    class RendererScenes;
    class RendererEventCollector;

    class SceneExpirationMonitor
    {
    public:
        SceneExpirationMonitor(const RendererScenes& scenes, RendererEventCollector& eventCollector);
        ~SceneExpirationMonitor();

        void onFlushApplied(SceneId sceneId, FlushTime::Clock::time_point expirationTimestamp);
        void onRendered(SceneId sceneId);
        void checkExpiredScenes(FlushTime::Clock::time_point currentTime);

        void stopMonitoringScene(SceneId sceneId);

        FlushTime::Clock::time_point getExpirationTimestampOfRenderedScene(SceneId sceneId) const;

    private:
        struct SceneTimestamps
        {
            Vector<FlushTime::Clock::time_point> expirationTSOfAppliedFlushesSinceRender;
            FlushTime::Clock::time_point expirationTSOfRenderedScene = FlushTime::InvalidTimestamp;
            bool inExpiredState = false;
        };
        HashMap<SceneId, SceneTimestamps> m_sceneTimestamps;

        const RendererScenes& m_scenes;
        RendererEventCollector& m_eventCollector;
    };
}

#endif
