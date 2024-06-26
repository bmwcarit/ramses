//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ITaskQueue.h"
#include "ITask.h"
#include "TaskExecutingThreadPool.h"
#include "internal/Core/TaskFramework/ProcessingTaskQueue.h"
#include "impl/ThreadWatchdogConfig.h"
#include "internal/Watchdog/ThreadWatchdog.h"
#include "internal/Watchdog/PlatformWatchdog.h"
#include "internal/PlatformAbstraction/Collections/Vector.h"

namespace ramses::internal
{
    /**
     * The (central) task executor class which puts execute request in the processing queue and uses the thread pool
     * for executing the tasks.
     */
    class ThreadedTaskExecutor final : public ITaskQueue, public ThreadWatchdog
    {
    public:
        /**
        * Create instance with the processing task queue and the thread pool.
        * @param   threadCount     The number of threads which should be created.
        * @param   watchdogConfig The configuration for watchdogHandling
        */
        explicit ThreadedTaskExecutor(uint16_t threadCount, const ThreadWatchdogConfig& watchdogConfig = ThreadWatchdogConfig());

        /**
         * Virtual destructor.
         */
        ~ThreadedTaskExecutor() override;

        /**
         * @name    TaskExecutor implementation
         * @see ITaskChainComponent
         * @{
         */
        bool enqueue(ITask& Task) override;

        /**
         * @}
         */

        /**
         * Deinit the instance with the processing task queue and the thread pool.
         */
        void deinit();

        /**
         * Stop the instance with the executing thread and the thread pool.
         */
        void stop();
        void disableAcceptingTasksAfterExecutingCurrentQueue() override;
    private:
        /**
         * Start the instance with the executing thread and the thread pool.
         */
        void start();

        /**
         * The processing task queue instance.
         */
        ProcessingTaskQueue m_taskQueue;
        /**
         * The thread pool instance.
         */
        TaskExecutingThreadPool m_threadPool;

        bool m_acceptingNewTasks;
        mutable std::mutex m_lock;
    };
}
