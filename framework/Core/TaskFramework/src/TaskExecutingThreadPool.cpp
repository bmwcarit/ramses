//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TaskFramework/TaskExecutingThreadPool.h"
#include "TaskFramework/TaskExecutingThread.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    TaskExecutingThreadPool::TaskExecutingThreadPool()
            : m_mutex()
            , m_bInitialized(false)
            , m_bStarted(false)
    {
    }

    TaskExecutingThreadPool::~TaskExecutingThreadPool()
    {
        stop();
        deinit();
    }

    void TaskExecutingThreadPool::init(UInt16 threadCount, IThreadAliveNotifier& threadAliveHandler)
    {
        std::lock_guard<std::mutex> mutexGuard(m_mutex);

        if (!m_bInitialized)
        {
            m_bInitialized = true;

            // Limit the maximum number of threads which could be created.
            if (threadCount > MAXIMUM_THREAD_COUNT)
            {
                threadCount = MAXIMUM_THREAD_COUNT;
            }

            for (UInt16 index = 0; index < threadCount; ++index)
            {
                m_threads.push_back(std::make_unique<TaskExecutingThread>(threadAliveHandler));
            }
        }
    }

    void TaskExecutingThreadPool::deinit()
    {
        std::lock_guard<std::mutex> mutexGuard(m_mutex);

        if (m_bInitialized)
        {
            m_bInitialized = false;
            m_threads.clear();
        }
    }


    void TaskExecutingThreadPool::start(ProcessingTaskQueue& blockingTaskQueue)
    {
        std::lock_guard<std::mutex> mutexGuard(m_mutex);

        if (m_bInitialized)
        {
            if (!m_bStarted)
            {
                for (auto& pThread : m_threads)
                {
                    pThread->start(blockingTaskQueue);
                }
                m_bStarted = true;
            }
        }
        else
        {
            LOG_ERROR(CONTEXT_FRAMEWORK, "Cannot start threadpool without being initialized.");
        }
    }


    void TaskExecutingThreadPool::stop()
    {
        std::lock_guard<std::mutex> mutexGuard(m_mutex);

        if (m_bStarted)
        {
            m_bStarted = false;
            for (auto& pThread : m_threads)
            {
                pThread->cancelThread();
            }
            for (auto& pThread : m_threads)
            {
                pThread->unlockThread();
            }
            for (auto& pThread : m_threads)
            {
                pThread->joinThread();
            }
        }
    }
}
