//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PROCESSINGTASKQUEUE_H
#define RAMSES_PROCESSINGTASKQUEUE_H

#include "Collections/BlockingQueue.h"
#include "TaskFramework/IBlockingTaskQueue.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "ITask.h"


namespace ramses_internal
{
    /**
     * The processing task queue stores task which should be executed according with their execution observer.
     */
class ProcessingTaskQueue : public IBlockingTaskQueue
{
    public:
        /**
         * Default constructor.
         */
        ProcessingTaskQueue();
        /**
         * Virtual destructor.
         */
        virtual ~ProcessingTaskQueue();

        /**
         * @name    IBlockingTaskQueue implementation.
         * @see IBlockingTaskQueue
         * @{
         */
        virtual void addTask(ITask* taskToAdd) override;

        virtual ITask* popTask(UInt32 timeoutMillis = 0) override;

        virtual Bool isEmpty() /*const*/ override;
        /**
         * @}
         */

    private:
        /**
         * Definition for the queue type.
         */
        typedef BlockingQueue<ITask*> TaskQueue;
        /**
         * The instance of the blocking queue for storing the tasks to execute.
         */
        TaskQueue m_taskQueue;
    };

    inline ProcessingTaskQueue::ProcessingTaskQueue()
    {
    }

    inline ProcessingTaskQueue::~ProcessingTaskQueue()
    {
    }

    inline void ProcessingTaskQueue::addTask(ITask* taskToAdd)
    {
        if (taskToAdd)
        {
            taskToAdd->addRef();
        }
        m_taskQueue.push(taskToAdd);
    }

    inline ITask* ProcessingTaskQueue::popTask(UInt32 timeoutMillis)
    {
        ITask* task = 0;
        EStatus status = m_taskQueue.pop(&task, timeoutMillis);
        switch (status)
        {
            case EStatus_RAMSES_OK:
                return task;
            case EStatus_RAMSES_TIMEOUT:
                return 0;
            default:
            {
                //some other error occurred, which may prevented the task queue to wait for the given amount of milliseconds.
                //In order to avoid having a busy looping, we sleep here again
                PlatformThread::Sleep(timeoutMillis);
                LOG_FATAL(CONTEXT_FRAMEWORK, "Error " << status << "while requesting task from TaskQueue. This should not happen.");
            }
        }

        return 0;
    }

    inline Bool ProcessingTaskQueue::isEmpty() /*const*/
    {
        return (m_taskQueue.empty());
    }
}

#endif
