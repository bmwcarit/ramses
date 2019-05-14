//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TASKEXECUTINGTHREADPOOL_H
#define RAMSES_TASKEXECUTINGTHREADPOOL_H

#include "PlatformAbstraction/PlatformTypes.h"

#include "Collections/Vector.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "IBlockingTaskQueue.h"
#include "TaskExecutingThread.h"
#include <memory>

namespace ramses_internal
{
    // class forward declarations
    class IBlockingTaskQueue;
    class TaskExecutingThread;

    /**
     * Pool for task executing threads.
     */
    class TaskExecutingThreadPool
    {
    public:
        /**
         * Default constructor.
         */
        TaskExecutingThreadPool();
        /**
         * Destructor.
         */
        ~TaskExecutingThreadPool();

        /**
         * Initialize the thread pool with the supplied amount of threads.
         * @param   threadCount   The number of threads which should be created.
         * @param   aliveHandler  Thread alive handler
         */
        void init(UInt16 threadCount, IThreadAliveNotifier& aliveHandler);

        /**
         * Deinitialize the thread pool.
         */
        void deinit();

        /**
         * Start all threads of the thread pool in order that they are ready for executing and provide the supplied
         * ProcessingTaskQueue to each thread so it can block on it.
         * @param   blockingTaskQueue   Reference to the blocking task queue.
         */
        void start(IBlockingTaskQueue& blockingTaskQueue);
        /**
         * Stop all threads of the thread pool.
         */
        void stop();

    private:
        /**
         * The instance of the container class which contains the threads of the thread pool which are ready.
         */
        std::vector<std::unique_ptr<TaskExecutingThread>> m_threads;

        /**
         * Mutex for protection the thread containers.
         */
        PlatformLightweightLock m_mutex;

        /**
         * Flag whether the pool is initialized.
         */
        Bool m_bInitialized;
        /**
         * Flag whether the threads within the pool are started.
         */
        Bool m_bStarted;

        /**
         * Anonymous enumeration for defining constants.
         */
        enum
        {
            /**
             * The maximum number of allowed threads within this pool.
             */
            MAXIMUM_THREAD_COUNT        = 256
        };
    };
}

#endif
