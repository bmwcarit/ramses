//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ThreadWatchdogConfig.h"
#include "PlatformWatchDogMock.h"
#include "Watchdog/ThreadWatchdog.h"

using namespace testing;

namespace ramses_internal
{
    class AThreadWatchdog : public ::testing::Test
    {
    public:
        void SetUp() override
        {
            config.setThreadWatchDogCallback(&mockCallback);
            config.setWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier::Workers, 0);
            EXPECT_CALL(mockCallback, registerThread(_));
        }

        void createWatchdog()
        {
            watchdog = std::make_unique<ThreadWatchdog>(config, ramses::ERamsesThreadIdentifier::Workers);
        }

        void TearDown() override
        {
            EXPECT_CALL(mockCallback, unregisterThread(_));
        }

        StrictMock<PlatformWatchdogMockCallback> mockCallback;
        ThreadWatchdogConfig config;
        std::unique_ptr<ThreadWatchdog> watchdog;
    };

    TEST(AThreadWatchdog_, registersUnregistersWithCorrectThreadIdentifier)
    {
        StrictMock<PlatformWatchdogMockCallback> mockCallback;
        ThreadWatchdogConfig config;
        config.setThreadWatchDogCallback(&mockCallback);

        {
            EXPECT_CALL(mockCallback, registerThread(ramses::ERamsesThreadIdentifier::Workers));
            ThreadWatchdog watchdog(config, ramses::ERamsesThreadIdentifier::Workers);

            Mock::VerifyAndClearExpectations(&mockCallback);
            EXPECT_CALL(mockCallback, unregisterThread(ramses::ERamsesThreadIdentifier::Workers));
        }
        {
            EXPECT_CALL(mockCallback, registerThread(ramses::ERamsesThreadIdentifier::Renderer));
            ThreadWatchdog watchdog(config, ramses::ERamsesThreadIdentifier::Renderer);

            Mock::VerifyAndClearExpectations(&mockCallback);
            EXPECT_CALL(mockCallback, unregisterThread(ramses::ERamsesThreadIdentifier::Renderer));
        }
    }

    TEST_F(AThreadWatchdog, issuesIncreasingIdentifiersOnRegister)
    {
        createWatchdog();
        auto t0 = watchdog->registerThread();
        auto t1 = watchdog->registerThread();
        auto t2 = watchdog->registerThread();
        watchdog->unregisterThread(t0);
        auto t3 = watchdog->registerThread();
        watchdog->unregisterThread(t3);
        auto t4 = watchdog->registerThread();

        EXPECT_LT(t0, t1);
        EXPECT_LT(t1, t2);
        EXPECT_LT(t2, t3);
        EXPECT_LT(t3, t4);
    }

    TEST_F(AThreadWatchdog, reportsToWatchDogNotifierWhenAllThreadsReportAlive)
    {
        createWatchdog();
        auto t0 = watchdog->registerThread();
        auto t1 = watchdog->registerThread();

        EXPECT_CALL(mockCallback, notifyThread(_));
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t1);

        Mock::VerifyAndClearExpectations(&mockCallback);

        EXPECT_CALL(mockCallback, notifyThread(_));
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t1);

        Mock::VerifyAndClearExpectations(&mockCallback);

        EXPECT_CALL(mockCallback, notifyThread(_));
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t1);
        watchdog->notifyAlive(t1);

        Mock::VerifyAndClearExpectations(&mockCallback);

        EXPECT_CALL(mockCallback, notifyThread(_));
        watchdog->notifyAlive(t1);
        watchdog->notifyAlive(t1);
        watchdog->notifyAlive(t1);
        watchdog->notifyAlive(t1);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t1);
        watchdog->notifyAlive(t1);
    }

    TEST_F(AThreadWatchdog, doesNotReportToWatchDogIfOneThreadIsBlocked)
    {
        createWatchdog();
        auto t0 = watchdog->registerThread();
        watchdog->registerThread();

        // no expects, one thread reports, the other isn't reporting
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
    }

    TEST_F(AThreadWatchdog, alwaysForwardsNotificationFromSingleThread)
    {
        createWatchdog();
        auto t0 = watchdog->registerThread();

        EXPECT_CALL(mockCallback, notifyThread(_)).Times(5);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
    }

    TEST_F(AThreadWatchdog, stopsForwardingNotificationWhenRegisteringMoreThreadsToWatch)
    {
        createWatchdog();
        auto t0 = watchdog->registerThread();

        EXPECT_CALL(mockCallback, notifyThread(_)).Times(2);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
        watchdog->registerThread();
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
    }

    TEST_F(AThreadWatchdog, notifiesWhenUnregisteringBlockedThread)
    {
        createWatchdog();
        auto t0 = watchdog->registerThread();
        auto t1 = watchdog->registerThread();

        EXPECT_CALL(mockCallback, notifyThread(_));
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t1);
        Mock::VerifyAndClearExpectations(&mockCallback);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t0);
        Mock::VerifyAndClearExpectations(&mockCallback);
        EXPECT_CALL(mockCallback, notifyThread(_));
        watchdog->unregisterThread(t1);
    }

    TEST_F(AThreadWatchdog, unregisteringLastThreadDoesNotTriggerNotification)
    {
        createWatchdog();

        auto t0 = watchdog->registerThread();
        watchdog->unregisterThread(t0);
    }

    TEST_F(AThreadWatchdog, notifiesProperlyWhileRegisteringAndUnregisteringThreads)
    {
        createWatchdog();
        auto t0 = watchdog->registerThread();
        auto t1 = watchdog->registerThread();
        EXPECT_CALL(mockCallback, notifyThread(_));
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t1);
        Mock::VerifyAndClearExpectations(&mockCallback);

        auto t2 = watchdog->registerThread();
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t1);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t1);
        EXPECT_CALL(mockCallback, notifyThread(_));
        watchdog->notifyAlive(t2);
        Mock::VerifyAndClearExpectations(&mockCallback);

        watchdog->notifyAlive(t2);
        watchdog->notifyAlive(t2);
        auto t3 = watchdog->registerThread();
        watchdog->notifyAlive(t3);
        watchdog->notifyAlive(t2);
        watchdog->notifyAlive(t0);
        EXPECT_CALL(mockCallback, notifyThread(_));
        watchdog->notifyAlive(t1);
        Mock::VerifyAndClearExpectations(&mockCallback);

        watchdog->notifyAlive(t1);
        watchdog->notifyAlive(t2);
        watchdog->notifyAlive(t0);
        watchdog->unregisterThread(t2);
        Mock::VerifyAndClearExpectations(&mockCallback);
        EXPECT_CALL(mockCallback, notifyThread(_));
        watchdog->notifyAlive(t3);
        Mock::VerifyAndClearExpectations(&mockCallback);
        watchdog->notifyAlive(t1);
        watchdog->unregisterThread(t0);
        EXPECT_CALL(mockCallback, notifyThread(_));
        watchdog->unregisterThread(t3);
    }

    // TODO(Carsten): test if actual value is respected as soon as time system can be mocked
    TEST_F(AThreadWatchdog, respectsNotificationInterval)
    {
        config.setWatchdogNotificationInterval(ramses::ERamsesThreadIdentifier::Workers, static_cast<uint32_t>(-1));
        createWatchdog();
        auto t0 = watchdog->registerThread();
        auto t1 = watchdog->registerThread();

        EXPECT_CALL(mockCallback, notifyThread(_));
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t1);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t1);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t1);
        watchdog->notifyAlive(t0);
        watchdog->notifyAlive(t1);
    }
}
