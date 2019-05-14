//  -------------------------------------------------------------------------
//  Copyright (C) 2018 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "PlatformAbstraction/synchronized_clock.h"
#include <thread>

namespace ramses_internal
{
    TEST(PlatformTimeSynchronized, nowNotZero)
    {
        EXPECT_NE(0u, synchronized_clock::now().time_since_epoch().count());
    }

    TEST(PlatformTimeMonotonic, notZero)
    {
        EXPECT_NE(0u, PlatformTime::GetMillisecondsMonotonic());
        EXPECT_NE(0u, PlatformTime::GetMicrosecondsMonotonic());
    }

    TEST(PlatformTimeAbsolute, notZero)
    {
        EXPECT_NE(0u, PlatformTime::GetMillisecondsAbsolute());
        EXPECT_NE(0u, PlatformTime::GetMicrosecondsAbsolute());
    }

    TEST(PlatformTimeAbsolute, isBiggerThanSomeReferenceTime)
    {
        const uint64_t milliSeconds = PlatformTime::GetMillisecondsAbsolute();
        EXPECT_GE(milliSeconds, 1357210706813ull); // check against some reference time (03.01.2013, 12:00)
    }

    TEST(PlatformTimeMonotonic, doesNotMoveBackward)
    {
        const uint64_t milliSeconds = PlatformTime::GetMicrosecondsMonotonic();
        const uint64_t milliSeconds2 = PlatformTime::GetMicrosecondsMonotonic();
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        const uint64_t milliSeconds3 = PlatformTime::GetMicrosecondsMonotonic();
        EXPECT_LE(milliSeconds, milliSeconds2);
        EXPECT_LE(milliSeconds2, milliSeconds3);
    }

    TEST(PlatformTimeMonotonic, expectTimeProgressed_Milliseconds)
    {
        const uint64_t milliSeconds = PlatformTime::GetMillisecondsMonotonic();
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        const uint64_t milliSeconds2 = PlatformTime::GetMillisecondsMonotonic();
        EXPECT_TRUE((milliSeconds2 - milliSeconds) > 40);
    }

    TEST(PlatformTimeMonotonic, expectTimeProgressed_Microseconds)
    {
        const uint64_t milliSeconds = PlatformTime::GetMicrosecondsMonotonic();
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        const uint64_t milliSeconds2 = PlatformTime::GetMicrosecondsMonotonic();
        EXPECT_TRUE((milliSeconds2 - milliSeconds) > 40000);
    }

    TEST(PlatformTimeSynchronized, expectTimeProgressed)
    {
        const auto first = synchronized_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        const auto second = synchronized_clock::now();
        EXPECT_TRUE((second - first) > std::chrono::milliseconds(40));
    }

#if !defined(RAMSES_LINUX_USE_DEV_PTP) && !defined(__INTEGRITY)
    // no ptp support: test synchroized time is system_clock
    TEST(PlatformTimeSynchronized, non_ptp_behavesAsSystemClock)
    {
        const auto tSys1 = std::chrono::system_clock::now().time_since_epoch();
        const auto tSync = synchronized_clock::now().time_since_epoch();
        const auto tSys2 = std::chrono::system_clock::now().time_since_epoch();

        EXPECT_LE(tSys1, tSync);
        EXPECT_LE(tSync, tSys2);
    }
#endif

    TEST(PlatformTimeSynchronized, integralConversionsWork)
    {
        EXPECT_EQ(0u, asMilliseconds(synchronized_clock::time_point(std::chrono::milliseconds(0))));
        EXPECT_EQ(10u, asMilliseconds(synchronized_clock::time_point(std::chrono::milliseconds(10))));
        EXPECT_EQ(12345u, asMilliseconds(synchronized_clock::time_point(std::chrono::milliseconds(12345))));

        EXPECT_EQ(0u, asMicroseconds(synchronized_clock::time_point(std::chrono::microseconds(0))));
        EXPECT_EQ(10u, asMicroseconds(synchronized_clock::time_point(std::chrono::microseconds(10))));
        EXPECT_EQ(12345u, asMicroseconds(synchronized_clock::time_point(std::chrono::microseconds(12345))));
        EXPECT_EQ(10000u, asMicroseconds(synchronized_clock::time_point(std::chrono::milliseconds(10))));
    }

    TEST(PlatformTimeMonotonic, integralConversionsWork)
    {
        EXPECT_EQ(0u, asMilliseconds(std::chrono::steady_clock::time_point(std::chrono::milliseconds(0))));
        EXPECT_EQ(10u, asMilliseconds(std::chrono::steady_clock::time_point(std::chrono::milliseconds(10))));
        EXPECT_EQ(12345u, asMilliseconds(std::chrono::steady_clock::time_point(std::chrono::milliseconds(12345))));

        EXPECT_EQ(0u, asMicroseconds(std::chrono::steady_clock::time_point(std::chrono::microseconds(0))));
        EXPECT_EQ(10u, asMicroseconds(std::chrono::steady_clock::time_point(std::chrono::microseconds(10))));
        EXPECT_EQ(12345u, asMicroseconds(std::chrono::steady_clock::time_point(std::chrono::microseconds(12345))));
        EXPECT_EQ(10000u, asMicroseconds(std::chrono::steady_clock::time_point(std::chrono::milliseconds(10))));
    }

    TEST(PlatformTimeAbsolute, integralConversionsWork)
    {
        EXPECT_EQ(0u, asMilliseconds(std::chrono::system_clock::time_point(std::chrono::milliseconds(0))));
        EXPECT_EQ(10u, asMilliseconds(std::chrono::system_clock::time_point(std::chrono::milliseconds(10))));
        EXPECT_EQ(12345u, asMilliseconds(std::chrono::system_clock::time_point(std::chrono::milliseconds(12345))));

        EXPECT_EQ(0u, asMicroseconds(std::chrono::system_clock::time_point(std::chrono::microseconds(0))));
        EXPECT_EQ(10u, asMicroseconds(std::chrono::system_clock::time_point(std::chrono::microseconds(10))));
        EXPECT_EQ(12345u, asMicroseconds(std::chrono::system_clock::time_point(std::chrono::microseconds(12345))));
        EXPECT_EQ(10000u, asMicroseconds(std::chrono::system_clock::time_point(std::chrono::milliseconds(10))));
    }

    TEST(PlatformTimeMonotonic, DISABLED_getMicrosecondsTestLongTime_Monotonic)
    {
        const uint64_t milliSeconds = PlatformTime::GetMicrosecondsMonotonic();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        const uint64_t milliSeconds2 = PlatformTime::GetMicrosecondsMonotonic();
        const uint64_t diff = milliSeconds2 - milliSeconds;
        EXPECT_GE(diff, 2000000u - 200000u);
        EXPECT_LE(diff, 2000000u + 200000u);
    }

    TEST(PlatformTimeAbsolute, DISABLED_getMicrosecondsTestLongTime_Absolute)
    {
        const uint64_t milliSeconds = PlatformTime::GetMicrosecondsAbsolute();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        const uint64_t milliSeconds2 = PlatformTime::GetMicrosecondsAbsolute();
        const uint64_t diff = milliSeconds2 - milliSeconds;
        EXPECT_GE(diff, 2000000u - 200000u);
        EXPECT_LE(diff, 2000000u + 200000u);
    }

    TEST(PlatformTimeSynchronized, DISABLED_longerTimeDiffTest)
    {
        const auto first = synchronized_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        const auto second = synchronized_clock::now();
        EXPECT_GE(second-first, std::chrono::milliseconds(2000u - 50u));
        EXPECT_LE(second-first, std::chrono::milliseconds(2000u + 50u));
    }
}
