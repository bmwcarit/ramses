//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TaskFramework/ThreadedTaskExecutor.h"
#include "PlatformAbstraction/PlatformGuard.h"


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
    }

    ThreadedTaskExecutor::~ThreadedTaskExecutor()
    {
        disableAcceptingTasksAfterExecutingCurrentQueue();
        stop();
        deinit();
    }

    Bool ThreadedTaskExecutor::enqueue(ITask& task)
    {
        // Just add the supplied task to the queue, the execution threads of the pool are listening on the queue for
        // new work.
        PlatformLightweightGuard guard(m_lock);
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
        PlatformLightweightGuard guard(m_lock);
        m_acceptingNewTasks = true;
        m_threadPool.start(m_taskQueue);
    }

    void ThreadedTaskExecutor::stop()
    {
        PlatformLightweightGuard guard(m_lock);
        m_threadPool.stop();
        while (!m_taskQueue.isEmpty())
        {
            m_taskQueue.popTask();
        }
    }

    void ThreadedTaskExecutor::disableAcceptingTasksAfterExecutingCurrentQueue()
    {
        PlatformLightweightGuard guard(m_lock);
        m_acceptingNewTasks = false;
        while (!m_taskQueue.isEmpty())
        {
            m_condVarQueueEmpty.wait(&m_lock, 5);
        }
    }

    void ThreadedTaskExecutor::enableAcceptingTasks()
    {
        PlatformLightweightGuard guard(m_lock);
        m_acceptingNewTasks = true;
    }

    void ThreadedTaskExecutor::notifyAlive(UInt16 threadIndex)
    {
        PlatformLightweightGuard g(m_aliveLock);
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
        PlatformLightweightGuard g(m_aliveLock);
        return m_watchdogNotifier.calculateTimeout();
    }
}
