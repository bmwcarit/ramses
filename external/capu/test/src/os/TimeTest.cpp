/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include "ramses-capu/Config.h"
#include "ramses-capu/os/Thread.h"
#include "ramses-capu/os/Time.h"

TEST(Time, getMillisecondsTest_Monotonic)
{
    uint64_t milliSeconds = ramses_capu::Time::GetMillisecondsMonotonic();
    ramses_capu::Thread::Sleep(50);
    uint64_t milliSeconds2 = ramses_capu::Time::GetMillisecondsMonotonic();
    EXPECT_TRUE((milliSeconds2 - milliSeconds) > 40);
}

TEST(Time, getMillisecondsTest_Absolute)
{
    uint64_t milliSeconds = ramses_capu::Time::GetMillisecondsAbsolute();
    ramses_capu::Thread::Sleep(50);
    uint64_t milliSeconds2 = ramses_capu::Time::GetMillisecondsAbsolute();
    EXPECT_TRUE((milliSeconds2 - milliSeconds) > 40);
}

TEST(Time, isBiggerThanSomeReferenceTime)
{
    uint64_t milliSeconds = ramses_capu::Time::GetMillisecondsAbsolute();
    EXPECT_GE(milliSeconds, 1357210706813ull); // check against some reference time (03.01.2013, 12:00)
}

TEST(Time, getMicrosecondsTest_Monotonic)
{
    uint64_t milliSeconds = ramses_capu::Time::GetMicrosecondsMonotonic();
    ramses_capu::Thread::Sleep(50);
    uint64_t milliSeconds2 = ramses_capu::Time::GetMicrosecondsMonotonic();
    EXPECT_TRUE((milliSeconds2 - milliSeconds) > 40000);
}

TEST(Time, getMicrosecondsTest_Absolute)
{
    uint64_t milliSeconds = ramses_capu::Time::GetMicrosecondsAbsolute();
    ramses_capu::Thread::Sleep(50);
    uint64_t milliSeconds2 = ramses_capu::Time::GetMicrosecondsAbsolute();
    EXPECT_TRUE((milliSeconds2 - milliSeconds) > 40000);
}

TEST(Time, getMicrosecondsTestLongTime_Monotonic)
{
    uint64_t milliSeconds = ramses_capu::Time::GetMicrosecondsMonotonic();
    ramses_capu::Thread::Sleep(2000);
    uint64_t milliSeconds2 = ramses_capu::Time::GetMicrosecondsMonotonic();
    uint64_t diff = milliSeconds2 - milliSeconds;
    EXPECT_GE(diff, 2000000u - 50000u);
    EXPECT_LE(diff, 2000000u + 50000u);
}

TEST(Time, getMicrosecondsTestLongTime_Absolute)
{
    uint64_t milliSeconds = ramses_capu::Time::GetMicrosecondsAbsolute();
    ramses_capu::Thread::Sleep(2000);
    uint64_t milliSeconds2 = ramses_capu::Time::GetMicrosecondsAbsolute();
    uint64_t diff = milliSeconds2 - milliSeconds;
    EXPECT_GE(diff, 2000000u - 50000u);
    EXPECT_LE(diff, 2000000u + 50000u);
}
