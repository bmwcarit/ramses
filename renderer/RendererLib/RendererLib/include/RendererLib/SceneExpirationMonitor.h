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
#include "SceneAPI/SceneVersionTag.h"
#include "RendererLib/StagingInfo.h"
#include "Components/FlushTimeInformation.h"
#include <chrono>
#include <unordered_map>

namespace ramses_internal
{
    class RendererScenes;
    class RendererEventCollector;
    class RendererStatistics;

    class SceneExpirationMonitor
    {
    public:
        SceneExpirationMonitor(const RendererScenes& scenes, RendererEventCollector& eventCollector, RendererStatistics& statistics);
        ~SceneExpirationMonitor();

        void onFlushApplied(SceneId sceneId, FlushTime::Clock::time_point expirationTimestamp, SceneVersionTag versionTag, uint64_t flushIndex);
        void onRendered(SceneId sceneId);
        void onHidden(SceneId sceneId);
        void checkExpiredScenes(FlushTime::Clock::time_point currentTime);

        void onDestroyed(SceneId sceneId);

        [[nodiscard]] FlushTime::Clock::time_point getExpirationTimestampOfRenderedScene(SceneId sceneId) const;

    private:
        struct TimeStampTag
        {
            FlushTime::Clock::time_point ts = FlushTime::InvalidTimestamp;
            SceneVersionTag tag;
            uint64_t internalIndex = std::numeric_limits<uint64_t>::max();
        };

        struct SceneTimestamps
        {
            TimeStampTag expirationTSOfLastAppliedFlush;
            TimeStampTag expirationTSOfRenderedScene;
            bool inExpiredState = false;
        };

        void checkAndTriggerExpirationEvent(SceneId sceneId, SceneTimestamps& timestamps, bool expired);
        std::unordered_map<SceneId, SceneTimestamps> m_monitoredScenes;

        const RendererScenes& m_scenes;
        RendererEventCollector& m_eventCollector;
        RendererStatistics& m_statistics;
    };
}

#endif
