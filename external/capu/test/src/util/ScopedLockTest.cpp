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

#include <gmock/gmock.h>

#include "ramses-capu/Config.h"
#include "ramses-capu/util/ScopedLock.h"
#include "ramses-capu/os/Mutex.h"

class Lockable
{
public:

    Lockable()
        : lockCount(0)
    {
    }

    void lock()
    {
        ++lockCount;
    }

    void unlock()
    {
        --lockCount;
    }

    int32_t lockCount;
};


TEST(ScopedLockTest, ScopedLock)
{
    Lockable lockable;

    EXPECT_EQ(0, lockable.lockCount);
    {
        ramses_capu::ScopedLock<Lockable> lock1(lockable);
        EXPECT_EQ(1, lockable.lockCount);
        {
            ramses_capu::ScopedLock<Lockable> lock2(lockable);
            EXPECT_EQ(2, lockable.lockCount);
        }
        EXPECT_EQ(1, lockable.lockCount);
    }
    EXPECT_EQ(0, lockable.lockCount);
}

TEST(ScopedLockTest, ScopedMutexLock)
{
    ramses_capu::Mutex mutex;

    // just a test for the typedef, must compile fine
    // -> no expectation
    ramses_capu::ScopedMutexLock lock(mutex);
}
