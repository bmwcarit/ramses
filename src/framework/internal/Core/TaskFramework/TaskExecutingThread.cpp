//  -------------------------------------------------------------------------
//  Copyright (C) 2013 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/TaskFramework/TaskExecutingThread.h"
#include "internal/Core/TaskFramework/ITask.h"
#include "internal/Core/TaskFramework/ProcessingTaskQueue.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    TaskExecutingThread::TaskExecutingThread(IThreadAliveNotifier& aliveHandler)
            : m_pBlockingTaskQueue(nullptr)
            , m_thread("Taskpool_Thrd")
            , m_aliveHandler(aliveHandler)
            , m_aliveIdentifier(m_aliveHandler.registerThread())
            , m_bThreadStarted(false)
    {
    }

    TaskExecutingThread::~TaskExecutingThread()
    {
        stop();
        m_aliveHandler.unregisterThread(m_aliveIdentifier);
    }


    void TaskExecutingThread::start(ProcessingTaskQueue& blockingTaskQueue)
    {
        std::lock_guard<std::mutex> g(m_startStopLock);
        if (!m_bThreadStarted)
        {
            m_bThreadStarted = true;

            m_pBlockingTaskQueue = &blockingTaskQueue;

            resetCancel();

            m_thread.start(*this);
        }
    }

    void TaskExecutingThread::stop()
    {
        std::lock_guard<std::mutex> g(m_startStopLock);
        if (m_bThreadStarted)
        {
            // Signal the runnable the cancel request.
            m_thread.cancel();
            m_pBlockingTaskQueue->addTask(nullptr);

            m_thread.join();
            m_pBlockingTaskQueue = nullptr;
            m_bThreadStarted = false;
        }
    }

    void TaskExecutingThread::cancelThread()
    {
        std::lock_guard<std::mutex> g(m_startStopLock);
        if (m_bThreadStarted)
        {
            // Signal the runnable the cancel request.
            m_thread.cancel();
        }
    }

    void TaskExecutingThread::unlockThread()
    {
        m_pBlockingTaskQueue->addTask(nullptr);
    }

    void TaskExecutingThread::joinThread()
    {
        std::lock_guard<std::mutex> g(m_startStopLock);
        if (m_bThreadStarted && isCancelRequested())
        {
            m_thread.join();
            m_pBlockingTaskQueue = nullptr;
            m_bThreadStarted = false;
        }
    }

    void TaskExecutingThread::run()
    {
        if (nullptr != m_pBlockingTaskQueue)
        {
            m_aliveHandler.notifyAlive(m_aliveIdentifier);
            while (!isCancelRequested())
            {
                ITask* const pTaskToExecute = m_pBlockingTaskQueue->popTask(std::chrono::milliseconds{m_aliveHandler.calculateTimeout()});
                m_aliveHandler.notifyAlive(m_aliveIdentifier);
                if (nullptr != pTaskToExecute)
                {
                    pTaskToExecute->execute();
                    pTaskToExecute->release();
                }
            }
        }
        else
        {
            LOG_WARN(CONTEXT_FRAMEWORK, "TaskExecutingThread::run() pointer to blocking task queue is null, leaving thread.");
        }

        LOG_TRACE(CONTEXT_FRAMEWORK, "TaskExecutingThread::run() leaving thread.");
    }
}
