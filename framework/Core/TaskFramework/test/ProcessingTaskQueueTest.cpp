//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "TaskFramework/ProcessingTaskQueue.h"

using namespace testing;

namespace ramses_internal
{
    class MockTask : public ITask
    {
    public:
        virtual void execute() override {};
    };

    TEST(ProcessingTaskQueueTest, addAndPopTask)
    {
        ProcessingTaskQueue q;
        MockTask t;
        q.addTask(&t);
        EXPECT_EQ(q.popTask(20), &t);
    }

    TEST(ProcessingTaskQueueTest, isEmptyShowsIfThereAreTasks)
    {
        ProcessingTaskQueue q;
        MockTask t1;
        MockTask t2;
        MockTask t3;

        EXPECT_TRUE(q.isEmpty());
        q.addTask(&t1);
        EXPECT_FALSE(q.isEmpty());
        q.addTask(&t2);
        EXPECT_FALSE(q.isEmpty());
        q.popTask(20);
        EXPECT_FALSE(q.isEmpty());
        q.addTask(&t3);
        EXPECT_FALSE(q.isEmpty());
        q.popTask(20);
        EXPECT_FALSE(q.isEmpty());
        q.popTask(20);
        EXPECT_TRUE(q.isEmpty());
    }

    TEST(ProcessingTaskQueueTest, queueUnblocksWithNullptr)
    {
        ProcessingTaskQueue q;
        auto start = PlatformTime::GetMillisecondsMonotonic();
        EXPECT_TRUE(q.isEmpty());
        q.addTask(nullptr);
        EXPECT_EQ(q.popTask(400u), nullptr);
        EXPECT_GT(50u, PlatformTime::GetMillisecondsMonotonic() - start);
    }

    TEST(ProcessingTaskQueueTest, popTimesOutCorrectly)
    {
        ProcessingTaskQueue q;
        auto start = PlatformTime::GetMillisecondsMonotonic();
        EXPECT_TRUE(q.isEmpty());
        const UInt32 timeout = 50u;
        // There are (unconfirmed) signs of system time being changed on some test targets during execution.
        // Even though we use steady clock to measure all of our own timings, it seems that some condition variable
        // implementations use system time internally which then lead to random fails here.
        // Unfortunately there is no way to fix that on our side without either using high tolerance or removing the test...
        const UInt32 tolerance = 20u;
        EXPECT_EQ(q.popTask(timeout), nullptr);
        EXPECT_GE(PlatformTime::GetMillisecondsMonotonic() - start, timeout - tolerance);
    }
}
