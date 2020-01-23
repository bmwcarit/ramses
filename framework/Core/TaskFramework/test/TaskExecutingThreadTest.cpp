//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "PlatformAbstraction/PlatformTypes.h"
#include "TaskFramework/TaskExecutingThread.h"
#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "TaskFramework/ProcessingTaskQueue.h"
#include "PlatformAbstraction/PlatformEvent.h"

#include <thread>
#include <chrono>

using namespace testing;

namespace ramses_internal
{
    class AliveHandlerMock : public IThreadAliveNotifier
    {
    public:
        MOCK_METHOD1(notifyAlive, void(UInt16 threadIndex));
        MOCK_CONST_METHOD0(calculateTimeout, UInt32());
    };

    TEST(ATaskExecutingThread, callsAliveNotificationWithGivenThreadID)
    {
        PlatformEvent syncWaiter;
        AliveHandlerMock handlerMock;
        ProcessingTaskQueue q;
        TaskExecutingThread thread(13, handlerMock);

        EXPECT_CALL(handlerMock, notifyAlive(13))
            .Times(AtLeast(2))
            .WillOnce(Return())
            .WillRepeatedly(InvokeWithoutArgs([&]() { syncWaiter.signal(); }));
        EXPECT_CALL(handlerMock, calculateTimeout()).Times(AtLeast(1)).WillRepeatedly(Return(100));
        thread.start(q);

        EXPECT_TRUE(syncWaiter.wait(1000));
    }

    TEST(ATaskExecutingThread, stopThreadWorks)
    {
        AliveHandlerMock handlerMock;
        ProcessingTaskQueue q;
        TaskExecutingThread thread(13, handlerMock);
        thread.start(q);
        EXPECT_FALSE(thread.isCancelRequested());
        thread.stop();
        EXPECT_TRUE(thread.isCancelRequested());
    }

    TEST(ATaskExecutingThread, cancelUnlockJoinThreadWorks)
    {
        AliveHandlerMock handlerMock;
        ProcessingTaskQueue q;
        TaskExecutingThread thread(13, handlerMock);
        thread.start(q);
        EXPECT_FALSE(thread.isCancelRequested());
        thread.cancelThread();
        EXPECT_TRUE(thread.isCancelRequested());
        thread.unlockThread();
        thread.joinThread();
    }

    TEST(ATaskExecutingThread, queueAcceptsAndUnlocksWithNullptr)
    {
        PlatformEvent syncWaiter;
        AliveHandlerMock handlerMock;
        ProcessingTaskQueue q;
        TaskExecutingThread thread(13, handlerMock);

        EXPECT_CALL(handlerMock, notifyAlive(13))
            .Times(AtLeast(2))
            .WillOnce(Return())
            .WillRepeatedly(InvokeWithoutArgs([&]() { syncWaiter.signal(); }));
        EXPECT_CALL(handlerMock, calculateTimeout()).Times(AtLeast(1)).WillRepeatedly(Return(2000));
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
    }
}
