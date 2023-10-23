//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Core/TaskFramework/TaskExecutingThread.h"
#include "gmock/gmock.h"
#include "internal/Core/TaskFramework/ProcessingTaskQueue.h"
#include "internal/PlatformAbstraction/PlatformEvent.h"
#include "Watchdog/ThreadAliveNotifierMock.h"

#include <cstdint>
#include <thread>
#include <chrono>

using namespace testing;
using namespace std::chrono_literals;
namespace ramses::internal
{
    TEST(ATaskExecutingThread, callsAliveNotificationWithGivenThreadID)
    {
        PlatformEvent syncWaiter;
        ThreadAliveNotifierMock handlerMock;
        ProcessingTaskQueue q;

        EXPECT_CALL(handlerMock, registerThread()).WillOnce(Return(13));
        TaskExecutingThread thread(handlerMock);

        EXPECT_CALL(handlerMock, notifyAlive(13))
            .Times(AtLeast(2))
            .WillOnce(Return())
            .WillRepeatedly(InvokeWithoutArgs([&]() { syncWaiter.signal(); }));
        EXPECT_CALL(handlerMock, calculateTimeout()).Times(AtLeast(1)).WillRepeatedly(Return(100ms));
        thread.start(q);

        EXPECT_TRUE(syncWaiter.wait(1000));
        EXPECT_CALL(handlerMock, unregisterThread(13));
    }

    TEST(ATaskExecutingThread, stopThreadWorks)
    {
        ThreadAliveNotifierMock handlerMock;
        ProcessingTaskQueue q;

        EXPECT_CALL(handlerMock, registerThread()).WillOnce(Return(13));
        TaskExecutingThread thread(handlerMock);
        thread.start(q);
        EXPECT_FALSE(thread.isCancelRequested());
        thread.stop();
        EXPECT_TRUE(thread.isCancelRequested());
        EXPECT_CALL(handlerMock, unregisterThread(13));
    }

    TEST(ATaskExecutingThread, cancelUnlockJoinThreadWorks)
    {
        ThreadAliveNotifierMock handlerMock;
        ProcessingTaskQueue q;

        EXPECT_CALL(handlerMock, registerThread()).WillOnce(Return(13));
        TaskExecutingThread thread(handlerMock);
        thread.start(q);
        EXPECT_FALSE(thread.isCancelRequested());
        thread.cancelThread();
        EXPECT_TRUE(thread.isCancelRequested());
        thread.unlockThread();
        thread.joinThread();
        EXPECT_CALL(handlerMock, unregisterThread(13));
    }

    TEST(ATaskExecutingThread, queueAcceptsAndUnlocksWithNullptr)
    {
        PlatformEvent syncWaiter;
        ThreadAliveNotifierMock handlerMock;
        ProcessingTaskQueue q;

        EXPECT_CALL(handlerMock, registerThread()).WillOnce(Return(13));
        TaskExecutingThread thread(handlerMock);

        EXPECT_CALL(handlerMock, notifyAlive(13))
            .Times(AtLeast(2))
            .WillOnce(Return())
            .WillRepeatedly(InvokeWithoutArgs([&]() { syncWaiter.signal(); }));
        EXPECT_CALL(handlerMock, calculateTimeout()).Times(AtLeast(1)).WillRepeatedly(Return(2000ms));
        thread.start(q);

        std::thread t([&]()
        {
            while (!thread.isCancelRequested())
            {
                q.addTask(nullptr);
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });

        EXPECT_TRUE(syncWaiter.wait(1000));
        thread.stop();
        t.join();
        EXPECT_CALL(handlerMock, unregisterThread(13));
    }
}
