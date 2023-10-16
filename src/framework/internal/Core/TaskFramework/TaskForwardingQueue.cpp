//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/TaskFramework/TaskForwardingQueue.h"
#include "internal/Core/TaskFramework/TaskFinishHandlerDecorator.h"

namespace ramses::internal
{
    TaskForwardingQueue::TaskForwardingQueue(ITaskQueue& nextQueue)
    : m_nextQueue(nextQueue)
    , m_acceptNewTasks(true)
    {
    }

    TaskForwardingQueue::~TaskForwardingQueue() = default;

    bool TaskForwardingQueue::enqueue(ITask& task)
    {
        std::lock_guard<std::recursive_mutex> g(m_lock);
        if (m_acceptNewTasks)
        {
            auto decorator = new TaskFinishHandlerDecorator(*this, task);
            m_ongoingTasks.put(&task);
            m_nextQueue.enqueue(*decorator);
            decorator->release();
        }
        return m_acceptNewTasks;
    }

    void TaskForwardingQueue::disableAcceptingTasksAfterExecutingCurrentQueue()
    {
        std::lock_guard<std::recursive_mutex> l(m_lock);
        m_acceptNewTasks = false;
        m_cond.wait(m_lock, [&]() { return m_ongoingTasks.size() == 0; });
    }

    void TaskForwardingQueue::TaskFinished(ITask& task)
    {
        std::lock_guard<std::recursive_mutex> g(m_lock);
        if (m_ongoingTasks.remove(&task))
        {
            m_cond.notify_all();
        }
    }
}
