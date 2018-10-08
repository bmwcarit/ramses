//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TaskFramework/EnqueueOnlyOneAtATimeQueue.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "TaskFramework/TaskFinishHandlerDecorator.h"
#include "PlatformAbstraction/PlatformThread.h"

namespace ramses_internal
{
    EnqueueOnlyOneAtATimeQueue::EnqueueOnlyOneAtATimeQueue(ITaskQueue& nextQueue)
        : m_nextQueue(nextQueue)
        , m_hasOutstandingTask(false)
        , m_acceptingTasks(true)
    {

    }

    EnqueueOnlyOneAtATimeQueue::~EnqueueOnlyOneAtATimeQueue()
    {
        PlatformGuard guard(mlock);
        for (auto task : m_waitingTasks)
        {
            task->release();
        }
    }

    Bool EnqueueOnlyOneAtATimeQueue::enqueue(ITask& Task)
    {
        PlatformGuard guard(mlock);
        if (m_acceptingTasks)
        {
            TaskFinishHandlerDecorator* decorator = new TaskFinishHandlerDecorator(*this, Task);
            if (!m_hasOutstandingTask)
            {
                moveTaskToNextQueue(*decorator);
                return true;
            }
            else
            {
                m_waitingTasks.push_back(decorator);
                return true;
            }
        }
        return false;
    }
    void EnqueueOnlyOneAtATimeQueue::moveTaskToNextQueue(ITask& task)
    {
        m_hasOutstandingTask = true;
        m_nextQueue.enqueue(task);
        task.release();
    }

    void EnqueueOnlyOneAtATimeQueue::TaskFinished(ITask& )
    {
        PlatformGuard guard(mlock);
        m_hasOutstandingTask = false;
        if (!m_waitingTasks.empty())
        {
            ITask* task = m_waitingTasks.front();
            m_waitingTasks.pop_front();
            moveTaskToNextQueue(*task);
        }
    }

    void EnqueueOnlyOneAtATimeQueue::enableAcceptingTasks()
    {
        PlatformGuard guard(mlock);
        m_acceptingTasks = true;
    }

    void EnqueueOnlyOneAtATimeQueue::disableAcceptingTasksAfterExecutingCurrentQueue()
    {
        mlock.lock();
        m_acceptingTasks = false;
        while (!m_waitingTasks.empty() || m_hasOutstandingTask)
        {
            mlock.unlock();
            PlatformThread::Sleep(5);
            mlock.lock();
        }
        mlock.unlock();
    }
}
