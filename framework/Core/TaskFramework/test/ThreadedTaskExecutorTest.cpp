//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "TaskFramework/ThreadedTaskExecutor.h"
#include "framework_common_gmock_header.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "TaskFramework/EnqueueOnlyOneAtATimeQueue.h"
#include "ThreadWatchdogConfig.h"
#include "PlatformWatchDogMock.h"
#include "PlatformAbstraction/PlatformEvent.h"

#include <thread>
#include <chrono>

using namespace testing;

namespace ramses_internal
{
    class LongRunningTestTask : public ITask
    {
    public:
        explicit LongRunningTestTask(UInt32 howLong = 50)
            : time(howLong)
        {
            ON_CALL(*this, execute()).WillByDefault(Invoke(this, &LongRunningTestTask::doSomethingLong));
        }
        virtual ~LongRunningTestTask(){};
        MOCK_METHOD(void, execute, (), (override));
        virtual void doSomethingLong()
        {
            executeStarted.signal();
            PlatformThread::Sleep(time);
            executeFinished.signal();
        }

        PlatformEvent executeStarted;
        PlatformEvent executeFinished;
        UInt32 time;
    };

    class BlockingTask : public ITask
    {
    public:
        BlockingTask(PlatformLock& blockingLock, PlatformEvent& blockedExecutionStateEvent)
            : m_blockingLock(blockingLock)
            , m_blockedExecutionStateEvent(blockedExecutionStateEvent)
        {
        }
        virtual void execute() override
        {
            m_blockedExecutionStateEvent.signal();
            m_blockingLock.lock();
            m_blockingLock.unlock();
        }

        PlatformLock& m_blockingLock;
        PlatformEvent& m_blockedExecutionStateEvent;
    };

    TEST(AThreadedTaskExecutor, reportsToWatchDogNotifierWhenAllThreadsReportAlive)
    {
        PlatformEvent syncWaiter;
        PlatformWatchdogMockCallback mockCallback;
        EXPECT_CALL(mockCallback, registerThread(ramses::ERamsesThreadIdentifier_Workers));
        ThreadWatchdogConfig config;
        config.setThreadWatchDogCallback(&mockCallback);
        config.setWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier_Workers, 100);

        EXPECT_CALL(mockCallback, notifyThread(ramses::ERamsesThreadIdentifier_Workers))
            .Times(AtLeast(1))
            .WillRepeatedly(InvokeWithoutArgs([&]() { syncWaiter.signal(); }));
        ThreadedTaskExecutor ex(2, config);

        EXPECT_TRUE(syncWaiter.wait(2000));

        EXPECT_CALL(mockCallback, unregisterThread(ramses::ERamsesThreadIdentifier_Workers));
    }

    class CountingWatchdogNotifier : public ramses::IThreadWatchdogNotification
    {
    public:
        explicit CountingWatchdogNotifier(std::atomic<uint32_t>& counter)
            : m_counter(counter)
        {
        }

        virtual void notifyThread(ramses::ERamsesThreadIdentifier) override
        {
            ++m_counter;
        }

        virtual void registerThread(ramses::ERamsesThreadIdentifier) override
        {
        }

        virtual void unregisterThread(ramses::ERamsesThreadIdentifier) override
        {
        }
        std::atomic<uint32_t>& m_counter;
    };

    TEST(AThreadedTaskExecutor, doesNotReportToWatchDogIfOneThreadIsBlocked)
    {
        std::atomic<uint32_t> counter(0);
        CountingWatchdogNotifier mockCallback(counter);
        ThreadWatchdogConfig config;
        config.setThreadWatchDogCallback(&mockCallback);
        PlatformLock taskBlockingLock;

        // take lock, so worker thread will block in execution
        taskBlockingLock.lock();
        PlatformEvent taskIsBeingWorkedEvent;
        BlockingTask blockingTask(taskBlockingLock, taskIsBeingWorkedEvent);
        ThreadedTaskExecutor taskSystem(2, config);
        taskSystem.enqueue(blockingTask);

        // wait for thread to go into blocked state during execution
        EXPECT_TRUE(taskIsBeingWorkedEvent.wait(1000));

        const uint32_t notificationsAtStartOfBlocking = counter;

        // watchdog can still be called in this state if now blocked thread has already reported back, and now the second thread reports back
        // so when we wait some more time at most one more call to watchdogNotifier can happen
        PlatformThread::Sleep(200);

        const uint32_t notificationsAfterWaitingInBlockedState = counter;
        EXPECT_LE(notificationsAfterWaitingInBlockedState, notificationsAtStartOfBlocking + 1);

        taskBlockingLock.unlock();
    }

    TEST(AThreadedTaskExecutor, executesEnqueuedTasks)
    {
        ThreadedTaskExecutor ex(4);

        LongRunningTestTask task;
        EXPECT_CALL(task, execute());
        ex.enqueue(task);

        ex.disableAcceptingTasksAfterExecutingCurrentQueue();
        ex.stop();
    }

    TEST(ThreadedTaskExecutor, destructorWaitsForUnfinishedTasks)
    {
        ThreadedTaskExecutor* ex = new ThreadedTaskExecutor(16);

        NiceMock<LongRunningTestTask> task;

        ex->enqueue(task);

        delete ex; // would cause errors if

        task.executeFinished.wait();
    }

    TEST(AThreadedTaskExecutor, unblockThreadsWhenStopping)
    {
        PlatformEvent syncWaiter;
        PlatformWatchdogMockCallback mockCallback;
        EXPECT_CALL(mockCallback, registerThread(ramses::ERamsesThreadIdentifier_Workers));
        ThreadWatchdogConfig config;
        config.setThreadWatchDogCallback(&mockCallback);
        config.setWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier_Workers, 10000);

        EXPECT_CALL(mockCallback, notifyThread(ramses::ERamsesThreadIdentifier_Workers))
            .Times(AtLeast(1))
            .WillRepeatedly(InvokeWithoutArgs([&]() { syncWaiter.signal(); }));
        ThreadedTaskExecutor ex(2, config);

        std::thread t([&]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            ex.stop();
        });

        EXPECT_TRUE(syncWaiter.wait(config.getWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier_Workers)));

        EXPECT_CALL(mockCallback, unregisterThread(ramses::ERamsesThreadIdentifier_Workers));
        t.join();
    }

    TEST(ThreadedTaskExecutor, stoppingReleasesUnhandledTasks)
    {
        ThreadedTaskExecutor* ex = new ThreadedTaskExecutor(1);

        PlatformLock taskBlockingLock;
        taskBlockingLock.lock();

        PlatformEvent taskIsBeingWorkedEvent;

        LongRunningTestTask firstTask(500);
        BlockingTask blockingTask(taskBlockingLock, taskIsBeingWorkedEvent);

        EXPECT_CALL(firstTask, execute());
        ex->enqueue(firstTask);
        ex->enqueue(blockingTask);

        firstTask.executeStarted.wait();
        ex->stop();

        delete ex; // would lock if blocking task would be executed due to missing task unlock
        taskBlockingLock.unlock();
    }

    TEST(AnEnqueueOnlyOneAtATimeQueue, waitForAllTasksExecutionWhenAskedToShutDown)
    {
        ThreadedTaskExecutor ex(1);
        // c-->b-->a-->threaded executor
        EnqueueOnlyOneAtATimeQueue a(ex);
        EnqueueOnlyOneAtATimeQueue b(static_cast<ITaskQueue&>(a));
        EnqueueOnlyOneAtATimeQueue c(static_cast<ITaskQueue&>(b));

        LongRunningTestTask task(500);
        LongRunningTestTask task2(500);

        EXPECT_CALL(task, execute());
        EXPECT_CALL(task2, execute());

        c.enqueue(task);
        c.enqueue(task2);
        c.disableAcceptingTasksAfterExecutingCurrentQueue();

        // would cause errors if tasks were not done yet
        Mock::VerifyAndClearExpectations(&task);
        Mock::VerifyAndClearExpectations(&task2);
    }
}
