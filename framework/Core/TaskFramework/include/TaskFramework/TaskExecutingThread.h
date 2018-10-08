//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_TASKEXECUTINGTHREAD_H
#define RAMSES_TASKEXECUTINGTHREAD_H

#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "IBlockingTaskQueue.h"


namespace ramses_internal
{


    // class forward declarations
    class ITask;
    class ITaskExecutionObserver;
    class IBlockingTaskQueue;

    class IThreadAliveNotifier
    {
    public:
        virtual ~IThreadAliveNotifier()
        {
        }
        virtual void notifyAlive(UInt16 threadIndex) = 0;
        virtual UInt32 calculateTimeout() const = 0;
    };

    /**
     * This class is an active thread and can execute ITask instances in this active thread context.
     */
    class TaskExecutingThread : public Runnable
    {

    public:
        TaskExecutingThread(UInt16 workerIndex, IThreadAliveNotifier& aliveHandler);
        ~TaskExecutingThread();


        /**
         * Start the thread and use the provided processing task queue to pop the next task for execution.
         * @param   blockingTaskQueue   Reference to the blocking task queue.
         */
        void start(IBlockingTaskQueue& blockingTaskQueue);

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
        virtual void run() override;
        /**
         * @}
         */


    private:
        /**
         * Pointer to the blocking task queue.
         */
        IBlockingTaskQueue* m_pBlockingTaskQueue;

        /**
         * The thread which calls our run function.
         */
        PlatformThread m_thread;

        PlatformLightweightLock m_startStopLock;

        UInt16 m_workerIndex;

        IThreadAliveNotifier& m_aliveHandler;
        /**
         * Flag whether the thread is started.
         */
        Bool m_bThreadStarted;
    };


}

#endif
