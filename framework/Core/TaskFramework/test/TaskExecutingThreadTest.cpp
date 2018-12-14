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

        EXPECT_CALL(handlerMock, notifyAlive(13)).Times(AtLeast(2)).WillOnce(Return()).WillRepeatedly(ReleaseSyncCall(&syncWaiter));
        EXPECT_CALL(handlerMock, calculateTimeout()).Times(AtLeast(1)).WillRepeatedly(Return(100));
        thread.start(q);

        const EStatus status = syncWaiter.wait(1000);
        EXPECT_EQ(EStatus_RAMSES_OK, status);
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

        EXPECT_CALL(handlerMock, notifyAlive(13)).Times(AtLeast(2)).WillOnce(Return()).WillRepeatedly(ReleaseSyncCall(&syncWaiter));
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

        const EStatus status = syncWaiter.wait(1000);
        EXPECT_EQ(EStatus_RAMSES_OK, status);
        thread.stop();
        t.join();
    }
}
