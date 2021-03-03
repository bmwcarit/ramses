//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/SceneExpirationMonitor.h"
#include "RendererLib/RendererScenes.h"
#include "RendererEventCollector.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "Utils/ThreadLocalLog.h"

using namespace testing;
using namespace ramses_internal;

class ASceneExpirationMonitor : public ::testing::Test
{
public:
    ASceneExpirationMonitor()
        : rendererScenes(eventCollector)
        , expirationMonitor(rendererScenes, eventCollector)
    {
        // caller is expected to have a display prefix for logs
        ThreadLocalLog::SetPrefix(1);

        rendererScenes.createScene(SceneInfo{ scene1 });
        rendererScenes.createScene(SceneInfo{ scene2 });
        rendererScenes.createScene(SceneInfo{ scene3 });
        rendererScenes.createScene(SceneInfo{ scene4 });
        rendererScenes.createScene(SceneInfo{ scene5 });
    }

    ~ASceneExpirationMonitor()
    {
        expirationMonitor.onDestroyed(scene1);
        expirationMonitor.onDestroyed(scene2);
        expirationMonitor.onDestroyed(scene3);
        expirationMonitor.onDestroyed(scene4);
        expirationMonitor.onDestroyed(scene5);
        rendererScenes.destroyScene(scene1);
        rendererScenes.destroyScene(scene2);
        rendererScenes.destroyScene(scene3);
        rendererScenes.destroyScene(scene4);
        rendererScenes.destroyScene(scene5);
        expectNoEvent();
    }

protected:
    void expectNoEvent()
    {
        RendererEventVector events;
        RendererEventVector dummy;
        eventCollector.appendAndConsumePendingEvents(dummy, events);
        EXPECT_TRUE(events.empty());
        for (const auto& e : events)
        {
            EXPECT_EQ(ERendererEventType::Invalid, e.eventType);
            EXPECT_EQ(0u, e.sceneId.getValue());
        }
    }

    void expectEvents(std::initializer_list< std::pair<SceneId, ERendererEventType> > expectedEvents)
    {
        RendererEventVector events;
        RendererEventVector dummy;
        eventCollector.appendAndConsumePendingEvents(dummy, events);
        EXPECT_EQ(expectedEvents.size(), events.size());
        for (const auto& expectedEvent : expectedEvents)
        {
            auto matchesEvent = [expectedEvent](const RendererEvent& e) { return e.sceneId == expectedEvent.first && e.eventType == expectedEvent.second; };
            auto it = std::find_if(events.begin(), events.end(), matchesEvent);
            ASSERT_TRUE(it != events.end());
            events.erase(it);
        }
    }

    RendererEventCollector eventCollector;
    RendererScenes         rendererScenes;
    SceneExpirationMonitor expirationMonitor;

    const FlushTime::Clock::time_point currentFakeTime{ std::chrono::milliseconds(10000) };

    const SceneId scene1{ 22u };
    const SceneId scene2{ 23u };
    const SceneId scene3{ 24u };
    const SceneId scene4{ 25u };
    const SceneId scene5{ 26u };
};

TEST_F(ASceneExpirationMonitor, reportInvalidTSForScenesWithNoExpiration)
{
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene3));
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene4));
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene5));
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, doesNotCheckAnythingIfNoScenesMonitored)
{
    expirationMonitor.checkExpiredScenes(currentFakeTime);
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, doesNotGenerateEventIfZeroLimit)
{
    expirationMonitor.onFlushApplied(scene1, FlushTime::InvalidTimestamp, {}, 0);
    expirationMonitor.onFlushApplied(scene2, FlushTime::InvalidTimestamp, {}, 0);
    expirationMonitor.checkExpiredScenes(currentFakeTime);
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, TSFromFlushAppliedDoesNotAffectRenderedTS)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime, {}, 0);
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::hours(1), {}, 0);
    expirationMonitor.onFlushApplied(scene2, currentFakeTime, {}, 0);
    expirationMonitor.onFlushApplied(scene2, currentFakeTime + std::chrono::hours(2), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled }, { scene2, ERendererEventType::SceneExpirationMonitoringEnabled } });
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));
}

TEST_F(ASceneExpirationMonitor, lastTSFromFlushAppliedBecomesRenderedTSWhenSceneReportedRendered)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime, {}, 0);
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::hours(1), {}, 0);
    expirationMonitor.onFlushApplied(scene2, currentFakeTime, {}, 0);
    expirationMonitor.onFlushApplied(scene2, currentFakeTime + std::chrono::hours(2), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled }, { scene2, ERendererEventType::SceneExpirationMonitoringEnabled } });
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));

    expirationMonitor.onRendered(scene1);
    EXPECT_EQ(currentFakeTime + std::chrono::hours(1), expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));

    expirationMonitor.onRendered(scene2);
    EXPECT_EQ(currentFakeTime + std::chrono::hours(1), expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(currentFakeTime + std::chrono::hours(2), expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));

    expirationMonitor.onRendered(scene1);
    expirationMonitor.onRendered(scene2);
    EXPECT_EQ(currentFakeTime + std::chrono::hours(1), expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(currentFakeTime + std::chrono::hours(2), expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));
}

TEST_F(ASceneExpirationMonitor, renderedTSIsResetIfSceneHidden)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime, {}, 0);
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::hours(1), {}, 0);
    expirationMonitor.onFlushApplied(scene2, currentFakeTime, {}, 0);
    expirationMonitor.onFlushApplied(scene2, currentFakeTime + std::chrono::hours(2), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled }, { scene2, ERendererEventType::SceneExpirationMonitoringEnabled } });
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));

    expirationMonitor.onRendered(scene1);
    expirationMonitor.onRendered(scene2);
    EXPECT_EQ(currentFakeTime + std::chrono::hours(1), expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(currentFakeTime + std::chrono::hours(2), expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));

    expirationMonitor.onHidden(scene1);
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(currentFakeTime + std::chrono::hours(2), expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));

    expirationMonitor.onHidden(scene2);
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));
}

TEST_F(ASceneExpirationMonitor, renderedTSIsTakenFromLastAppliedFlushIfSceneRenderedAgainAfterHidden)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime, {}, 0);
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::hours(1), {}, 0);
    expirationMonitor.onFlushApplied(scene2, currentFakeTime, {}, 0);
    expirationMonitor.onFlushApplied(scene2, currentFakeTime + std::chrono::hours(2), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled }, { scene2, ERendererEventType::SceneExpirationMonitoringEnabled } });
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));

    expirationMonitor.onRendered(scene1);
    expirationMonitor.onRendered(scene2);
    EXPECT_EQ(currentFakeTime + std::chrono::hours(1), expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(currentFakeTime + std::chrono::hours(2), expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));

    expirationMonitor.onHidden(scene1);
    expirationMonitor.onHidden(scene2);
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));

    // if rendered again, TS becomes valid - taken from last applied flush
    expirationMonitor.onRendered(scene1);
    expirationMonitor.onRendered(scene2);
    EXPECT_EQ(currentFakeTime + std::chrono::hours(1), expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));
    EXPECT_EQ(currentFakeTime + std::chrono::hours(2), expirationMonitor.getExpirationTimestampOfRenderedScene(scene2));
}

TEST_F(ASceneExpirationMonitor, doesNotGenerateEventIfLimitNotExceeded)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime);
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, AlwaysExpiresIfZeroTimeComesFromClock)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime, {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);

    // Zero time comes when PTP is broken or has lost sync for too long
    expirationMonitor.checkExpiredScenes( FlushTime::Clock::time_point{ std::chrono::milliseconds(0) });
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });
}

TEST_F(ASceneExpirationMonitor, canHandleTimeStampInFutureAndTreatsAsNotExceeded)
{
    expirationMonitor.onFlushApplied(scene1, FlushTime::Clock::time_point(currentFakeTime + std::chrono::hours(100)) + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime);
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, generatesEventIfAppliedFlushTSExpired)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime, {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(1));
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });
}

TEST_F(ASceneExpirationMonitor, generatesEventIfLastAppliedFlushTSExpired)
{
    expirationMonitor.onFlushApplied(scene1, FlushTime::InvalidTimestamp, {}, 0);
    expirationMonitor.onFlushApplied(scene1, currentFakeTime, {}, 0);
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::hours(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(1));
    expectNoEvent();

    expirationMonitor.onFlushApplied(scene1, currentFakeTime, {}, 0);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(1));
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });
}

TEST_F(ASceneExpirationMonitor, generatesEventIfRenderedTSExpired)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime, {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(1));
    expectEvents({ {scene1, ERendererEventType::SceneExpired} });
}

TEST_F(ASceneExpirationMonitor, generatesEventIfLastAppliedFlushTSExpiredEvenIfRenderedTSNotExpired)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::hours(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);

    expirationMonitor.onFlushApplied(scene1, currentFakeTime, {}, 0);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(1));
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });
}

TEST_F(ASceneExpirationMonitor, generatesEventIfLastPendingFlushExpired)
{
    // enable monitoring by applying at least one flush with timestamp
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::hours(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });

    // create some pending flushes
    auto& pendingFlushes = rendererScenes.getStagingInfo(scene1).pendingData.pendingFlushes;
    pendingFlushes.resize(3u);
    pendingFlushes[0].timeInfo.expirationTimestamp = FlushTime::InvalidTimestamp;
    pendingFlushes[1].timeInfo.expirationTimestamp = currentFakeTime;
    pendingFlushes[2].timeInfo.expirationTimestamp = currentFakeTime + std::chrono::hours(1);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(1));
    expectNoEvent();

    pendingFlushes.push_back({});
    pendingFlushes[3].timeInfo.expirationTimestamp = currentFakeTime;
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(1));
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });
}

TEST_F(ASceneExpirationMonitor, generatesEventIfAnyOfPendingFlushesExpiredEvenIfAppliedAndRenderedNotExpired)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::hours(1), {}, 0);
    expirationMonitor.onRendered(scene1);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });

    // create some pending flushes
    auto& pendingFlushes = rendererScenes.getStagingInfo(scene1).pendingData.pendingFlushes;
    pendingFlushes.resize(3u);
    pendingFlushes[0].timeInfo.expirationTimestamp = FlushTime::InvalidTimestamp;
    pendingFlushes[1].timeInfo.expirationTimestamp = currentFakeTime;
    pendingFlushes[2].timeInfo.expirationTimestamp = currentFakeTime + std::chrono::hours(1);
    expectNoEvent();

    pendingFlushes.push_back({});
    pendingFlushes[3].timeInfo.expirationTimestamp = currentFakeTime;
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(1));
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });
}

TEST_F(ASceneExpirationMonitor, generatesEventOnlyOnceIfExpiredAndCheckedMultipleTimesWithSingleApplyOnly)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(2));
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });

    // still in exceeded state, no more events
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(2));
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(3));
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(4));
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(5));
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, evaluatesExpirationBasedOnLimitFromLastAppliedFlush)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(10000), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(10));
    expectNoEvent();

    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expirationMonitor.onRendered(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(100));
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });
}

TEST_F(ASceneExpirationMonitor, generatesEventForAllScenesWhereAppropriate)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expirationMonitor.onFlushApplied(scene2, currentFakeTime + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onFlushApplied(scene3, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled }, { scene2, ERendererEventType::SceneExpirationMonitoringEnabled }, { scene3, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    expirationMonitor.onRendered(scene2);
    expirationMonitor.onRendered(scene3);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(2));
    expectEvents({ { scene1, ERendererEventType::SceneExpired },{ scene3, ERendererEventType::SceneExpired } });
}

TEST_F(ASceneExpirationMonitor, generatesEventOnlyOnceIfSceneKeepsBeingExpired)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(2));
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });

    for (size_t i = 0; i < 5u; ++i)
    {
        expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(i) + std::chrono::milliseconds(1), {}, 0);
        expirationMonitor.onRendered(scene1);
        expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(i + 2));
    }
    // is still exceeded
    expirationMonitor.onRendered(scene1);
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, generatesRecoveryEventWhenExceedingStops)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(2));
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });

    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onRendered(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(3));
    expectEvents({ { scene1, ERendererEventType::SceneRecoveredFromExpiration } });
}

TEST_F(ASceneExpirationMonitor, generatesRecoveryEventOnlyOnceWhenExceedingStopsAndKeepsBelowLimit)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(2));
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });

    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onRendered(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(3));
    expectEvents({ { scene1, ERendererEventType::SceneRecoveredFromExpiration } });

    for (size_t i = 0; i < 5u; ++i)
    {
        expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(i) + std::chrono::milliseconds(10000), {}, 0);
        expirationMonitor.onRendered(scene1);
        expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(i + 10));
    }
    // is still below limit
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, willReportSceneThatKeepsBeingFlushedButNotRendered)
{
    // scene must be rendered at least once with valid expiration, only then it can expire
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expirationMonitor.onRendered(scene1);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });

    RendererEventVector events;
    for (size_t i = 0; i < 5; ++i)
    {
        expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(i) + std::chrono::milliseconds(2), {}, 0);
        expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(i + 1));
    }
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });
}

TEST_F(ASceneExpirationMonitor, willNotReportSceneThatKeepsBeingFlushedButHidden)
{
    // scene must be rendered at least once with valid expiration, only then it can expire
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expirationMonitor.onRendered(scene1);
    expirationMonitor.onHidden(scene1);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });

    RendererEventVector events;
    for (size_t i = 0; i < 5; ++i)
    {
        expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(i) + std::chrono::milliseconds(2), {}, 0);
        expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(i + 1));
    }
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, willReportSceneThatKeepsBeingRenderedButNotFlushed)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(2), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });

    RendererEventVector events;
    for (size_t i = 0; i < 5; ++i)
    {
        expirationMonitor.onRendered(scene1);
        expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(i));
    }
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });
}

TEST_F(ASceneExpirationMonitor, doesNotGenerateEventIfLastRenderedTSExpiredButSceneHidden)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime);
    expectNoEvent();

    expirationMonitor.onHidden(scene1);

    // keep flushing up-to-date, no rendering
    for (int i = 0; i < 5; ++i)
    {
        expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1 + i), {}, 0);
        expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(i));
    }
    // if renderedTS would be checked there would be expiration event
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, generatesRecoveryEventIfRenderedTSExpiredButSceneGetsHiddenAndNewFlushesArrive)
{
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(2));
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });

    // hiding alone will not cause recovery - last applied flush still expired
    expirationMonitor.onHidden(scene1);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(2));
    expectNoEvent();

    // keep scene hidden and apply new up-to-date flush
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(5), {}, 0);
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(3));
    expectEvents({ { scene1, ERendererEventType::SceneRecoveredFromExpiration } });
}

TEST_F(ASceneExpirationMonitor, confidenceTest_checkingOfOneSceneDoesNotAffectCheckingOfOtherScene)
{
    // flush 1
    expirationMonitor.onFlushApplied(scene1, FlushTime::Clock::now() - std::chrono::milliseconds(10) + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });

    // few cycles of scene 2
    expirationMonitor.onFlushApplied(scene2, FlushTime::Clock::now() - std::chrono::milliseconds(10) + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene2, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene2);
    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    expectEvents({ { scene1, ERendererEventType::SceneExpired },{ scene2, ERendererEventType::SceneExpired } });
    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    expirationMonitor.onFlushApplied(scene2, FlushTime::Clock::now() - std::chrono::milliseconds(10) + std::chrono::milliseconds(1), {}, 0);
    expirationMonitor.onRendered(scene2);
    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    expirationMonitor.onFlushApplied(scene2, FlushTime::Clock::now() - std::chrono::milliseconds(10) + std::chrono::milliseconds(1), {}, 0);
    expirationMonitor.onRendered(scene2);
    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    expectNoEvent();

    // both exceeded, get both back to normal
    expirationMonitor.onFlushApplied(scene1, FlushTime::Clock::now() + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onFlushApplied(scene2, FlushTime::Clock::now() + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onRendered(scene1);
    expirationMonitor.onRendered(scene2);
    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    expectEvents({ { scene1, ERendererEventType::SceneRecoveredFromExpiration },{ scene2, ERendererEventType::SceneRecoveredFromExpiration } });

    expirationMonitor.onFlushApplied(scene1, FlushTime::Clock::now() + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onFlushApplied(scene2, FlushTime::Clock::now() + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onRendered(scene1);
    expirationMonitor.onRendered(scene2);
    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    // both still below limit
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, confidenceTest_severalScenesWithDifferentBehaviorAndLimits)
{
    //scene1 checking enabled, sometimes exceeds
    //scene2 checking disabled
    //scene3 checking enabled, never exceeds
    //scene4 checking enabled, always exceeds
    //scene5 checking enabled, sometimes exceeds

    // 'randomize' times of apply/render/checks

    expirationMonitor.onFlushApplied(scene1, FlushTime::Clock::now() - std::chrono::milliseconds(10) + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onFlushApplied(scene3, FlushTime::Clock::now() + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onFlushApplied(scene5, FlushTime::Clock::now() - std::chrono::milliseconds(10) + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled }, { scene3, ERendererEventType::SceneExpirationMonitoringEnabled }, { scene5, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene2);
    expirationMonitor.onRendered(scene5);

    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    expectEvents({ {scene5, ERendererEventType::SceneExpired} });

    expirationMonitor.onFlushApplied(scene4, FlushTime::Clock::now() - std::chrono::milliseconds(10) + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene4, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    expirationMonitor.onRendered(scene3);

    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    expectEvents({ { scene4, ERendererEventType::SceneExpired } });

    expirationMonitor.onFlushApplied(scene5, FlushTime::Clock::now() + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onFlushApplied(scene1, FlushTime::Clock::now() - std::chrono::milliseconds(10) + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onRendered(scene2);
    expirationMonitor.onRendered(scene4);
    expirationMonitor.onRendered(scene5);

    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    expectEvents({ { scene5, ERendererEventType::SceneRecoveredFromExpiration } });

    expirationMonitor.onFlushApplied(scene1, FlushTime::Clock::now() - std::chrono::milliseconds(10) + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onFlushApplied(scene3, FlushTime::Clock::now() + std::chrono::milliseconds(10000), {}, 0);
    expirationMonitor.onFlushApplied(scene4, FlushTime::Clock::now() - std::chrono::milliseconds(10) + std::chrono::milliseconds(1), {}, 0);
    expirationMonitor.onRendered(scene1);
    expirationMonitor.onRendered(scene3);
    expirationMonitor.onRendered(scene5);

    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    expectNoEvent();

    expirationMonitor.onFlushApplied(scene1, FlushTime::Clock::now() - std::chrono::milliseconds(10) + std::chrono::milliseconds(1), {}, 0);
    expirationMonitor.onRendered(scene1);

    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });

    expirationMonitor.checkExpiredScenes(FlushTime::Clock::now());
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, stopsMonitoringSceneOnDestroy)
{
    RendererEventVector events;
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    expirationMonitor.onDestroyed(scene1);

    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));

    // this would trigger exceeded event if still monitored
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(10));
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, stopsMonitoringSceneOnInvalidTS)
{
    RendererEventVector events;
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);

    // disable
    expirationMonitor.onFlushApplied(scene1, FlushTime::InvalidTimestamp, {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringDisabled } });

    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));

    // this would trigger exceeded event if still monitored
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(10));
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, stopsMonitoringSceneOnInvalidTS_alreadyExpired)
{
    RendererEventVector events;
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);
    // expire
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(10));
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });

    // disable
    expirationMonitor.onFlushApplied(scene1, FlushTime::InvalidTimestamp, {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringDisabled } });

    EXPECT_EQ(FlushTime::InvalidTimestamp, expirationMonitor.getExpirationTimestampOfRenderedScene(scene1));

    // this would trigger exceeded event if still monitored
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(10));
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, stopsMonitoringSceneOnInvalidTS_withExpiredPendingFlush)
{
    RendererEventVector events;
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);

    // disable
    expirationMonitor.onFlushApplied(scene1, FlushTime::InvalidTimestamp, {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringDisabled } });

    // create expired pending flush
    auto& pendingFlushes = rendererScenes.getStagingInfo(scene1).pendingData.pendingFlushes;
    pendingFlushes.resize(1u);
    pendingFlushes[0].timeInfo.expirationTimestamp = currentFakeTime;
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(1));
    expectNoEvent();
}

TEST_F(ASceneExpirationMonitor, canReenableMonitoringAfterDisable)
{
    RendererEventVector events;
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });
    expirationMonitor.onRendered(scene1);

    // disable
    expirationMonitor.onFlushApplied(scene1, FlushTime::InvalidTimestamp, {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringDisabled } });

    // this would trigger exceeded event if still monitored
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(10));
    expectNoEvent();

    // re-enable
    expirationMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(1), {}, 0);
    expectEvents({ { scene1, ERendererEventType::SceneExpirationMonitoringEnabled } });

    // this would trigger exceeded event if still monitored
    expirationMonitor.checkExpiredScenes(currentFakeTime + std::chrono::milliseconds(10));
    expectEvents({ { scene1, ERendererEventType::SceneExpired } });
}
