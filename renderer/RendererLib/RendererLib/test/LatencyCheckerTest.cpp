//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/LatencyMonitor.h"
#include "RendererEventCollector.h"
#include "PlatformAbstraction/PlatformThread.h"

using namespace testing;
using namespace ramses_internal;

class ALatencyMonitor : public ::testing::Test
{
public:
    ALatencyMonitor()
        : latencyMonitor(eventCollector)
    {
    }

    ~ALatencyMonitor()
    {
        latencyMonitor.stopMonitoringScene(scene1);
        latencyMonitor.stopMonitoringScene(scene2);
        latencyMonitor.stopMonitoringScene(scene3);
        latencyMonitor.stopMonitoringScene(scene4);
        latencyMonitor.stopMonitoringScene(scene5);
        expectNoEvent();
    }

protected:
    void expectNoEvent()
    {
        RendererEventVector events;
        eventCollector.dispatchEvents(events);
        EXPECT_TRUE(events.empty());
    }

    void expectEvents(std::initializer_list< std::pair<SceneId, ERendererEventType> > expectedEvents)
    {
        RendererEventVector events;
        eventCollector.dispatchEvents(events);
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
    LatencyMonitor latencyMonitor;

    const LatencyMonitor::Clock::time_point currentFakeTime{ std::chrono::milliseconds(10000) };

    const SceneId scene1{ 22u };
    const SceneId scene2{ 23u };
    const SceneId scene3{ 24u };
    const SceneId scene4{ 25u };
    const SceneId scene5{ 26u };
};

TEST_F(ALatencyMonitor, doesNotMonitorAnySceneInitially)
{
    EXPECT_FALSE(latencyMonitor.isMonitoringScene(scene1));
    EXPECT_FALSE(latencyMonitor.isMonitoringScene(scene2));
    EXPECT_FALSE(latencyMonitor.isMonitoringScene(scene3));
    EXPECT_FALSE(latencyMonitor.isMonitoringScene(scene4));
    EXPECT_FALSE(latencyMonitor.isMonitoringScene(scene5));
    expectNoEvent();
}

TEST_F(ALatencyMonitor, initiatesMonitoringOnlyForScenesWithAppliedFlushWithNonZeroLimit)
{
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(1));
    latencyMonitor.onFlushApplied(scene2, currentFakeTime, std::chrono::milliseconds(0));
    latencyMonitor.onFlushApplied(scene4, currentFakeTime, std::chrono::milliseconds(10));

    EXPECT_TRUE(latencyMonitor.isMonitoringScene(scene1));
    EXPECT_FALSE(latencyMonitor.isMonitoringScene(scene2));
    EXPECT_FALSE(latencyMonitor.isMonitoringScene(scene3));
    EXPECT_TRUE(latencyMonitor.isMonitoringScene(scene4));
    EXPECT_FALSE(latencyMonitor.isMonitoringScene(scene5));
    expectNoEvent();
}

TEST_F(ALatencyMonitor, doesNotCheckAnythingIfNoScenesProvidedLatencyInfo)
{
    latencyMonitor.checkLatency(currentFakeTime);
    expectNoEvent();
}

TEST_F(ALatencyMonitor, doesNotGenerateEventIfZeroLimit)
{
    latencyMonitor.onFlushApplied(scene1, currentFakeTime - std::chrono::milliseconds(1), std::chrono::milliseconds(0));
    latencyMonitor.onFlushApplied(scene2, currentFakeTime + std::chrono::milliseconds(1), std::chrono::milliseconds(0));
    latencyMonitor.checkLatency(currentFakeTime);
    expectNoEvent();
}

TEST_F(ALatencyMonitor, doesNotGenerateEventIfLimitNotExceeded)
{
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.checkLatency(currentFakeTime);
    expectNoEvent();
}

TEST_F(ALatencyMonitor, canHandleTimeStampInFutureAndTreatsAsNotExceeded)
{
    latencyMonitor.onFlushApplied(scene1, LatencyMonitor::Clock::time_point(currentFakeTime + std::chrono::hours(100)), std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.checkLatency(currentFakeTime);
    expectNoEvent();
}

TEST_F(ALatencyMonitor, generatesEventIfLimitExceeded)
{
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(2));
    expectEvents({ {scene1, ERendererEventType_SceneUpdateLatencyExceededLimit} });
}

TEST_F(ALatencyMonitor, generatesEventOnlyOnceIfLatencyCheckedMultipleTimesWithSingleApplyOnly)
{
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(2));
    expectEvents({ { scene1, ERendererEventType_SceneUpdateLatencyExceededLimit } });

    // still in exceeded state, no more events
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(2));
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(3));
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(4));
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(5));
    expectNoEvent();
}

TEST_F(ALatencyMonitor, evaluatesLatencyBasedOnLimitFromLastAppliedFlush)
{
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(1));
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(1));
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(10000));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(10));
    expectNoEvent();

    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(10000));
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(10000));
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(100));
    expectEvents({ { scene1, ERendererEventType_SceneUpdateLatencyExceededLimit } });
}

TEST_F(ALatencyMonitor, generatesEventForAllScenesWhereAppropriate)
{
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(1));
    latencyMonitor.onFlushApplied(scene2, currentFakeTime, std::chrono::milliseconds(10000));
    latencyMonitor.onFlushApplied(scene3, currentFakeTime, std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.onRendered(scene2);
    latencyMonitor.onRendered(scene3);
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(2));
    expectEvents({ { scene1, ERendererEventType_SceneUpdateLatencyExceededLimit },{ scene3, ERendererEventType_SceneUpdateLatencyExceededLimit } });
}

TEST_F(ALatencyMonitor, generatesEventOnlyOnceIfSceneLatencyKeepsBeingExceeded)
{
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(2));
    expectEvents({ { scene1, ERendererEventType_SceneUpdateLatencyExceededLimit } });

    for (size_t i = 0; i < 5u; ++i)
    {
        latencyMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(i), std::chrono::milliseconds(1));
        latencyMonitor.onRendered(scene1);
        latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(i + 2));
    }
    // is still exceeded
    latencyMonitor.onRendered(scene1);
    expectNoEvent();
}

TEST_F(ALatencyMonitor, generatesRecoveryEventWhenExceedingStops)
{
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(2));
    expectEvents({ { scene1, ERendererEventType_SceneUpdateLatencyExceededLimit } });

    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(10000));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(3));
    expectEvents({ { scene1, ERendererEventType_SceneUpdateLatencyBackBelowLimit } });
}

TEST_F(ALatencyMonitor, generatesRecoveryEventOnlyOnceWhenExceedingStopsAndKeepsBelowLimit)
{
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(2));
    expectEvents({ { scene1, ERendererEventType_SceneUpdateLatencyExceededLimit } });

    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(10000));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(3));
    expectEvents({ { scene1, ERendererEventType_SceneUpdateLatencyBackBelowLimit } });

    for (size_t i = 0; i < 5u; ++i)
    {
        latencyMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(i), std::chrono::milliseconds(10000));
        latencyMonitor.onRendered(scene1);
        latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(i + 10));
    }
    // is still below limit
    expectNoEvent();
}

TEST_F(ALatencyMonitor, willReportSceneThatKeepsBeingFlushedButNotRendered)
{
    RendererEventVector events;
    for (size_t i = 0; i < 5; ++i)
    {
        latencyMonitor.onFlushApplied(scene1, currentFakeTime + std::chrono::milliseconds(i), std::chrono::milliseconds(2));
        latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(i + 1));
    }
    expectEvents({ { scene1, ERendererEventType_SceneUpdateLatencyExceededLimit } });
}

TEST_F(ALatencyMonitor, willReportSceneThatKeepsBeingRenderedButNotFlushed)
{
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(2));

    RendererEventVector events;
    for (size_t i = 0; i < 5; ++i)
    {
        latencyMonitor.onRendered(scene1);
        latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(i));
    }
    expectEvents({ { scene1, ERendererEventType_SceneUpdateLatencyExceededLimit } });
}

TEST_F(ALatencyMonitor, confidenceTest_checkingOfOneSceneDoesNotAffectCheckingOfOtherScene)
{
    // flush 1
    latencyMonitor.onFlushApplied(scene1, LatencyMonitor::Clock::now() - std::chrono::milliseconds(10), std::chrono::milliseconds(1));

    // few cycles of scene 2
    latencyMonitor.onFlushApplied(scene2, LatencyMonitor::Clock::now() - std::chrono::milliseconds(10), std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene2);
    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    expectEvents({ { scene1, ERendererEventType_SceneUpdateLatencyExceededLimit },{ scene2, ERendererEventType_SceneUpdateLatencyExceededLimit } });
    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    latencyMonitor.onFlushApplied(scene2, LatencyMonitor::Clock::now() - std::chrono::milliseconds(10), std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene2);
    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    latencyMonitor.onFlushApplied(scene2, LatencyMonitor::Clock::now() - std::chrono::milliseconds(10), std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene2);
    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    expectNoEvent();

    // both exceeded, get both back to normal
    latencyMonitor.onFlushApplied(scene1, LatencyMonitor::Clock::now(), std::chrono::milliseconds(10000));
    latencyMonitor.onFlushApplied(scene2, LatencyMonitor::Clock::now(), std::chrono::milliseconds(10000));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.onRendered(scene2);
    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    expectEvents({ { scene1, ERendererEventType_SceneUpdateLatencyBackBelowLimit },{ scene2, ERendererEventType_SceneUpdateLatencyBackBelowLimit } });

    latencyMonitor.onFlushApplied(scene1, LatencyMonitor::Clock::now(), std::chrono::milliseconds(10000));
    latencyMonitor.onFlushApplied(scene2, LatencyMonitor::Clock::now(), std::chrono::milliseconds(10000));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.onRendered(scene2);
    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    // both still below limit
    expectNoEvent();
}

TEST_F(ALatencyMonitor, confidenceTest_severalScenesWithDifferentBehaviorAndLimits)
{
    //scene1 checking enabled, sometimes exceeds
    //scene2 checking disabled
    //scene3 checking enabled, never exceeds
    //scene4 checking enabled, always exceeds
    //scene5 checking enabled, sometimes exceeds

    // 'randomize' times of apply/render/checks

    latencyMonitor.onFlushApplied(scene1, LatencyMonitor::Clock::now() - std::chrono::milliseconds(10), std::chrono::milliseconds(10000));
    latencyMonitor.onFlushApplied(scene3, LatencyMonitor::Clock::now(), std::chrono::milliseconds(10000));
    latencyMonitor.onFlushApplied(scene5, LatencyMonitor::Clock::now() - std::chrono::milliseconds(10), std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene2);
    latencyMonitor.onRendered(scene5);

    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    expectEvents({ {scene5, ERendererEventType_SceneUpdateLatencyExceededLimit} });

    latencyMonitor.onFlushApplied(scene4, LatencyMonitor::Clock::now() - std::chrono::milliseconds(10), std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.onRendered(scene3);

    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    expectEvents({ { scene4, ERendererEventType_SceneUpdateLatencyExceededLimit } });

    latencyMonitor.onFlushApplied(scene5, LatencyMonitor::Clock::now(), std::chrono::milliseconds(10000));
    latencyMonitor.onFlushApplied(scene1, LatencyMonitor::Clock::now() - std::chrono::milliseconds(10), std::chrono::milliseconds(10000));
    latencyMonitor.onRendered(scene2);
    latencyMonitor.onRendered(scene4);

    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    expectEvents({ { scene5, ERendererEventType_SceneUpdateLatencyBackBelowLimit } });

    latencyMonitor.onFlushApplied(scene1, LatencyMonitor::Clock::now() - std::chrono::milliseconds(10), std::chrono::milliseconds(10000));
    latencyMonitor.onFlushApplied(scene3, LatencyMonitor::Clock::now(), std::chrono::milliseconds(10000));
    latencyMonitor.onFlushApplied(scene4, LatencyMonitor::Clock::now() - std::chrono::milliseconds(10), std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene1);
    latencyMonitor.onRendered(scene3);
    latencyMonitor.onRendered(scene5);

    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    expectNoEvent();

    latencyMonitor.onFlushApplied(scene1, LatencyMonitor::Clock::now() - std::chrono::milliseconds(10), std::chrono::milliseconds(1));
    latencyMonitor.onRendered(scene1);

    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    expectEvents({ { scene1, ERendererEventType_SceneUpdateLatencyExceededLimit } });

    latencyMonitor.checkLatency(LatencyMonitor::Clock::now());
    expectNoEvent();
}

TEST_F(ALatencyMonitor, stopsMonitoringScene)
{
    RendererEventVector events;
    latencyMonitor.onFlushApplied(scene1, currentFakeTime, std::chrono::milliseconds(1));
    latencyMonitor.stopMonitoringScene(scene1);

    // this would trigger exceeded event if still monitored
    latencyMonitor.checkLatency(currentFakeTime + std::chrono::milliseconds(10));
    expectNoEvent();
}
