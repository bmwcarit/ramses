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
        : m_taskQueue()
        , m_threadPool()
        , m_acceptingNewTasks(true)
        , m_numberOfThreads(threadCount)
        , m_aliveThreads(threadCount, false)
        , m_watchdogNotifier(watchdogConfig.getWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier_Workers), ramses::ERamsesThreadIdentifier_Workers, watchdogConfig.getCallBack())
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

    void ThreadedTaskExecutor::notifyAlive(UInt16 threadIndex)
    {
        std::lock_guard<std::mutex> g(m_aliveLock);
        m_aliveThreads[threadIndex] = true;

        for (uint32_t i = 0; i < m_numberOfThreads; ++i)
        {
            if (!m_aliveThreads[i])
            {
                return;
            }
        }

        m_watchdogNotifier.notifyWatchdog();
        for (uint32_t i = 0; i < m_numberOfThreads; ++i)
        {
            m_aliveThreads[i] = false;
        }
    }

    UInt32 ThreadedTaskExecutor::calculateTimeout() const
    {
        std::lock_guard<std::mutex> g(m_aliveLock);
        return m_watchdogNotifier.calculateTimeout();
    }
}
