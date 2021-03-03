//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "Watchdog/PlatformWatchdog.h"
#include "PlatformAbstraction/PlatformThread.h"
#include "PlatformWatchDogMock.h"

using namespace std::chrono_literals;
namespace ramses_internal
{
    TEST(PlatformWatchDogTest, callsRegisterAndUnregister)
    {
        PlatformWatchdogMockCallback callback;
        EXPECT_CALL(callback, registerThread(ramses::ERamsesThreadIdentifier_Workers));
        PlatformWatchdog watchdogNotifer(100ms, ramses::ERamsesThreadIdentifier_Workers, &callback);
        EXPECT_CALL(callback, unregisterThread(ramses::ERamsesThreadIdentifier_Workers));
    }

    TEST(PlatformWatchDogTest, callsThePlatformFunctionRightAway)
    {
        PlatformWatchdogMockCallback callback;
        EXPECT_CALL(callback, registerThread(ramses::ERamsesThreadIdentifier_Workers));
        PlatformWatchdog watchdogNotifer(100ms, ramses::ERamsesThreadIdentifier_Workers, &callback);

        EXPECT_CALL(callback, notifyThread(ramses::ERamsesThreadIdentifier_Workers));
        watchdogNotifer.notifyWatchdog();

        EXPECT_CALL(callback, unregisterThread(ramses::ERamsesThreadIdentifier_Workers));
    }

    TEST(PlatformWatchDogTest, debouncesCallsToPlatformWatchdog)
    {
        PlatformWatchdogMockCallback callback;
        EXPECT_CALL(callback, registerThread(ramses::ERamsesThreadIdentifier_Workers));
        PlatformWatchdog watchdogNotifer(10000ms, ramses::ERamsesThreadIdentifier_Workers, &callback);

        EXPECT_CALL(callback, notifyThread(ramses::ERamsesThreadIdentifier_Workers)).Times(1);
        watchdogNotifer.notifyWatchdog(); // this calls right away
        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed
        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed
        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed

        EXPECT_CALL(callback, unregisterThread(ramses::ERamsesThreadIdentifier_Workers));
    }

    TEST(PlatformWatchDogTest, againCallsPlatformAfterDebounceTime)
    {
        PlatformWatchdogMockCallback callback;
        EXPECT_CALL(callback, registerThread(ramses::ERamsesThreadIdentifier_Workers));
        PlatformWatchdog watchdogNotifer(200ms, ramses::ERamsesThreadIdentifier_Workers, &callback);

        EXPECT_CALL(callback, notifyThread(ramses::ERamsesThreadIdentifier_Workers)).Times(1);
        watchdogNotifer.notifyWatchdog(); // this calls right away

        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed
        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed
        ::testing::Mock::VerifyAndClearExpectations(&watchdogNotifer);

        EXPECT_CALL(callback, notifyThread(ramses::ERamsesThreadIdentifier_Workers)).Times(1);
        PlatformThread::Sleep(500);
        watchdogNotifer.notifyWatchdog(); // this calls, because first after wait time
        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed
        watchdogNotifer.notifyWatchdog(); // no call, not enough time passed

        EXPECT_CALL(callback, unregisterThread(ramses::ERamsesThreadIdentifier_Workers));
    }

    TEST(PlatformWatchDogTest, alwaysAllowsFirstNotification)
    {
        PlatformWatchdogMockCallback callback;
        EXPECT_CALL(callback, registerThread(ramses::ERamsesThreadIdentifier_Workers));
        PlatformWatchdog watchdogNotifer(std::chrono::milliseconds::max(), ramses::ERamsesThreadIdentifier_Workers, &callback);
        EXPECT_CALL(callback, unregisterThread(ramses::ERamsesThreadIdentifier_Workers));

        EXPECT_CALL(callback, notifyThread(ramses::ERamsesThreadIdentifier_Workers)).Times(1);
        watchdogNotifer.notifyWatchdog();
    }

    TEST(PlatformWatchDogTest, alwaysNotifiesWithZeroInterval)
    {
        PlatformWatchdogMockCallback callback;
        EXPECT_CALL(callback, registerThread(ramses::ERamsesThreadIdentifier_Workers));
        PlatformWatchdog watchdogNotifer(0ms, ramses::ERamsesThreadIdentifier_Workers, &callback);
        EXPECT_CALL(callback, unregisterThread(ramses::ERamsesThreadIdentifier_Workers));

        EXPECT_CALL(callback, notifyThread(ramses::ERamsesThreadIdentifier_Workers)).Times(6);
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
