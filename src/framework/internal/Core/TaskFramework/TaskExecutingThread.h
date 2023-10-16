//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/PlatformAbstraction/PlatformThread.h"
#include "internal/Watchdog/IThreadAliveNotifier.h"
#include <mutex>

namespace ramses::internal
{
    // class forward declarations
    class ITask;
    class ITaskExecutionObserver;
    class ProcessingTaskQueue;

    /**
     * This class is an active thread and can execute ITask instances in this active thread context.
     */
    class TaskExecutingThread : public Runnable
    {

    public:
        explicit TaskExecutingThread(IThreadAliveNotifier& aliveHandler);
        ~TaskExecutingThread() override;


        /**
         * Start the thread and use the provided processing task queue to pop the next task for execution.
         * @param   blockingTaskQueue   Reference to the blocking task queue.
         */
        void start(ProcessingTaskQueue& blockingTaskQueue);

        /**
         * Stop and join the thread when tasks are finished.
         */
        void stop();

        /**
         * Signals thread to stop after tasks are finished - non blocking function
         * \post joinThread can and should be called
         */
        void cancelThread();

        /**
        * Pushes an empty task to the queue of this thread to stop waiting for tasks
        * Use this in between cancelThread and joinThread to speed up joining
        */
        void unlockThread();

        /**
         * Waits for thread to stop, joins and cleans thread up - blocking function
         * \pre cancelThread was called
         */
        void joinThread();

    protected:
        /**
         * @see Runnable
         * @{
         */
        void run() override;
        /**
         * @}
         */


    private:
        /**
         * Pointer to the blocking task queue.
         */
        ProcessingTaskQueue* m_pBlockingTaskQueue;

        /**
         * The thread which calls our run function.
         */
        PlatformThread m_thread;

        std::mutex m_startStopLock;

        IThreadAliveNotifier& m_aliveHandler;
        const uint64_t m_aliveIdentifier;

        /**
         * Flag whether the thread is started.
         */
        bool m_bThreadStarted;
    };


}
