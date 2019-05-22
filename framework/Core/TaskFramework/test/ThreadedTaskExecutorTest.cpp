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
    ACTION_P(ReleaseSyncCall, syncer)
    {
        UNUSED(arg9);
        UNUSED(arg8);
        UNUSED(arg7);
        UNUSED(arg6);
        UNUSED(arg5);
        UNUSED(arg4);
        UNUSED(arg3);
        UNUSED(arg2);
        UNUSED(arg1);
        UNUSED(arg0);
        UNUSED(args);
        syncer->signal();
    }

    class LongRunningTestTask : public ITask
    {
    public:
        LongRunningTestTask(UInt32 howLong = 50)
            : time(howLong)
        {
            ON_CALL(*this, execute()).WillByDefault(Invoke(this, &LongRunningTestTask::doSomethingLong));
        }
        virtual ~LongRunningTestTask(){};
        MOCK_METHOD0(execute, void());
        virtual void doSomethingLong()
        {
            executeStarted.broadcast();
            PlatformThread::Sleep(time);
            executeFinished.broadcast();
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
        virtual void execute()
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
        ThreadedTaskExecutor ex(2, config);

        EXPECT_CALL(mockCallback, notifyThread(ramses::ERamsesThreadIdentifier_Workers)).Times(AtLeast(1)).WillRepeatedly(ReleaseSyncCall(&syncWaiter));
        ex.start();

        const EStatus status = syncWaiter.wait(2000);
        EXPECT_EQ(EStatus_RAMSES_OK, status);

        EXPECT_CALL(mockCallback, unregisterThread(ramses::ERamsesThreadIdentifier_Workers));
    }

    class CountingWatchdogNotifier : public ramses::IThreadWatchdogNotification
    {
    public:
        CountingWatchdogNotifier(std::atomic<uint32_t>& counter)
            : m_counter(counter)
        {
        }

        virtual void notifyThread(ramses::ERamsesThreadIdentifier)
        {
            ++m_counter;
        }

        virtual void registerThread(ramses::ERamsesThreadIdentifier)
        {
        }

        virtual void unregisterThread(ramses::ERamsesThreadIdentifier)
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
        taskSystem.start();
        taskSystem.enqueue(blockingTask);

        // wait for thread to go into blocked state during execution
        const EStatus status = taskIsBeingWorkedEvent.wait(1000);
        EXPECT_EQ(EStatus_RAMSES_OK, status);

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
        ex.start();

        LongRunningTestTask task;
        EXPECT_CALL(task, execute());
        ex.enqueue(task);

        ex.disableAcceptingTasksAfterExecutingCurrentQueue();
        ex.stop();
    }

    TEST(ThreadedTaskExecutor, destructorWaitsForUnfinishedTasks)
    {
        ThreadedTaskExecutor* ex = new ThreadedTaskExecutor(16);
        ex->start();

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
        ThreadedTaskExecutor ex(2, config);

        EXPECT_CALL(mockCallback, notifyThread(ramses::ERamsesThreadIdentifier_Workers)).Times(AtLeast(1)).WillRepeatedly(ReleaseSyncCall(&syncWaiter));
        ex.start();

        std::thread t([&]()
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            ex.stop();
        });

        const EStatus status = syncWaiter.wait(config.getWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier_Workers));
        EXPECT_EQ(EStatus_RAMSES_OK, status);

        EXPECT_CALL(mockCallback, unregisterThread(ramses::ERamsesThreadIdentifier_Workers));
        t.join();
    }

    class TimeMeasuringWatchDogNotifier : public ramses::IThreadWatchdogNotification
    {
    public:
        TimeMeasuringWatchDogNotifier() {}

        virtual void notifyThread(ramses::ERamsesThreadIdentifier)
        {
            auto time = std::chrono::system_clock::now();
            if (initialized)
            {
                values.push_back(std::chrono::duration_cast<std::chrono::milliseconds>(time - last));
            }

            initialized = true;
            last = time;
        }

        virtual void registerThread(ramses::ERamsesThreadIdentifier) {}
        virtual void unregisterThread(ramses::ERamsesThreadIdentifier) {}

        std::chrono::system_clock::time_point last;
        std::vector<std::chrono::milliseconds> values;
        bool initialized = false;
    };

    class WatchDogTimeoutTest : public testing::TestWithParam<UInt32> {};

    INSTANTIATE_TEST_CASE_P(testWithDifferentNotificationValues,
        WatchDogTimeoutTest,
        testing::Values(300, 800));

    TEST_P(WatchDogTimeoutTest, watchdogConfigTimeoutsAreBaseForAliveNotification)
    {
        const auto timeout = GetParam();
        PlatformEvent syncWaiter;
        TimeMeasuringWatchDogNotifier watchDog;
        ThreadWatchdogConfig config;
        config.setThreadWatchDogCallback(&watchDog);
        config.setWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier_Workers, timeout);
        ThreadedTaskExecutor ex(3, config);

        ex.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout * 3));
        ex.stop();

        for (auto const& dur : watchDog.values)
        {
            EXPECT_GE(dur.count(), std::chrono::milliseconds(timeout / 4 - 1).count());
            EXPECT_LE(dur.count(), std::chrono::milliseconds(timeout * 2).count());
        }
    }

    TEST(ThreadedTaskExecutor, stoppingReleasesUnhandledTasks)
    {
        ThreadedTaskExecutor* ex = new ThreadedTaskExecutor(1);
        ex->start();

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
        ex.start();
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
