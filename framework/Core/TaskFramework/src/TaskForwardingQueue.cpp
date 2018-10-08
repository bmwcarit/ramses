//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TaskFramework/TaskForwardingQueue.h"
#include "TaskFramework/TaskFinishHandlerDecorator.h"
#include "PlatformAbstraction/PlatformGuard.h"

namespace ramses_internal
{
    TaskForwardingQueue::TaskForwardingQueue(ITaskQueue& nextQueue)
    : m_nextQueue(nextQueue)
    , m_acceptNewTasks(true)
    , m_ongoingTasks()
    {
    }

    TaskForwardingQueue::~TaskForwardingQueue()
    {
    }

    Bool TaskForwardingQueue::enqueue(ITask& task)
    {
        PlatformGuard guard(m_lock);
        if (m_acceptNewTasks)
        {
            auto decorator = new TaskFinishHandlerDecorator(*this, task);
            m_ongoingTasks.put(&task);
            m_nextQueue.enqueue(*decorator);
            decorator->release();
        }
        return m_acceptNewTasks;
    }

    void TaskForwardingQueue::enableAcceptingTasks()
    {
        PlatformGuard guard(m_lock);
        m_acceptNewTasks = true;
    }

    void TaskForwardingQueue::disableAcceptingTasksAfterExecutingCurrentQueue()
    {
        PlatformGuard guard(m_lock);
        m_acceptNewTasks = false;
        while (m_ongoingTasks.count() != 0)
        {
            m_cond.wait(&m_lock);
        }
    }

    void TaskForwardingQueue::TaskFinished(ITask& task)
    {
        PlatformGuard guard(m_lock);
        if (m_ongoingTasks.remove(&task) == EStatus_RAMSES_OK)
        {
            m_cond.broadcast();
        }
    }
}
