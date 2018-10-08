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
#include "PlatformAbstraction/PlatformLock.h"
#include "PlatformAbstraction/PlatformConditionVariable.h"

namespace ramses_internal
{
    class TaskForwardingQueue : public ITaskQueue, public ITaskFinishHandler
    {
    public:
        explicit TaskForwardingQueue(ITaskQueue& nextQueue);
        explicit TaskForwardingQueue(const TaskForwardingQueue&) = delete;
        virtual ~TaskForwardingQueue();

        virtual Bool enqueue(ITask& task) override;
        virtual void enableAcceptingTasks() override;
        virtual void disableAcceptingTasksAfterExecutingCurrentQueue() override;

        virtual void TaskFinished(ITask& Task) override;

    protected:
        PlatformLock                m_lock;
        PlatformConditionVariable   m_cond;

        ITaskQueue&                 m_nextQueue;
        Bool                        m_acceptNewTasks;
        HashSet<ITask*>             m_ongoingTasks;
    };
}

#endif
