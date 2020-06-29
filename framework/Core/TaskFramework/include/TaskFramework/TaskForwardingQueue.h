//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------


#ifndef RAMSES_TASKFORWARDINGQUEUE_H
#define RAMSES_TASKFORWARDINGQUEUE_H

#include "ITaskQueue.h"
#include "ITaskFinishHandler.h"
#include "Collections/HashSet.h"
#include <mutex>
#include <condition_variable>

namespace ramses_internal
{
    class TaskForwardingQueue : public ITaskQueue, public ITaskFinishHandler
    {
    public:
        explicit TaskForwardingQueue(ITaskQueue& nextQueue);
        explicit TaskForwardingQueue(const TaskForwardingQueue&) = delete;
        virtual ~TaskForwardingQueue();

        virtual bool enqueue(ITask& task) override;
        virtual void enableAcceptingTasks() override;
        virtual void disableAcceptingTasksAfterExecutingCurrentQueue() override;

        virtual void TaskFinished(ITask& Task) override;

    protected:
        // recursive mutex needed for direct task executor: calls TaskFinished from inside enqueue
        std::recursive_mutex        m_lock;
        std::condition_variable_any m_cond;

        ITaskQueue&                 m_nextQueue;
        bool                        m_acceptNewTasks;
        HashSet<ITask*>             m_ongoingTasks;
    };
}

#endif
