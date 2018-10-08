//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/ThreadBarrier.h"
#include "gtest/gtest.h"
#include "PlatformAbstraction/PlatformThread.h"

namespace ramses_internal
{
    class AThreadBarrier : public ::testing::Test
    {
    };

    namespace
    {
        class TestRunnable : public Runnable
        {
        public:
            TestRunnable(ThreadBarrier& barrier_)
                : barrier(barrier_)
                , done(false)
            {
            }

            virtual void run() override
            {
                barrier.wait();
                done = true;
            }

            ThreadBarrier& barrier;
            std::atomic<bool> done;
        };
    }

    TEST_F(AThreadBarrier, onlyReturnsFromWaitWhenExpectedNumberOfThreadsHaveReachedIt)
    {
        ThreadBarrier barrier(4);
        TestRunnable r1(barrier);
        TestRunnable r2(barrier);
        TestRunnable r3(barrier);
        PlatformThread th1("ThBarrier1");
        PlatformThread th2("ThBarrier1");
        PlatformThread th3("ThBarrier1");

        EXPECT_FALSE(r1.done);
        EXPECT_FALSE(r2.done);
        EXPECT_FALSE(r3.done);

        th1.start(r1);
        th2.start(r2);

        EXPECT_FALSE(r1.done);
        EXPECT_FALSE(r2.done);
        EXPECT_FALSE(r3.done);

        th3.start(r3);

        EXPECT_FALSE(r1.done);
        EXPECT_FALSE(r2.done);
        EXPECT_FALSE(r3.done);

        barrier.wait();

        th1.join();
        th2.join();
        th3.join();

        EXPECT_TRUE(r1.done);
        EXPECT_TRUE(r2.done);
        EXPECT_TRUE(r3.done);
    }
}
