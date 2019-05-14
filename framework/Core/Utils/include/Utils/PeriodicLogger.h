//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PERIODICLOGGER_H
#define RAMSES_PERIODICLOGGER_H

#include "PlatformAbstraction/PlatformTypes.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformAbstraction/PlatformEvent.h"
#include "PlatformAbstraction/synchronized_clock.h"
#include "Collections/Vector.h"
#include "Utils/IPeriodicLogSupplier.h"
#include "Utils/StatisticCollection.h"
#include "Collections/HashMap.h"
#include "SceneAPI/SceneId.h"
#include <atomic>

namespace ramses_internal
{
    class PeriodicLogger : public Runnable
    {
    public:
        PeriodicLogger(PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection);
        ~PeriodicLogger();

        void startLogging(UInt32 periodicLogTimeoutSeconds);

        void registerPeriodicLogSupplier(IPeriodicLogSupplier* supplier);
        void removePeriodicLogSupplier(IPeriodicLogSupplier* supplier);

        void registerStatisticCollectionScene(const SceneId& sceneId, StatisticCollectionScene& statisticCollectionScene);
        void removeStatisticCollectionScene(const SceneId& sceneId);

    private:
        virtual void run() override final;
        void printVersion();
        void printStatistic();

        template<typename DataType>
        void logStatisticSummaryEntry(StringOutputStream& stream, const SummaryEntry<DataType>& summary, UInt32 numberTimeIntervals);

        bool           m_isRunning;
        PlatformEvent  m_event;
        UInt32         m_periodicLogTimeoutSeconds;
        PlatformThread m_thread;
        PlatformLock&  m_frameworkLock;
        std::vector<IPeriodicLogSupplier*> m_periodicLogSuppliers;
        StatisticCollectionFramework& m_statisticCollection;
        UInt32 m_triggerCounter;
        HashMap<SceneId, StatisticCollectionScene*> m_statisticCollectionScenes;

        std::chrono::steady_clock::time_point m_previousSteadyTime;
        synchronized_clock::time_point m_previousSyncTime;

        const UInt64 m_ramsesInstanceStartupTime;
        static const UInt64 m_processStartupTime;
        static std::atomic<UInt32> m_numberOfRamsesInstancesCurrentlyActive;
        static std::atomic<UInt32> m_numberOfRamsesInstancesStartedInProcess;
    };

    template<typename DataType>
    void PeriodicLogger::logStatisticSummaryEntry(StringOutputStream& stream, const SummaryEntry<DataType>& summary, UInt32 numberTimeIntervals)
    {
        if (numberTimeIntervals > 0)
        {
            stream << "(" << summary.minValue << "/" << summary.maxValue << "/" << summary.sum / numberTimeIntervals << ")";
        }
        else
        {
            stream << "(n.a.)";
        }
    }

}

#endif
