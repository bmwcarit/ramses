//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_THREADEDTASKEXECUTOR_H
#define RAMSES_THREADEDTASKEXECUTOR_H

#include "ITaskQueue.h"
#include "ITask.h"
#include "TaskExecutingThreadPool.h"
#include "TaskFramework/ProcessingTaskQueue.h"
#include "PlatformAbstraction/PlatformConditionVariable.h"

#include "ThreadWatchdogConfig.h"
#include "Watchdog/PlatformWatchdog.h"
#include "Collections/Vector.h"

namespace ramses_internal
{
    /**
     * The (central) task executor class which puts execute request in the processing queue and uses the thread pool
     * for executing the tasks.
     */
    class ThreadedTaskExecutor final : public ITaskQueue, public IThreadAliveNotifier
    {
    public:
        /**
        * Create instance with the processing task queue and the thread pool.
        * @param   threadCount     The number of threads which should be created.
        * @param   watchdogConfig The configuration for watchdogHandling
        */
        ThreadedTaskExecutor(UInt16 threadCount, const ThreadWatchdogConfig& watchdogConfig = ThreadWatchdogConfig());

        /**
         * Virtual destructor.
         */
        virtual ~ThreadedTaskExecutor() override;

        /**
         * @name    TaskExecutor implementation
         * @see ITaskChainComponent
         * @{
         */
        virtual Bool enqueue(ITask& Task) override;

        /**
         * @}
         */

        /**
         * Deinit the instance with the processing task queue and the thread pool.
         */
        void deinit();

        /**
         * Start the instance with the executing thread and the thread pool.
         */
        void start();
        /**
         * Stop the instance with the executing thread and the thread pool.
         */
        void stop();
        virtual void disableAcceptingTasksAfterExecutingCurrentQueue() override;
        virtual void enableAcceptingTasks() override;

        virtual void notifyAlive(UInt16 threadIndex) override;

        virtual UInt32 calculateTimeout() const override;

    private:
        /**
         * The processing task queue instance.
         */
        ProcessingTaskQueue m_taskQueue;
        /**
         * The thread pool instance.
         */
        TaskExecutingThreadPool m_threadPool;

        Bool m_acceptingNewTasks;
        mutable PlatformLightweightLock m_lock;

        PlatformConditionVariable m_condVarQueueEmpty;

        UInt32       m_numberOfThreads;
        mutable PlatformLightweightLock m_aliveLock;
        std::vector<Bool> m_aliveThreads;
        PlatformWatchdog m_watchdogNotifier;
    };
}

#endif
