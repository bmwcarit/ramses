//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TaskFramework/ThreadedTaskExecutor.h"
#include <thread>

namespace ramses_internal
{
    ThreadedTaskExecutor::ThreadedTaskExecutor(UInt16 threadCount, const ThreadWatchdogConfig& watchdogConfig)
        : ThreadWatchdog(watchdogConfig, ramses::ERamsesThreadIdentifier::Workers)
        , m_taskQueue()
        , m_threadPool()
        , m_acceptingNewTasks(true)
    {
        m_threadPool.init(threadCount, *this);
        start();
    }

    ThreadedTaskExecutor::~ThreadedTaskExecutor()
    {
        disableAcceptingTasksAfterExecutingCurrentQueue();
        stop();
        deinit();
    }

    bool ThreadedTaskExecutor::enqueue(ITask& task)
    {
        // Just add the supplied task to the queue, the execution threads of the pool are listening on the queue for
        // new work.
        std::lock_guard<std::mutex> guard(m_lock);
        if (m_acceptingNewTasks)
        {
            m_taskQueue.addTask(&task);
        }
        return true;
    }

    void ThreadedTaskExecutor::deinit()
    {
        m_threadPool.deinit();
    }

    void ThreadedTaskExecutor::start()
    {
        std::lock_guard<std::mutex> guard(m_lock);
        m_acceptingNewTasks = true;
        m_threadPool.start(m_taskQueue);
    }

    void ThreadedTaskExecutor::stop()
    {
        std::lock_guard<std::mutex> guard(m_lock);
        m_threadPool.stop();
        while (!m_taskQueue.isEmpty())
        {
            m_taskQueue.popTask();
        }
    }

    void ThreadedTaskExecutor::disableAcceptingTasksAfterExecutingCurrentQueue()
    {
        std::unique_lock<std::mutex> l(m_lock);
        m_acceptingNewTasks = false;
        // TODO(tobias) there is no trigger to wait for, just keep polling
        while (!m_taskQueue.isEmpty())
        {
            l.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds{5});
            l.lock();
        }
    }
}
