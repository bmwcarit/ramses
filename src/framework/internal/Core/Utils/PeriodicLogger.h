//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/PlatformAbstraction/PlatformEvent.h"
#include "internal/PlatformAbstraction/PlatformLock.h"
#include "internal/PlatformAbstraction/synchronized_clock.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"
#include "internal/Core/Utils/IPeriodicLogSupplier.h"
#include "internal/Core/Utils/StatisticCollection.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"
#include "internal/SceneGraph/SceneAPI/SceneId.h"

#include <cstdint>
#include <atomic>

namespace ramses::internal
{
    class PeriodicLogger : public Runnable
    {
    public:
        PeriodicLogger(PlatformLock& frameworkLock, StatisticCollectionFramework& statisticCollection);
        ~PeriodicLogger() override;

        void startLogging(uint32_t periodicLogTimeoutSeconds);

        void registerPeriodicLogSupplier(IPeriodicLogSupplier* supplier);
        void removePeriodicLogSupplier(IPeriodicLogSupplier* supplier);

        void registerStatisticCollectionScene(const SceneId& sceneId, StatisticCollectionScene& statisticCollectionScene);
        void removeStatisticCollectionScene(const SceneId& sceneId);

    private:
        void run() final override;
        void printVersion();
        void printStatistic();

        bool           m_isRunning;
        PlatformEvent  m_event;
        uint32_t         m_periodicLogTimeoutSeconds;
        PlatformThread m_thread;
        PlatformLock&  m_frameworkLock;
        std::vector<IPeriodicLogSupplier*> m_periodicLogSuppliers;
        StatisticCollectionFramework& m_statisticCollection;
        uint32_t m_triggerCounter;
        HashMap<SceneId, StatisticCollectionScene*> m_statisticCollectionScenes;

        std::chrono::steady_clock::time_point m_previousSteadyTime;
        synchronized_clock::time_point m_previousSyncTime;

        const uint64_t m_ramsesInstanceStartupTime;
        static const uint64_t m_processStartupTime;
        static std::atomic<uint32_t> m_numberOfRamsesInstancesCurrentlyActive;
        static std::atomic<uint32_t> m_numberOfRamsesInstancesStartedInProcess;
    };
}
