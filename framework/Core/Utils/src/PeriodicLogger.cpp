//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/PeriodicLogger.h"
#include "Utils/LogMacros.h"
#include "ramses-sdk-build-config.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "PlatformAbstraction/PlatformTime.h"

namespace ramses_internal
{
    const UInt64 PeriodicLogger::m_processStartupTime = PlatformTime::GetMillisecondsMonotonic();
    std::atomic<UInt32> PeriodicLogger::m_numberOfRamsesInstancesStartedInProcess(0);
    std::atomic<UInt32> PeriodicLogger::m_numberOfRamsesInstancesCurrentlyActive(0);

    PeriodicLogger::PeriodicLogger(PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection)
        : m_isRunning(false)
        , m_periodicLogTimeoutSeconds(0)
        , m_thread("R_PerLogger")
        , m_frameworkLock(frameworkLock)
        , m_statisticCollection(statisticCollection)
        , m_triggerCounter(0)
        , m_previousSteadyTime(std::chrono::steady_clock::now())
        , m_previousSyncTime(synchronized_clock::now())
        , m_ramsesInstanceStartupTime(PlatformTime::GetMillisecondsMonotonic())
    {
        ++m_numberOfRamsesInstancesStartedInProcess;
        ++m_numberOfRamsesInstancesCurrentlyActive;
    }

    PeriodicLogger::~PeriodicLogger()
    {
        m_thread.cancel();
        m_event.signal();
        if (m_isRunning)
        {
            m_thread.join();
        }
        --m_numberOfRamsesInstancesCurrentlyActive;
    }

    void PeriodicLogger::startLogging(UInt32 periodicLogTimeoutSeconds)
    {
        assert(!m_isRunning);
        assert(periodicLogTimeoutSeconds > 0);
        m_periodicLogTimeoutSeconds = periodicLogTimeoutSeconds;
        m_isRunning = true;
        m_statisticCollection.reset();
        m_thread.start(*this);
    }

    void PeriodicLogger::run()
    {
        while (!isCancelRequested())
        {
            m_event.wait(1000);
            m_triggerCounter++;

            {
                PlatformGuard guard(m_frameworkLock);  //guards m_statisticCollectionScenes and m_periodicLogSuppliers

                m_statisticCollection.nextTimeInterval();

                for (auto entry : m_statisticCollectionScenes)
                {
                    entry.value->nextTimeInterval();
                }

                if (m_triggerCounter == m_periodicLogTimeoutSeconds)
                {
                    m_triggerCounter = 0;
                    printVersion();

                    for (auto supplier : m_periodicLogSuppliers)
                    {
                        supplier->triggerLogMessageForPeriodicLog();
                    }

                    printStatistic();
                }
            }
        }
    }

    void PeriodicLogger::registerPeriodicLogSupplier(IPeriodicLogSupplier* supplier)
    {
        PlatformGuard guard(m_frameworkLock);
        m_periodicLogSuppliers.push_back(supplier);
    }

    void PeriodicLogger::removePeriodicLogSupplier(IPeriodicLogSupplier* supplier)
    {
        PlatformGuard guard(m_frameworkLock);

        auto it = find_c(m_periodicLogSuppliers, supplier);
        if (it != m_periodicLogSuppliers.end())
        {
            m_periodicLogSuppliers.erase(it);
        }
    }

    void PeriodicLogger::registerStatisticCollectionScene(const SceneId& sceneId, StatisticCollectionScene& statisticCollectionScene)
    {
        PlatformGuard guard(m_frameworkLock);
        m_statisticCollectionScenes[sceneId] = &statisticCollectionScene;
    }

    void PeriodicLogger::removeStatisticCollectionScene(const SceneId& sceneId)
    {
        PlatformGuard guard(m_frameworkLock);
        m_statisticCollectionScenes.remove(sceneId);
    }

    void PeriodicLogger::printVersion()
    {
        auto steadyNow = std::chrono::steady_clock::now();
        auto syncNow = synchronized_clock::now();

        uint64_t pUp = asMilliseconds(steadyNow) - m_processStartupTime;
        uint64_t rUp = asMilliseconds(steadyNow) - m_ramsesInstanceStartupTime;

        int64_t steadyDiff = std::chrono::duration_cast<std::chrono::milliseconds>(steadyNow - m_previousSteadyTime).count();
        int64_t syncDiff = std::chrono::duration_cast<std::chrono::milliseconds>(syncNow - m_previousSyncTime).count();

        LOG_INFO(CONTEXT_PERIODIC, "Version: " << ::ramses_sdk::RAMSES_SDK_PROJECT_VERSION_STRING << " Hash:" << ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_HASH
            << " Commit:" << ::ramses_sdk::RAMSES_SDK_GIT_COMMIT_COUNT << " Type:" << ::ramses_sdk::RAMSES_SDK_CMAKE_BUILD_TYPE
            << " Env:" << ::ramses_sdk::RAMSES_SDK_BUILD_ENV_VERSION_INFO_FULL_ESCAPED
            << " SyncT:" << asMilliseconds(syncNow) << "ms (dtSteady:" << steadyDiff << " - dtSync:" << syncDiff << " -> " << (steadyDiff - syncDiff) << ")"
            << " PUp:" << pUp <<  " RUp:" << rUp
            << " RInit:" << m_numberOfRamsesInstancesStartedInProcess << " RParallel:" << m_numberOfRamsesInstancesCurrentlyActive);

        m_previousSyncTime = syncNow;
        m_previousSteadyTime = steadyNow;
    }

    void PeriodicLogger::printStatistic()
    {
        LOG_INFO_F(CONTEXT_PERIODIC, ([&](ramses_internal::StringOutputStream& output) {
                    UInt32 numberTimeIntervals = m_statisticCollection.getNumberTimeIntervalsSinceLastSummaryReset();
                    output << "msgIn ";
                    logStatisticSummaryEntry(output, m_statisticCollection.statMessagesReceived.getSummary(), numberTimeIntervals);
                    output << " msgO ";
                    logStatisticSummaryEntry(output, m_statisticCollection.statMessagesSent.getSummary(), numberTimeIntervals);
                    output << " res+ ";
                    logStatisticSummaryEntry(output, m_statisticCollection.statResourcesCreated.getSummary(), numberTimeIntervals);
                    output << " res- ";
                    logStatisticSummaryEntry(output, m_statisticCollection.statResourcesDestroyed.getSummary(), numberTimeIntervals);
                    output << " resNr ";
                    logStatisticSummaryEntry(output, m_statisticCollection.statResourcesNumber.getSummary(), numberTimeIntervals);
                    output << " resOS ";
                    logStatisticSummaryEntry(output, m_statisticCollection.statResourcesSentSize.getSummary(), numberTimeIntervals);
                    output << " resF ";
                    logStatisticSummaryEntry(output, m_statisticCollection.statResourcesLoadedFromFileNumber.getSummary(), numberTimeIntervals);
                    output << " resFS ";
                    logStatisticSummaryEntry(output, m_statisticCollection.statResourcesLoadedFromFileSize.getSummary(), numberTimeIntervals);
        }));

        m_statisticCollection.resetSummaries();

        if (m_statisticCollectionScenes.count() > 0)
        {
            LOG_INFO_F(CONTEXT_PERIODIC, ([&](ramses_internal::StringOutputStream& output) {
                        for (auto entry : m_statisticCollectionScenes)
                        {
                            UInt32 numberTimeIntervals = entry.value->getNumberTimeIntervalsSinceLastSummaryReset();
                            output << "scene: " << entry.key;
                            output << " flush ";
                            logStatisticSummaryEntry(output, entry.value->statFlushesTriggered.getSummary(), numberTimeIntervals);
                            output << " obj+ ";
                            logStatisticSummaryEntry(output, entry.value->statObjectsCreated.getSummary(), numberTimeIntervals);
                            output << " obj- ";
                            logStatisticSummaryEntry(output, entry.value->statObjectsDestroyed.getSummary(), numberTimeIntervals);
                            output << " objNr ";
                            logStatisticSummaryEntry(output, entry.value->statObjectsNumber.getSummary(), numberTimeIntervals);
                            output << " actG ";
                            logStatisticSummaryEntry(output, entry.value->statSceneActionsGenerated.getSummary(), numberTimeIntervals);
                            output << " actGS ";
                            logStatisticSummaryEntry(output, entry.value->statSceneActionsGeneratedSize.getSummary(), numberTimeIntervals);
                            output << " actO ";
                            logStatisticSummaryEntry(output, entry.value->statSceneActionsSent.getSummary(), numberTimeIntervals);
                            output << " ";

                            entry.value->resetSummaries();
                        }
                    }));
        }
    }

}
