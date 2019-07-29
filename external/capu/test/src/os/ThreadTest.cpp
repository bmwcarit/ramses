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
#include "ramses-capu/os/Thread.h"

class ThreadTest : public ramses_capu::Runnable
{
public:
    static int32_t variable;

    ThreadTest(int32_t val)
    {
        variable = 0;
        mVal = val;
    }

    void run()
    {
        //0.1 second delay
        EXPECT_EQ(ramses_capu::CAPU_OK, ramses_capu::Thread::Sleep(100));
        variable = mVal;
    }
private:
    int32_t mVal;
};

class IncrementerThreadTest : public ramses_capu::Runnable
{
public:
    static int32_t variable;

    IncrementerThreadTest()
    {
        variable = 0;
    }

    void run()
    {
        do
        {
            variable++;
            EXPECT_EQ(ramses_capu::CAPU_OK, ramses_capu::Thread::Sleep(100));
        }
        while (!isCancelRequested());
    }
};

class ThreadTest2 : public ramses_capu::Runnable
{
public:

    ThreadTest2()
    {
    }

    void run()
    {
        ramses_capu::Thread::Sleep(1000);
    }
};

class ThreadTestCancel : public ramses_capu::Runnable
{
public:
    ThreadTestCancel()
    {
    }

    void run()
    {
        while (!isCancelRequested())
        {
            ramses_capu::Thread::Sleep(10);
        }
    }
};


int32_t ThreadTest::variable = 0;
int32_t IncrementerThreadTest::variable = 0;

TEST(Thread, canSetAndGetThreadName)
{
    //CREATE THREAD
    ramses_capu::Thread thread("test name of thread");
    EXPECT_STREQ("test name of thread", thread.getName());
}

TEST(Thread, startAndJoinTest)
{
    ThreadTest _test(6);
    ThreadTest2 _test2;

    //CREATE THREAD
    ramses_capu::Thread* CAPU_thread = new ramses_capu::Thread();
    CAPU_thread->start(_test);

    //WAIT UNTIL IT FINISH
    EXPECT_EQ(ramses_capu::CAPU_OK, CAPU_thread->join());

    //CHECK THE VALUE CALCULATED IN THREAD
    EXPECT_EQ(6, ThreadTest::variable);
    delete CAPU_thread;

    ramses_capu::Thread* CAPU_thread2 = new ramses_capu::Thread();
    CAPU_thread2->start(_test2);
    EXPECT_EQ(ramses_capu::CAPU_OK, CAPU_thread2->join());
    // multiple join is not ok
    EXPECT_EQ(ramses_capu::CAPU_ERROR, CAPU_thread2->join());
    delete CAPU_thread2;
}

TEST(Thread, reuseThread)
{
    ThreadTest runnable1(1);
    ThreadTest runnable2(42);

    {
        ramses_capu::Thread thread;
        EXPECT_EQ(ramses_capu::TS_NEW, thread.getState());
        EXPECT_EQ(ramses_capu::CAPU_OK, thread.start(runnable1));

        // start with another runnable should fail until joined
        EXPECT_NE(ramses_capu::CAPU_OK, thread.start(runnable2));

        EXPECT_EQ(ramses_capu::CAPU_OK, thread.join());
        EXPECT_EQ(ramses_capu::TS_TERMINATED, thread.getState());
        EXPECT_EQ(1, ThreadTest::variable);
        EXPECT_EQ(ramses_capu::CAPU_OK, thread.start(runnable2));
        EXPECT_NE(ramses_capu::TS_TERMINATED, thread.getState());
    }

    // thread should be joined
    EXPECT_EQ(42, ThreadTest::variable);
}

TEST(Thread, startAndCancelTest)
{
    ThreadTestCancel runnable;

    ramses_capu::Thread* thread = new ramses_capu::Thread();
    thread->start(runnable);


    thread->cancel();

    // test blocks forever if cancel didn't work
    EXPECT_EQ(ramses_capu::CAPU_OK, thread->join());

    delete thread;
}

TEST(Thread, reuseCancelledThread)
{
    IncrementerThreadTest runnable;
    {
        ramses_capu::Thread thread;
        EXPECT_EQ(ramses_capu::TS_NEW, thread.getState());
        EXPECT_EQ(ramses_capu::CAPU_OK, thread.start(runnable));

        // start with another runnable should fail until joined
        EXPECT_NE(ramses_capu::CAPU_OK, thread.start(runnable));

        thread.cancel();
        EXPECT_EQ(ramses_capu::CAPU_OK, thread.join());
        EXPECT_EQ(ramses_capu::TS_TERMINATED, thread.getState());
        EXPECT_LT(0, IncrementerThreadTest::variable); // should have run at least once until now
        int32_t previousValue = IncrementerThreadTest::variable;

        // restart without reset only runs first part of code
        EXPECT_EQ(ramses_capu::CAPU_OK, thread.start(runnable));
        EXPECT_EQ(ramses_capu::CAPU_OK, thread.join());
        EXPECT_EQ(previousValue + 1, IncrementerThreadTest::variable);
        previousValue = IncrementerThreadTest::variable;

        // restart with reset
        EXPECT_EQ(ramses_capu::CAPU_OK, thread.start(runnable));
        thread.cancel();
        EXPECT_EQ(ramses_capu::CAPU_OK, thread.join());
        EXPECT_LT(previousValue, IncrementerThreadTest::variable);

    }
}

TEST(Thread, joinWithoutStartingIsOK)
{
    ThreadTestCancel runnable;

    ramses_capu::Thread* thread = new ramses_capu::Thread();
    // join without having started
    EXPECT_EQ(ramses_capu::CAPU_ERROR, thread->join());

    delete thread;
}

TEST(Thread, startAndDestructorTest)
{
    int32_t newval = 6;
    ThreadTest _test(newval);

    ramses_capu::Thread* CAPU_thread = new ramses_capu::Thread();
    CAPU_thread->start(_test);
    delete CAPU_thread;

    EXPECT_EQ(newval, ThreadTest::variable);
}

TEST(Thread, sleepTest)
{
    int32_t newval = 5;
    ThreadTest _test(newval);
    //CREATE THREAD
    ramses_capu::Thread* CAPU_thread = new ramses_capu::Thread();
    CAPU_thread->start(_test);
    //WAIT UNTIL IT FINISH
    EXPECT_EQ(ramses_capu::CAPU_OK, CAPU_thread->join());
    EXPECT_EQ(newval, ThreadTest::variable);
    delete CAPU_thread;
}

TEST(Thread, contextTest)
{
    int32_t newval = 4;
    ThreadTest _test(newval);
    {
        ramses_capu::Thread thread;
        thread.start(_test);
    }
    EXPECT_EQ(newval, ThreadTest::variable);
}

TEST(Thread, getState)
{
    ThreadTestCancel r1;
    ramses_capu::Thread thread;
    ramses_capu::ThreadState state = thread.getState();
    EXPECT_EQ(ramses_capu::TS_NEW, state);
    thread.start(r1);
    state = thread.getState();
    EXPECT_NE(ramses_capu::TS_TERMINATED, state);
    thread.cancel();
    thread.join();
    state = thread.getState();
    EXPECT_EQ(ramses_capu::TS_TERMINATED, state);
}

class IdTester : public ramses_capu::Runnable
{
public:
    ramses_capu::uint_t m_id;

    void run()
    {
        ramses_capu::Thread::Sleep(500);
        m_id = ramses_capu::Thread::CurrentThreadId();
    }
};

TEST(Thread, getId)
{
    ramses_capu::uint_t id1 = ramses_capu::Thread::CurrentThreadId();
    IdTester idTester1;
    IdTester idTester2;
    ramses_capu::Thread t1;
    t1.start(idTester1);
    ramses_capu::Thread t2;
    t2.start(idTester2);
    t1.join();
    t2.join();
    ramses_capu::uint_t id2 = idTester1.m_id;
    ramses_capu::uint_t id3 = idTester2.m_id;
    EXPECT_NE(id1, id2);
    EXPECT_NE(id2, id3);
    EXPECT_NE(id1, id3);
}

TEST(Thread, cancelWithoutRunnable)
{
    ramses_capu::Thread thread;
    thread.cancel();

    // no test as just no crash is expected
}
