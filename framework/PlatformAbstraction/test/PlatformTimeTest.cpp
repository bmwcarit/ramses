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
    TEST(PlatformTime, getMillisecondsTest_Monotonic)
    {
        const uint64_t milliSeconds = PlatformTime::GetMillisecondsMonotonic();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        const uint64_t milliSeconds2 = PlatformTime::GetMillisecondsMonotonic();
        EXPECT_TRUE((milliSeconds2 - milliSeconds) > 40);
    }

    TEST(PlatformTime, getMillisecondsTest_Absolute)
    {
        const uint64_t milliSeconds = PlatformTime::GetMillisecondsAbsolute();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        const uint64_t milliSeconds2 = PlatformTime::GetMillisecondsAbsolute();
        EXPECT_TRUE((milliSeconds2 - milliSeconds) > 40);
    }

    TEST(PlatformTime, isBiggerThanSomeReferenceTime)
    {
        const uint64_t milliSeconds = PlatformTime::GetMillisecondsAbsolute();
        EXPECT_GE(milliSeconds, 1357210706813ull); // check against some reference time (03.01.2013, 12:00)
    }

    TEST(PlatformTime, getMicrosecondsTest_Monotonic)
    {
        const uint64_t milliSeconds = PlatformTime::GetMicrosecondsMonotonic();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        const uint64_t milliSeconds2 = PlatformTime::GetMicrosecondsMonotonic();
        EXPECT_TRUE((milliSeconds2 - milliSeconds) > 40000);
    }

    TEST(PlatformTime, getMicrosecondsTest_Absolute)
    {
        const uint64_t milliSeconds = PlatformTime::GetMicrosecondsAbsolute();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        const uint64_t milliSeconds2 = PlatformTime::GetMicrosecondsAbsolute();
        EXPECT_TRUE((milliSeconds2 - milliSeconds) > 40000);
    }

    TEST(PlatformTime, getMicrosecondsTestLongTime_Monotonic)
    {
        const uint64_t milliSeconds = PlatformTime::GetMicrosecondsMonotonic();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        const uint64_t milliSeconds2 = PlatformTime::GetMicrosecondsMonotonic();
        const uint64_t diff = milliSeconds2 - milliSeconds;
        EXPECT_GE(diff, 2000000u - 200000u);
        EXPECT_LE(diff, 2000000u + 200000u);
    }

    TEST(PlatformTime, getMicrosecondsTestLongTime_Absolute)
    {
        const uint64_t milliSeconds = PlatformTime::GetMicrosecondsAbsolute();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        const uint64_t milliSeconds2 = PlatformTime::GetMicrosecondsAbsolute();
        const uint64_t diff = milliSeconds2 - milliSeconds;
        EXPECT_GE(diff, 2000000u - 200000u);
        EXPECT_LE(diff, 2000000u + 200000u);
    }

    TEST(ASynchronizedClock, nowNotZero)
    {
        EXPECT_NE(0u, synchronized_clock::now().time_since_epoch().count());
    }

    TEST(ASynchronizedClock, timeDiffTest)
    {
        const auto first = synchronized_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        const auto second = synchronized_clock::now();
        EXPECT_TRUE((second - first) > std::chrono::milliseconds(40));
    }

    TEST(ASynchronizedClock, longerTimeDiffTest)
    {
        const auto first = synchronized_clock::now();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        const auto second = synchronized_clock::now();
        EXPECT_GE(second-first, std::chrono::milliseconds(2000u - 50u));
        EXPECT_LE(second-first, std::chrono::milliseconds(2000u + 50u));
    }

#if !defined(RAMSES_LINUX_USE_DEV_PTP) && !defined(__INTEGRITY)
    // no ptp support: test synchroized time is system_clock
    TEST(ASynchronizedClock, non_ptp_behavesAsSystemClock)
    {
        const auto tSys1 = std::chrono::system_clock::now().time_since_epoch();
        const auto tSync = synchronized_clock::now().time_since_epoch();
        const auto tSys2 = std::chrono::system_clock::now().time_since_epoch();

        EXPECT_LE(tSys1, tSync);
        EXPECT_LE(tSync, tSys2);
    }
#endif

    TEST(ASynchronizedClock, asMillisecondsWorks)
    {
        EXPECT_EQ(0u, asMilliseconds(synchronized_clock::time_point(std::chrono::milliseconds(0))));
        EXPECT_EQ(10u, asMilliseconds(synchronized_clock::time_point(std::chrono::milliseconds(10))));
        EXPECT_EQ(12345u, asMilliseconds(synchronized_clock::time_point(std::chrono::milliseconds(12345))));
    }

    TEST(ASynchronizedClock, asMicrosecondsWorks)
    {
        EXPECT_EQ(0u, asMicroseconds(synchronized_clock::time_point(std::chrono::microseconds(0))));
        EXPECT_EQ(10u, asMicroseconds(synchronized_clock::time_point(std::chrono::microseconds(10))));
        EXPECT_EQ(12345u, asMicroseconds(synchronized_clock::time_point(std::chrono::microseconds(12345))));
        EXPECT_EQ(10000u, asMicroseconds(synchronized_clock::time_point(std::chrono::milliseconds(10))));
    }
}
