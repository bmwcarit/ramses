//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "internal/Watchdog/PlatformWatchdog.h"
#include "internal/PlatformAbstraction/PlatformThread.h"
#include "PlatformWatchDogMock.h"

using namespace std::chrono_literals;
namespace ramses::internal
{
    TEST(PlatformWatchDogTest, callsRegisterAndUnregister)
    {
        PlatformWatchdogMockCallback callback;
        EXPECT_CALL(callback, registerThread(ERamsesThreadIdentifier::Workers));
        PlatformWatchdog watchdogNotifer(100ms, ERamsesThreadIdentifier::Workers, &callback);
        EXPECT_CALL(callback, unregisterThread(ERamsesThreadIdentifier::Workers));
    }

    TEST(PlatformWatchDogTest, callsThePlatformFunctionRightAway)
    {
        PlatformWatchdogMockCallback callback;
        EXPECT_CALL(callback, registerThread(ERamsesThreadIdentifier::Workers));
        PlatformWatchdog watchdogNotifer(100ms, ERamsesThreadIdentifier::Workers, &callback);

        EXPECT_CALL(callback, notifyThread(ERamsesThreadIdentifier::Workers));
        watchdogNotifer.notifyWatchdog();

        EXPECT_CALL(callback, unregisterThread(ERamsesThreadIdentifier::Workers));
    }

    TEST(PlatformWatchDogTest, debouncesCallsToPlatformWatchdog)
    {
        PlatformWatchdogMockCallback callback;
        EXPECT_CALL(callback, registerThread(ERamsesThreadIdentifier::Workers));
        PlatformWatchdog watchdogNotifer(10000ms, ERamsesThreadIdentifier::Workers, &callback);

        EXPECT_CALL(callback, notifyThread(ERamsesThreadIdentifier::Workers)).Times(1);
        watchdogNotifer.notifyWatchdog(); // this calls right away
        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed
        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed
        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed

        EXPECT_CALL(callback, unregisterThread(ERamsesThreadIdentifier::Workers));
    }

    TEST(PlatformWatchDogTest, againCallsPlatformAfterDebounceTime)
    {
        PlatformWatchdogMockCallback callback;
        EXPECT_CALL(callback, registerThread(ERamsesThreadIdentifier::Workers));
        PlatformWatchdog watchdogNotifer(200ms, ERamsesThreadIdentifier::Workers, &callback);

        EXPECT_CALL(callback, notifyThread(ERamsesThreadIdentifier::Workers)).Times(1);
        watchdogNotifer.notifyWatchdog(); // this calls right away

        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed
        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed
        ::testing::Mock::VerifyAndClearExpectations(&watchdogNotifer);

        EXPECT_CALL(callback, notifyThread(ERamsesThreadIdentifier::Workers)).Times(1);
        PlatformThread::Sleep(500);
        watchdogNotifer.notifyWatchdog(); // this calls, because first after wait time
        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed
        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed

        EXPECT_CALL(callback, unregisterThread(ERamsesThreadIdentifier::Workers));
    }

    TEST(PlatformWatchDogTest, alwaysAllowsFirstNotification)
    {
        PlatformWatchdogMockCallback callback;
        EXPECT_CALL(callback, registerThread(ERamsesThreadIdentifier::Workers));
        PlatformWatchdog watchdogNotifer(std::chrono::milliseconds::max(), ERamsesThreadIdentifier::Workers, &callback);
        EXPECT_CALL(callback, unregisterThread(ERamsesThreadIdentifier::Workers));

        EXPECT_CALL(callback, notifyThread(ERamsesThreadIdentifier::Workers)).Times(1);
        watchdogNotifer.notifyWatchdog();
    }

    TEST(PlatformWatchDogTest, alwaysNotifiesWithZeroInterval)
    {
        PlatformWatchdogMockCallback callback;
        EXPECT_CALL(callback, registerThread(ERamsesThreadIdentifier::Workers));
        PlatformWatchdog watchdogNotifer(0ms, ERamsesThreadIdentifier::Workers, &callback);
        EXPECT_CALL(callback, unregisterThread(ERamsesThreadIdentifier::Workers));

        EXPECT_CALL(callback, notifyThread(ERamsesThreadIdentifier::Workers)).Times(6);
        EXPECT_EQ(0ms, watchdogNotifer.calculateTimeout());
        watchdogNotifer.notifyWatchdog();
        EXPECT_EQ(0ms, watchdogNotifer.calculateTimeout());
        watchdogNotifer.notifyWatchdog();
        EXPECT_EQ(0ms, watchdogNotifer.calculateTimeout());
        watchdogNotifer.notifyWatchdog();
        EXPECT_EQ(0ms, watchdogNotifer.calculateTimeout());
        watchdogNotifer.notifyWatchdog();
        watchdogNotifer.notifyWatchdog();
        watchdogNotifer.notifyWatchdog();
    }
}
