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
#include "ITask.h"


namespace ramses_internal
{
    /**
     * The processing task queue stores task which should be executed according with their execution observer.
     */
    class ProcessingTaskQueue
    {
    public:
        void addTask(ITask* taskToAdd);
        ITask* popTask(std::chrono::milliseconds timeout = std::chrono::milliseconds{0});
        [[nodiscard]] bool isEmpty() const;

    private:
        BlockingQueue<ITask*> m_taskQueue;
    };

    inline void ProcessingTaskQueue::addTask(ITask* taskToAdd)
    {
        if (taskToAdd)
        {
            taskToAdd->addRef();
        }
        m_taskQueue.push(taskToAdd);
    }

    inline ITask* ProcessingTaskQueue::popTask(std::chrono::milliseconds timeout)
    {
        ITask* task = nullptr;
        m_taskQueue.pop(&task, timeout);
        return task;
    }

    inline bool ProcessingTaskQueue::isEmpty() const
    {
        return m_taskQueue.empty();
    }
}

#endif
